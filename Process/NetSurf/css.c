/**
 * NetSurf CSS wrapper implementation. See css.h.
 *
 * Follows libcss test/select.c: stylesheet params, select handler vtable,
 * select-then-compose walk, UA default hints. Unsupported selectors (sibling
 * combinators, dynamic pseudo-classes, ...) report no-match, which is legal:
 * those rules simply don't apply.
 */

#include "css.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <_xeneva.h>

#include <libcss/libcss.h>
#include <libcss/stylesheet.h>
#include <libcss/select.h>
#include <libcss/computed.h>
#include <libcss/font_face.h>
#include <libcss/hint.h>
#include <libwapcaplet/libwapcaplet.h>

#define NS_CSS_BASE_PX 14

static css_select_ctx* s_select = NULL;
static css_stylesheet* s_uaSheet = NULL;
static css_stylesheet* s_authorSheet = NULL;
static css_media s_media;
static css_unit_ctx s_unitCtx;

static lwc_string* s_attrClass = NULL;
static lwc_string* s_attrId = NULL;

/* UA reader defaults: white body text, sized headings, blue links. */
static const char kUaCss[] =
    "body{color:#ffffff}"
    "h1{font-size:22px}h2{font-size:19px}h3{font-size:17px}"
    "small{font-size:11px}"
    "a{color:#6689d5}";

static css_error ns_resolve_url(void* pw, const char* base, lwc_string* rel, lwc_string** abs) {
    (void)pw;
    (void)base;
    *abs = lwc_string_ref(rel);
    return CSS_OK;
}

/* System fonts degrade to fixed reader values. */
static css_error ns_resolve_font(void* pw, lwc_string* name, css_system_font* system_font) {
    (void)pw;
    (void)name;
    if (!system_font)
        return CSS_BADPARM;
    system_font->style = CSS_FONT_STYLE_NORMAL;
    system_font->variant = CSS_FONT_VARIANT_NORMAL;
    system_font->weight = CSS_FONT_WEIGHT_NORMAL;
    system_font->size.size = NS_CSS_BASE_PX * (1 << CSS_RADIX_POINT);
    system_font->size.unit = CSS_UNIT_PX;
    system_font->line_height.size = (NS_CSS_BASE_PX + 5) * (1 << CSS_RADIX_POINT);
    system_font->line_height.unit = CSS_UNIT_PX;
    return CSS_OK;
}

static int ns_make_sheet(const char* data, size_t len, bool inlineStyle, css_stylesheet** out) {
    css_stylesheet_params params;
    css_stylesheet* sheet = NULL;

    memset(&params, 0, sizeof(params));
    params.params_version = CSS_STYLESHEET_PARAMS_VERSION_1;
    params.level = CSS_LEVEL_21;
    params.charset = "UTF-8";
    params.url = "xeneva";
    params.title = "xeneva";
    params.allow_quirks = false;
    params.inline_style = inlineStyle;
    params.resolve = ns_resolve_url;
    params.resolve_pw = NULL;
    params.import = NULL;
    params.import_pw = NULL;
    params.color = NULL;
    params.color_pw = NULL;
    params.font = ns_resolve_font;
    params.font_pw = NULL;

    if (css_stylesheet_create(&params, &sheet) != CSS_OK)
        return -1;
    _KePrint("[css] sheet created\n");
    if (len > 0) {
        /* NEEDDATA is the normal "streaming" return, not an error:
         * data_done() finalises the sheet. */
        css_error err = css_stylesheet_append_data(sheet, (const uint8_t*)data, len);
        if (err != CSS_OK && err != CSS_NEEDDATA) {
            css_stylesheet_destroy(sheet);
            return -1;
        }
    }
    if (css_stylesheet_data_done(sheet) != CSS_OK) {
        css_stylesheet_destroy(sheet);
        return -1;
    }
    _KePrint("[css] sheet done\n");
    *out = sheet;
    return 0;
}

int NsCssBegin(void) {
    NsCssEnd();
    memset(&s_media, 0, sizeof(s_media));
    s_media.type = CSS_MEDIA_SCREEN;
    memset(&s_unitCtx, 0, sizeof(s_unitCtx));
    s_unitCtx.font_size_default = NS_CSS_BASE_PX * (1 << CSS_RADIX_POINT);

    if (css_select_ctx_create(&s_select) != CSS_OK)
        return -1;
    _KePrint("[css] ctx ok\n");
    if (ns_make_sheet(kUaCss, sizeof(kUaCss) - 1, false, &s_uaSheet) != 0)
        return -1;
    _KePrint("[css] ua ok\n");
    if (css_select_ctx_append_sheet(s_select, s_uaSheet, CSS_ORIGIN_UA, "screen") != CSS_OK)
        return -1;
    if (lwc_intern_string("class", 5, &s_attrClass) != lwc_error_ok)
        return -1;
    if (lwc_intern_string("id", 2, &s_attrId) != lwc_error_ok)
        return -1;
    return 0;
}

int NsCssAddSheet(const char* data, size_t len) {
    css_stylesheet* sheet = NULL;
    if (!s_select || !data || len == 0)
        return -1;
    if (s_authorSheet) {
        css_select_ctx_remove_sheet(s_select, s_authorSheet);
        css_stylesheet_destroy(s_authorSheet);
        s_authorSheet = NULL;
    }
    if (ns_make_sheet(data, len, false, &sheet) != 0)
        return -1;
    if (css_select_ctx_append_sheet(s_select, sheet, CSS_ORIGIN_AUTHOR, "screen") != CSS_OK) {
        css_stylesheet_destroy(sheet);
        return -1;
    }
    s_authorSheet = sheet;
    return 0;
}

void NsCssEnd(void) {
    if (s_select) {
        if (s_authorSheet) {
            css_select_ctx_remove_sheet(s_select, s_authorSheet);
            css_stylesheet_destroy(s_authorSheet);
            s_authorSheet = NULL;
        }
        if (s_uaSheet) {
            css_select_ctx_remove_sheet(s_select, s_uaSheet);
            css_stylesheet_destroy(s_uaSheet);
            s_uaSheet = NULL;
        }
        css_select_ctx_destroy(s_select);
        s_select = NULL;
    }
    if (s_attrClass) {
        lwc_string_unref(s_attrClass);
        s_attrClass = NULL;
    }
    if (s_attrId) {
        lwc_string_unref(s_attrId);
        s_attrId = NULL;
    }
}

/* ---- nodes ---- */

NsCssNode* NsCssNodeCreate(NsCssNode* parent, const char* tag) {
    NsCssNode* n = (NsCssNode*)malloc(sizeof(NsCssNode));
    if (!n)
        return NULL;
    memset(n, 0, sizeof(*n));
    n->parent = parent;
    strncpy(n->tag, tag ? tag : "div", NS_CSS_TAG_LEN);
    n->tag[NS_CSS_TAG_LEN] = 0;
    if (lwc_intern_string(n->tag, strlen(n->tag), (lwc_string**)&n->lwcName) != lwc_error_ok) {
        free(n);
        return NULL;
    }
    return n;
}

void NsCssNodeAddClass(NsCssNode* n, const char* cls) {
    if (!n || !cls || !cls[0] || n->n_cls >= NS_CSS_MAX_CLASSES)
        return;
    strncpy(n->cls[n->n_cls], cls, NS_CSS_CLASS_LEN);
    n->cls[n->n_cls][NS_CSS_CLASS_LEN] = 0;
    if (lwc_intern_string(n->cls[n->n_cls],
                          strlen(n->cls[n->n_cls]),
                          (lwc_string**)&n->lwcCls[n->n_cls]) == lwc_error_ok)
        n->n_cls++;
}

void NsCssNodeSetId(NsCssNode* n, const char* id) {
    if (!n || !id || !id[0])
        return;
    strncpy(n->id, id, NS_CSS_ID_LEN);
    n->id[NS_CSS_ID_LEN] = 0;
    lwc_intern_string(n->id, strlen(n->id), (lwc_string**)&n->lwcId);
}

void NsCssNodeSetLink(NsCssNode* n, bool link) {
    if (n)
        n->is_link = link;
}

int NsCssNodeSetInline(NsCssNode* n, const char* style) {
    css_stylesheet* sheet = NULL;
    if (!n || !style || !style[0])
        return -1;
    if (ns_make_sheet(style, strlen(style), true, &sheet) != 0)
        return -1;
    if (n->inlineSheet)
        css_stylesheet_destroy((css_stylesheet*)n->inlineSheet);
    n->inlineSheet = sheet;
    return 0;
}

void NsCssNodeDestroy(NsCssNode* n) {
    uint32_t i;
    if (!n)
        return;
    if (n->selectResults) {
        css_select_results_destroy((css_select_results*)n->selectResults);
        n->selectResults = NULL;
    }
    if (n->inlineSheet) {
        css_stylesheet_destroy((css_stylesheet*)n->inlineSheet);
        n->inlineSheet = NULL;
    }
    if (n->lwcName) {
        lwc_string_unref((lwc_string*)n->lwcName);
        n->lwcName = NULL;
    }
    for (i = 0; i < n->n_cls; i++) {
        if (n->lwcCls[i]) {
            lwc_string_unref((lwc_string*)n->lwcCls[i]);
            n->lwcCls[i] = NULL;
        }
    }
    if (n->lwcId) {
        lwc_string_unref((lwc_string*)n->lwcId);
        n->lwcId = NULL;
    }
    free(n);
}

/* ---- select handler ---- */

static css_error ns_node_name(void* pw, void* node, css_qname* qname) {
    NsCssNode* n = (NsCssNode*)node;
    (void)pw;
    qname->ns = NULL;
    qname->name = lwc_string_ref((lwc_string*)n->lwcName);
    return CSS_OK;
}

static css_error ns_node_classes(void* pw, void* node, lwc_string*** classes, uint32_t* n_classes) {
    NsCssNode* n = (NsCssNode*)node;
    uint32_t i;
    (void)pw;
    *classes = (lwc_string**)n->lwcCls;
    *n_classes = n->n_cls;
    for (i = 0; i < *n_classes; i++)
        (*classes)[i] = lwc_string_ref((lwc_string*)n->lwcCls[i]);
    return CSS_OK;
}

static css_error ns_node_id(void* pw, void* node, lwc_string** id) {
    NsCssNode* n = (NsCssNode*)node;
    (void)pw;
    if (n->lwcId)
        *id = lwc_string_ref((lwc_string*)n->lwcId);
    else
        *id = NULL;
    return CSS_OK;
}

static css_error ns_named_ancestor(void* pw, void* node, const css_qname* qname, void** ancestor) {
    NsCssNode* n = ((NsCssNode*)node)->parent;
    (void)pw;
    for (; n; n = n->parent) {
        bool match = false;
        if (lwc_string_caseless_isequal(qname->name, (lwc_string*)n->lwcName, &match) != lwc_error_ok)
            return CSS_BADPARM;
        if (match)
            break;
    }
    *ancestor = n;
    return CSS_OK;
}

static css_error ns_named_parent(void* pw, void* node, const css_qname* qname, void** parent) {
    NsCssNode* n = ((NsCssNode*)node)->parent;
    bool match = false;
    (void)pw;
    if (!n) {
        *parent = NULL;
        return CSS_OK;
    }
    if (lwc_string_caseless_isequal(qname->name, (lwc_string*)n->lwcName, &match) != lwc_error_ok)
        return CSS_BADPARM;
    *parent = match ? n : NULL;
    return CSS_OK;
}

static css_error ns_no_node(void* pw, void* node, const css_qname* qname, void** out) {
    (void)pw;
    (void)node;
    (void)qname;
    *out = NULL;
    return CSS_OK;
}

static css_error ns_parent_node(void* pw, void* node, void** parent) {
    (void)pw;
    *parent = ((NsCssNode*)node)->parent;
    return CSS_OK;
}

static css_error ns_sibling_node(void* pw, void* node, void** sibling) {
    (void)pw;
    (void)node;
    *sibling = NULL;
    return CSS_OK;
}

static css_error ns_no_match_bool(void* pw, void* node, bool* match) {
    (void)pw;
    (void)node;
    *match = false;
    return CSS_OK;
}

static css_error ns_no_attr_bool(void* pw, void* node, const css_qname* qname, bool* match) {
    (void)pw;
    (void)node;
    (void)qname;
    *match = false;
    return CSS_OK;
}

static css_error ns_no_attr_val_bool(
    void* pw, void* node, const css_qname* qname, lwc_string* value, bool* match) {
    (void)pw;
    (void)node;
    (void)qname;
    (void)value;
    *match = false;
    return CSS_OK;
}

static css_error ns_no_lang_bool(void* pw, void* node, lwc_string* lang, bool* match) {
    (void)pw;
    (void)node;
    (void)lang;
    *match = false;
    return CSS_OK;
}

static css_error ns_no_hints(void* pw, void* node, uint32_t* nhints, css_hint** hints) {
    (void)pw;
    (void)node;
    *nhints = 0;
    *hints = NULL;
    return CSS_OK;
}

static css_error ns_node_has_name(void* pw, void* node, const css_qname* qname, bool* match) {
    NsCssNode* n = (NsCssNode*)node;
    (void)pw;
    if (lwc_string_caseless_isequal(qname->name, (lwc_string*)n->lwcName, match) != lwc_error_ok)
        return CSS_BADPARM;
    return CSS_OK;
}

static css_error ns_node_has_class(void* pw, void* node, lwc_string* name, bool* match) {
    NsCssNode* n = (NsCssNode*)node;
    uint32_t i;
    (void)pw;
    *match = false;
    for (i = 0; i < n->n_cls; i++) {
        bool eq = false;
        if (lwc_string_isequal(name, (lwc_string*)n->lwcCls[i], &eq) != lwc_error_ok)
            return CSS_BADPARM;
        if (eq) {
            *match = true;
            break;
        }
    }
    return CSS_OK;
}

static css_error ns_node_has_id(void* pw, void* node, lwc_string* name, bool* match) {
    NsCssNode* n = (NsCssNode*)node;
    (void)pw;
    *match = false;
    if (n->lwcId) {
        if (lwc_string_isequal(name, (lwc_string*)n->lwcId, match) != lwc_error_ok)
            return CSS_BADPARM;
    }
    return CSS_OK;
}

static css_error ns_node_is_root(void* pw, void* node, bool* match) {
    (void)pw;
    *match = (((NsCssNode*)node)->parent == NULL);
    return CSS_OK;
}

static css_error ns_node_is_empty(void* pw, void* node, bool* match) {
    (void)pw;
    (void)node;
    *match = false;
    return CSS_OK;
}

static css_error ns_node_is_link(void* pw, void* node, bool* match) {
    (void)pw;
    *match = ((NsCssNode*)node)->is_link;
    return CSS_OK;
}

static css_error ns_node_count_siblings(void* pw, void* node, bool same_name, bool after, int32_t* count) {
    (void)pw;
    (void)node;
    (void)same_name;
    (void)after;
    *count = 0;
    return CSS_OK;
}

static css_error ns_ua_default(void* pw, uint32_t property, css_hint* hint) {
    (void)pw;
    if (property == CSS_PROP_COLOR) {
        hint->data.color = 0xffffffffu;
        hint->status = CSS_COLOR_COLOR;
    } else if (property == CSS_PROP_FONT_FAMILY) {
        hint->data.strings = NULL;
        hint->status = CSS_FONT_FAMILY_SANS_SERIF;
    } else if (property == CSS_PROP_QUOTES) {
        hint->data.strings = NULL;
        hint->status = CSS_QUOTES_NONE;
    } else if (property == CSS_PROP_VOICE_FAMILY) {
        hint->data.strings = NULL;
        hint->status = 0;
    } else {
        return CSS_INVALID;
    }
    return CSS_OK;
}

static css_error ns_set_node_data(void* pw, void* node, void* libcss_node_data) {
    (void)pw;
    (void)node;
    (void)libcss_node_data;
    return CSS_OK;
}

static css_error ns_get_node_data(void* pw, void* node, void** libcss_node_data) {
    (void)pw;
    (void)node;
    *libcss_node_data = NULL;
    return CSS_OK;
}

static css_select_handler s_handler = {
    CSS_SELECT_HANDLER_VERSION_1,
    ns_node_name,
    ns_node_classes,
    ns_node_id,
    ns_named_ancestor,
    ns_named_parent,
    ns_no_node, /* named_sibling_node: unsupported */
    ns_no_node, /* named_generic_sibling_node: unsupported */
    ns_parent_node,
    ns_sibling_node,
    ns_node_has_name,
    ns_node_has_class,
    ns_node_has_id,
    ns_no_attr_bool,
    ns_no_attr_val_bool,
    ns_no_attr_val_bool,
    ns_no_attr_val_bool,
    ns_no_attr_val_bool,
    ns_no_attr_val_bool,
    ns_no_attr_val_bool,
    ns_node_is_root,
    ns_node_count_siblings,
    ns_node_is_empty,
    ns_node_is_link,
    ns_no_match_bool, /* visited */
    ns_no_match_bool, /* hover */
    ns_no_match_bool, /* active */
    ns_no_match_bool, /* focus */
    ns_no_match_bool, /* enabled */
    ns_no_match_bool, /* disabled */
    ns_no_match_bool, /* checked */
    ns_no_match_bool, /* target */
    ns_no_lang_bool,
    ns_no_hints,
    ns_ua_default,
    ns_set_node_data,
    ns_get_node_data,
};

int NsCssSelect(NsCssNode* n) {
    css_select_results* sr = NULL;
    if (!n || !s_select)
        return -1;
    if (n->parent == NULL)
        s_unitCtx.root_style = NULL;
    if (css_select_style(s_select,
                         n,
                         &s_unitCtx,
                         &s_media,
                         (const css_stylesheet*)n->inlineSheet,
                         &s_handler,
                         NULL,
                         &sr) != CSS_OK)
        return -1;
    if (n->parent != NULL && n->parent->selectResults != NULL) {
        css_computed_style* composed = NULL;
        css_computed_style* parentStyle =
            ((css_select_results*)n->parent->selectResults)->styles[CSS_PSEUDO_ELEMENT_NONE];
        if (css_computed_style_compose(parentStyle,
                                       sr->styles[CSS_PSEUDO_ELEMENT_NONE],
                                       &s_unitCtx,
                                       &composed) == CSS_OK) {
            css_computed_style_destroy(sr->styles[CSS_PSEUDO_ELEMENT_NONE]);
            sr->styles[CSS_PSEUDO_ELEMENT_NONE] = composed;
        }
    }
    if (n->selectResults)
        css_select_results_destroy((css_select_results*)n->selectResults);
    n->selectResults = sr;
    if (n->parent == NULL)
        s_unitCtx.root_style = sr->styles[CSS_PSEUDO_ELEMENT_NONE];
    return 0;
}

/* ---- computed value readers ---- */

static css_computed_style* ns_style(NsCssNode* n) {
    if (!n || !n->selectResults)
        return NULL;
    return ((css_select_results*)n->selectResults)->styles[CSS_PSEUDO_ELEMENT_NONE];
}

uint32_t NsCssNodeColor(NsCssNode* n) {
    css_computed_style* st = ns_style(n);
    if (!st)
        return 0xffffffffu;
    {
        css_color color = 0;
        uint8_t type = css_computed_color(st, &color);
        if (type == CSS_COLOR_COLOR)
            return (uint32_t)color;
    }
    return 0xffffffffu;
}

static int ns_len_px(css_fixed length, css_unit unit, int basePx) {
    /* css_fixed has CSS_RADIX_POINT fractional bits. */
    if (unit == CSS_UNIT_PX) {
        return (int)(length >> CSS_RADIX_POINT);
    } else if (unit == CSS_UNIT_PT) {
        return (int)((length >> CSS_RADIX_POINT) * 4 / 3);
    } else if (unit == CSS_UNIT_PC) {
        return (int)((length >> CSS_RADIX_POINT) * 16);
    } else if (unit == CSS_UNIT_EM || unit == CSS_UNIT_REM || unit == CSS_UNIT_EX) {
        return (int)(length >> CSS_RADIX_POINT) * basePx;
    }
    return -1;
}

int NsCssNodeFontPx(NsCssNode* n) {
    css_computed_style* st = ns_style(n);
    if (!st)
        return NS_CSS_BASE_PX;
    {
        css_fixed length = 0;
        css_unit unit = CSS_UNIT_PX;
        uint8_t type = css_computed_font_size(st, &length, &unit);
        switch (type) {
        case CSS_FONT_SIZE_XX_SMALL:
            return 9;
        case CSS_FONT_SIZE_X_SMALL:
            return 10;
        case CSS_FONT_SIZE_SMALL:
            return 12;
        case CSS_FONT_SIZE_MEDIUM:
            return NS_CSS_BASE_PX;
        case CSS_FONT_SIZE_LARGE:
            return 17;
        case CSS_FONT_SIZE_X_LARGE:
            return 20;
        case CSS_FONT_SIZE_XX_LARGE:
            return 24;
        case CSS_FONT_SIZE_LARGER:
            return NS_CSS_BASE_PX + 2;
        case CSS_FONT_SIZE_SMALLER:
            return NS_CSS_BASE_PX - 2;
        case CSS_FONT_SIZE_DIMENSION: {
            int px = ns_len_px(length, unit, NS_CSS_BASE_PX);
            if (px < 8)
                px = 8;
            if (px > 28)
                px = 28;
            if (px > 0)
                return px;
            break;
        }
        default:
            break;
        }
    }
    return NS_CSS_BASE_PX;
}

static int ns_margin_px(css_computed_style* st, int which) {
    css_fixed length = 0;
    css_unit unit = CSS_UNIT_PX;
    uint8_t type;
    if (which == 0)
        type = css_computed_margin_top(st, &length, &unit);
    else
        type = css_computed_margin_bottom(st, &length, &unit);
    if (type != CSS_MARGIN_SET)
        return 0;
    {
        int px = ns_len_px(length, unit, NS_CSS_BASE_PX);
        if (px < 0)
            px = 0;
        return px;
    }
}

int NsCssNodeMarginLines(NsCssNode* n) {
    css_computed_style* st = ns_style(n);
    int lines;
    if (!st)
        return 0;
    lines = (ns_margin_px(st, 0) + ns_margin_px(st, 1)) / 14;
    if (lines < 0)
        lines = 0;
    if (lines > 2)
        lines = 2;
    return lines;
}
