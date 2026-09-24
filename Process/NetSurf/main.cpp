/**
 * NetSurf bootstrap browser for XenevaOS.
 *
 * Staging frontend, not upstream NetSurf core yet: a Chitralekha window with
 * a URL bar + Go button, HTTP/1.0 fetch over XEClib sockets (same path as
 * Process/http/curl.exe), and crude HTML-to-text rendering into a textbox.
 *
 * Real NetSurf (libcss/libdom/libparserutils) + HTTPS still need the musl
 * shim and an mbedTLS port first -- https:// URLs get an honest message.
 */

#include <stdint.h>
#include <_xeneva.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <chitralekha.h>
#include <widgets/base.h>
#include <widgets/button.h>
#include <widgets/textbox.h>
#include <widgets/window.h>
#include <keycode.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/socket.h>
#include <sys/netdb.h>
#include <arpa/inet.h>
#include <sys/iocodes.h>
#include <ctype.h>

#include "css.h"

#define NS_URL_MAX 256
#define NS_BODY_MAX (16 * 1024)
/* Visible page lines: 420px box at the painter's 19px line stride. */
#define NS_PAGE_LINES 22

/* Extended key codes (0xE0-prefixed AT Set-1, as the virtio-kbd driver
 * forwards them -- same encoding Doom's platform layer decodes). */
#define NS_KEY_LEFT 0xE04B
#define NS_KEY_RIGHT 0xE04D
#define NS_KEY_UP 0xE048
#define NS_KEY_DOWN 0xE050
#define NS_KEY_HOME 0xE047
#define NS_KEY_END 0xE04F
#define NS_KEY_PGUP 0xE049
#define NS_KEY_PGDN 0xE051

static ChitralekhaApp* s_app = NULL;
static ChWindow* s_mainWin = NULL;
static ChTextBox* s_urlBox = NULL;
static ChTextBox* s_pageBox = NULL;

static char s_url[NS_URL_MAX] = "example.com";
static size_t s_urlCursor = 0;
static char s_body[NS_BODY_MAX + 1];
static char s_text[NS_BODY_MAX + 1];
static char s_view[NS_BODY_MAX + 1];
static int s_pageTop = 0;
static size_t s_viewStart = 0;
static ChFont* s_fontBig = NULL;
/* Per-character style for s_text, recorded during the HTML pass. */
static uint32_t s_col[NS_BODY_MAX + 1];
static uint8_t s_sz[NS_BODY_MAX + 1];
/* Author <style> bodies collected before the render pass (8KB cap). */
static char s_cssBuf[8192];
/* Current emission run, refreshed from the element stack top. */
static uint32_t g_runColor = 0xffffffffu;
static uint8_t g_runSize = 14;

static void NsPageRender(void) {
    if (!s_pageBox || !s_mainWin)
        return;
    /* Clamp the first visible line, then slice [s_pageTop, +NS_PAGE_LINES). */
    int total = 0;
    if (s_text[0]) {
        total = 1;
        for (const char* p = s_text; *p; p++)
            if (*p == '\n')
                total++;
    }
    int maxTop = total - NS_PAGE_LINES;
    if (maxTop < 0)
        maxTop = 0;
    if (s_pageTop > maxTop)
        s_pageTop = maxTop;
    if (s_pageTop < 0)
        s_pageTop = 0;

    size_t di = 0;
    int line = 0;
    int shown = 0;
    bool marked = false;
    s_viewStart = 0;
    for (size_t i = 0; s_text[i] && di + 1 < sizeof(s_view); i++) {
        if (line < s_pageTop) {
            if (s_text[i] == '\n')
                line++;
            continue;
        }
        if (!marked) {
            s_viewStart = i;
            marked = true;
        }
        if (shown >= NS_PAGE_LINES)
            break;
        s_view[di++] = s_text[i];
        if (s_text[i] == '\n')
            shown++;
    }
    s_view[di] = 0;
    ChTextBoxSetText(s_pageBox, s_view);
    ChTextBoxUpdate(s_pageBox, s_mainWin);
}

static void NsPagePaint(ChWidget* wid, ChWindow* win) {
    /* Styled page paint: per-character color/size from the CSS pass.
     * Line stride stays 19px so the NS_PAGE_LINES scroll math holds. */
    ChTextBox* tb = (ChTextBox*)wid;
    uint32_t bg = s_mainWin ? s_mainWin->color : 0xFF252525u;
    ChDrawRect(win->canv, tb->wid.x, tb->wid.y, tb->wid.w, tb->wid.h, bg);
    if (!tb->font)
        return;
    ChRect clip;
    clip.x = tb->wid.x;
    clip.y = tb->wid.y;
    clip.w = tb->wid.w;
    clip.h = tb->wid.h;
    int penx = tb->wid.x + 4;
    int peny = tb->wid.y + 19;
    for (size_t k = 0; s_view[k]; k++) {
        char c = s_view[k];
        if (c == '\n') {
            penx = tb->wid.x + 4;
            peny += 19;
            if (peny > tb->wid.y + tb->wid.h)
                break;
            continue;
        }
        size_t si = s_viewStart + k;
        uint32_t col = (si <= NS_BODY_MAX) ? s_col[si] : 0xffffffffu;
        uint8_t szpx = (si <= NS_BODY_MAX) ? s_sz[si] : 14;
        ChFont* f = (szpx >= 17 && s_fontBig) ? s_fontBig : tb->font;
        ChFontDrawCharClipped(win->canv, f, c, penx, peny, col, &clip);
        penx += ChFontGetWidthChar(f, c);
    }
}

static void NsPageScroll(int delta) {
    s_pageTop += delta;
    NsPageRender();
}

static void NsShowStatus(const char* msg) {
    if (!s_pageBox || !s_mainWin || !msg)
        return;
    /* Every new page/message resets the scroll to the top. */
    if (msg != (const char*)s_text) {
        strncpy(s_text, msg, sizeof(s_text) - 1);
        s_text[sizeof(s_text) - 1] = 0;
        /* Unstyled message: UA white on dark, base size. */
        for (size_t f = 0; f <= NS_BODY_MAX && s_text[f]; f++) {
            s_col[f] = 0xffffffffu;
            s_sz[f] = 14;
        }
    }
    s_pageTop = 0;
    NsPageRender();
}

static void NsUrlPaint(ChWidget* wid, ChWindow* win) {
    /* Custom single-line paint: the default textbox painter starts its pen
     * 38px below the box top, which never fits a 24px bar -- and hover
     * repaints invoke the handler directly, so per-update cursor tweaks
     * don't survive mouse movement. Fixed pen here instead. */
    ChTextBox* tb = (ChTextBox*)wid;
    ChDrawRect(win->canv, tb->wid.x, tb->wid.y, tb->wid.w, tb->wid.h, 0xFFFFFFFFu);
    if (!tb->text || !tb->text[0] || !tb->font)
        return;
    ChRect clip;
    clip.x = tb->wid.x;
    clip.y = tb->wid.y;
    clip.w = tb->wid.w;
    clip.h = tb->wid.h;
    ChFontDrawTextClipped(win->canv,
                          tb->font,
                          tb->text,
                          tb->wid.x + 4,
                          tb->wid.y + 19,
                          tb->textColor,
                          &clip);
    /* Block caret at the edit cursor: measure the prefix width. */
    if (tb->font) {
        size_t len = strlen(tb->text);
        size_t caret = s_urlCursor <= len ? s_urlCursor : len;
        int64_t cw = 0;
        if (caret > 0) {
            char prefix[NS_URL_MAX + 1];
            memcpy(prefix, tb->text, caret);
            prefix[caret] = 0;
            cw = ChFontGetWidth(tb->font, prefix);
            if (cw < 0)
                cw = 0;
        }
        int cx = tb->wid.x + 4 + (int)cw;
        if (cx + 2 <= tb->wid.x + tb->wid.w)
            ChDrawRect(win->canv, cx, tb->wid.y + 3, 2, tb->wid.h - 6, tb->textColor);
    }
}

static void NsShowUrl(void) {
    if (!s_urlBox || !s_mainWin)
        return;
    ChTextBoxSetText(s_urlBox, s_url);
    ChTextBoxUpdate(s_urlBox, s_mainWin);
}

/* Split "http://host[:port]/path" into parts. Returns 0 ok, -1 malformed,
 * -2 https (no TLS on Xeneva yet). Mirrors Process/http/main.cpp. */
static int NsParseUrl(const char* url, char* host, size_t hostsz, char* path, size_t pathsz, uint16_t* port) {
    const char* p = url;
    *port = 80;
    strcpy(path, "/");

    if (!strncmp(p, "https://", 8))
        return -2;
    if (!strncmp(p, "http://", 7))
        p += 7;

    const char* slash = strchr(p, '/');
    const char* colon = strchr(p, ':');
    size_t hlen;

    if (colon && (!slash || colon < slash)) {
        hlen = (size_t)(colon - p);
        int prt = 0;
        const char* d = colon + 1;
        while (*d >= '0' && *d <= '9') {
            prt = prt * 10 + (*d - '0');
            d++;
        }
        if (prt > 0 && prt < 65536)
            *port = (uint16_t)prt;
        slash = (*d == '/') ? d : NULL;
    } else {
        hlen = slash ? (size_t)(slash - p) : strlen(p);
    }

    if (hlen == 0 || hlen >= hostsz)
        return -1;
    memcpy(host, (void*)p, hlen);
    host[hlen] = 0;

    if (slash && slash[0]) {
        if (strlen(slash) >= pathsz)
            return -1;
        strcpy(path, slash);
    }
    return 0;
}

/* Readability-mode HTML to text (NOT a layout engine): block tags become
 * newlines, list items get bullets, links keep their URLs, script/style
 * bodies are dropped, entities (named + numeric) are decoded. */
static void NsEmitChar(char* dst, size_t dstsz, size_t* di, char c) {
    if (*di + 1 >= dstsz)
        return;
    dst[*di] = c;
    if (*di <= NS_BODY_MAX) {
        s_col[*di] = g_runColor;
        s_sz[*di] = g_runSize;
    }
    (*di)++;
}

static void NsEmitNewlines(char* dst, size_t dstsz, size_t* di, int want) {
    int have = 0;
    size_t t = *di;
    while (t > 0 && dst[t - 1] == '\n' && have < want) {
        t--;
        have++;
    }
    while (have < want && *di + 1 < dstsz) {
        NsEmitChar(dst, dstsz, di, '\n');
        have++;
    }
}

static void NsEmitStr(char* dst, size_t dstsz, size_t* di, const char* s) {
    while (*s && *di + 1 < dstsz) {
        if (*s == '\n') {
            NsEmitNewlines(dst, dstsz, di, 1);
        } else {
            NsEmitChar(dst, dstsz, di, *s);
        }
        s++;
    }
}

/* Decode one entity at src[i]=='&'. Returns chars consumed (0 = literal). */
static int NsEntity(const char* src, size_t srclen, size_t i, char* dst, size_t dstsz, size_t* di) {
    static const struct {
        const char* name;
        const char* val;
    } kEnts[] = {{"amp", "&"}, {"lt", "<"}, {"gt", ">"}, {"quot", "\""},
                 {"nbsp", " "}, {"copy", "(c)"}, {"reg", "(R)"}, {"mdash", "-"},
                 {"ndash", "-"}, {"hellip", "..."}, {"trade", "(TM)"}, {NULL, NULL}};
    if (i + 1 < srclen && src[i + 1] == '#') {
        size_t j = i + 2;
        int base = 10;
        if (j < srclen && (src[j] == 'x' || src[j] == 'X')) {
            base = 16;
            j++;
        }
        unsigned v = 0;
        size_t digits = 0;
        while (j < srclen && digits < 6) {
            int d = -1;
            if (src[j] >= '0' && src[j] <= '9')
                d = src[j] - '0';
            else if (base == 16 && src[j] >= 'a' && src[j] <= 'f')
                d = src[j] - 'a' + 10;
            else if (base == 16 && src[j] >= 'A' && src[j] <= 'F')
                d = src[j] - 'A' + 10;
            else
                break;
            v = v * (unsigned)base + (unsigned)d;
            j++;
            digits++;
        }
        if (digits == 0)
            return 0;
        if (j < srclen && src[j] == ';')
            j++;
        if (v > 0 && v < 128)
            NsEmitChar(dst, dstsz, di, (char)v);
        else
            NsEmitChar(dst, dstsz, di, '?');
        return (int)(j - i);
    }
    for (int k = 0; kEnts[k].name; k++) {
        size_t nl = strlen(kEnts[k].name);
        if (i + 1 + nl < srclen && !strncmp(src + i + 1, kEnts[k].name, nl) &&
            src[i + 1 + nl] == ';') {
            NsEmitStr(dst, dstsz, di, kEnts[k].val);
            return (int)(nl + 2);
        }
    }
    return 0;
}

/* attribute="value" within tag text [j,k), case-insensitive name. */
static bool NsTagAttr(const char* src, size_t j, size_t k, const char* name, char* out, size_t outsz) {
    size_t nl = strlen(name);
    for (size_t h = j; h + nl < k; h++) {
        size_t t;
        for (t = 0; t < nl; t++) {
            char a = src[h + t];
            if (a >= 'A' && a <= 'Z')
                a += 32;
            if (a != name[t])
                break;
        }
        if (t != nl)
            continue;
        /* Name must end at a boundary (space, =, /, >). */
        {
            char e = src[h + nl];
            if (e != ' ' && e != '\t' && e != '=' && e != '/' && e != '>')
                continue;
        }
        size_t v = h + nl;
        while (v < k && (src[v] == ' ' || src[v] == '\t' || src[v] == '='))
            v++;
        if (v >= k)
            return false;
        if (src[v] == '"' || src[v] == '\'') {
            char aq = src[v++];
            size_t ol = 0;
            while (v < k && src[v] != aq && ol + 1 < outsz)
                out[ol++] = src[v++];
            out[ol] = 0;
            return true;
        }
        {
            size_t ol = 0;
            while (v < k && src[v] != ' ' && src[v] != '\t' && src[v] != '>' && ol + 1 < outsz)
                out[ol++] = src[v++];
            out[ol] = 0;
            return ol > 0;
        }
    }
    return false;
}

#define NS_CSS_STACK 24

static void NsRunFromTop(NsCssNode** stack, int depth) {
    NsCssNode* top = (depth > 0) ? stack[depth - 1] : NULL;
    if (!top) {
        g_runColor = 0xffffffffu;
        g_runSize = 14;
        return;
    }
    g_runColor = NsCssNodeColor(top);
    g_runSize = (uint8_t)NsCssNodeFontPx(top);
}

/* Collect <style> bodies (cap 8KB) before the render pass. */
static void NsCollectCss(const char* src, size_t srclen) {
    size_t di = 0;
    size_t i = 0;
    s_cssBuf[0] = 0;
    while (i < srclen && di + 1 < sizeof(s_cssBuf)) {
        if (src[i] != '<') {
            i++;
            continue;
        }
        size_t j = i + 1;
        char tag[8];
        int tl = 0;
        while (j < srclen && tl < 7 &&
               (isalpha((unsigned char)src[j]) || isdigit((unsigned char)src[j]))) {
            char tc = src[j];
            if (tc >= 'A' && tc <= 'Z')
                tc += 32;
            tag[tl++] = tc;
            j++;
        }
        tag[tl] = 0;
        if (strcmp(tag, "style") != 0) {
            i++;
            continue;
        }
        /* Quote-aware end of the open tag. */
        bool q = false;
        char qc = 0;
        while (j < srclen && (src[j] != '>' || q)) {
            if (!q && (src[j] == '"' || src[j] == '\'')) {
                q = true;
                qc = src[j];
            } else if (q && src[j] == qc) {
                q = false;
            }
            j++;
        }
        if (j < srclen)
            j++;
        /* Copy until </style (case-insensitive). */
        while (j < srclen && di + 1 < sizeof(s_cssBuf)) {
            if (src[j] == '<' && j + 7 < srclen && src[j + 1] == '/' &&
                (src[j + 2] == 's' || src[j + 2] == 'S') &&
                (src[j + 3] == 't' || src[j + 3] == 'T') &&
                (src[j + 4] == 'y' || src[j + 4] == 'Y') &&
                (src[j + 5] == 'l' || src[j + 5] == 'L') &&
                (src[j + 6] == 'e' || src[j + 6] == 'E')) {
                while (j < srclen && src[j] != '>')
                    j++;
                if (j < srclen)
                    j++;
                break;
            }
            s_cssBuf[di++] = src[j++];
        }
        s_cssBuf[di] = 0;
        i = j;
    }
    s_cssBuf[di] = 0;
}

static void NsHtmlToText(const char* src, size_t srclen, char* dst, size_t dstsz) {
    size_t di = 0;
    size_t i = 0;
    int skip = 0; /* 1 = inside <script>, 2 = inside <style> */
    char href[512];
    href[0] = 0;
    NsCssNode* stack[NS_CSS_STACK];
    int depth = 0;

    stack[0] = NsCssNodeCreate(NULL, "body");
    if (stack[0]) {
        NsCssSelect(stack[0]);
        depth = 1;
    }
    NsRunFromTop(stack, depth);

    while (i < srclen && di + 1 < dstsz) {
        char c = src[i];
        if (c == '<') {
            /* Comments first: they may contain '>' inside. */
            if (i + 3 < srclen && src[i + 1] == '!' && src[i + 2] == '-' && src[i + 3] == '-') {
                i += 4;
                while (i + 2 < srclen && !(src[i] == '-' && src[i + 1] == '-' && src[i + 2] == '>'))
                    i++;
                i += 3;
                continue;
            }
            /* Other declarations (<!DOCTYPE ...>): not elements,
             * skip to '>' without creating style nodes. */
            if (i + 1 < srclen && src[i + 1] == '!') {
                size_t j = i + 2;
                while (j < srclen && src[j] != '>')
                    j++;
                i = (j < srclen) ? j + 1 : srclen;
                continue;
            }
            if (skip) {
                /* Only the matching close tag ends the skip: any other
                 * '<' inside (e.g. JS `a < b`) is content, and parsing it
                 * as a tag would swallow the real </script> terminator. */
                const char* want = (skip == 1) ? "script" : "style";
                size_t j = i + 1;
                bool isClose = (j < srclen && src[j] == '/');
                if (isClose)
                    j++;
                size_t n = strlen(want);
                bool match = isClose;
                for (size_t t = 0; match && t < n; t++) {
                    if (j + t >= srclen || tolower((unsigned char)src[j + t]) != want[t])
                        match = false;
                }
                if (!match) {
                    i++;
                    continue;
                }
            }
            size_t j = i + 1;
            bool closing = false;
            if (j < srclen && src[j] == '/') {
                closing = true;
                j++;
            }
            char tag[16];
            int tl = 0;
            while (j < srclen && tl < 15 &&
                   (isalpha((unsigned char)src[j]) || isdigit((unsigned char)src[j]))) {
                tag[tl++] = (char)tolower((unsigned char)src[j]);
                j++;
            }
            tag[tl] = 0;
            /* Tag extent, quote-aware, for href capture. */
            bool q = false;
            char qc = 0;
            size_t k = j;
            while (k < srclen && (src[k] != '>' || q)) {
                if (!q && (src[k] == '"' || src[k] == '\'')) {
                    q = true;
                    qc = src[k];
                } else if (q && src[k] == qc) {
                    q = false;
                }
                k++;
            }
            if (skip) {
                if (closing && ((skip == 1 && !strcmp(tag, "script")) ||
                                (skip == 2 && !strcmp(tag, "style"))))
                    skip = 0;
                i = (k < srclen) ? k + 1 : srclen;
                continue;
            }
            if (!closing && (!strcmp(tag, "script") || !strcmp(tag, "style"))) {
                skip = !strcmp(tag, "script") ? 1 : 2;
                i = (k < srclen) ? k + 1 : srclen;
                continue;
            }
            bool isH = (tag[0] == 'h' && tag[1] >= '1' && tag[1] <= '6' && tag[2] == 0);
            if (!closing) {
                /* href first: the <a> node needs the link flag at creation. */
                if (!strcmp(tag, "a")) {
                    href[0] = 0;
                    for (size_t h = j; h < k; h++) {
                        if ((h + 4 < k) &&
                            (src[h] == 'h' || src[h] == 'H') && (src[h + 1] == 'r' || src[h + 1] == 'R') &&
                            (src[h + 2] == 'e' || src[h + 2] == 'E') && (src[h + 3] == 'f' || src[h + 3] == 'F')) {
                            size_t v = h + 4;
                            while (v < k && (src[v] == ' ' || src[v] == '\t' || src[v] == '='))
                                v++;
                            while (v < k && (src[v] == ' ' || src[v] == '\t'))
                                v++;
                            if (v < k && (src[v] == '"' || src[v] == '\'')) {
                                char hq = src[v++];
                                size_t hl = 0;
                                while (v < k && src[v] != hq && hl + 1 < sizeof(href))
                                    href[hl++] = src[v++];
                                href[hl] = 0;
                            }
                            break;
                        }
                    }
                }
                /* Push the element, select its style, adopt the run.
                 * Empty tag names select nothing. */
                if (tag[0] != 0 && depth < NS_CSS_STACK) {
                    NsCssNode* el = NsCssNodeCreate(depth > 0 ? stack[depth - 1] : NULL, tag);
                    if (el) {
                        char attr[1024];
                        if (NsTagAttr(src, j, k, "class", attr, sizeof(attr))) {
                            char* tok = attr;
                            while (*tok && el->n_cls < NS_CSS_MAX_CLASSES) {
                                while (*tok == ' ' || *tok == '\t')
                                    tok++;
                                if (!*tok)
                                    break;
                                char* end = tok;
                                while (*end && *end != ' ' && *end != '\t')
                                    end++;
                                char save = *end;
                                *end = 0;
                                NsCssNodeAddClass(el, tok);
                                *end = save;
                                tok = end;
                            }
                        }
                        if (NsTagAttr(src, j, k, "id", attr, sizeof(attr)))
                            NsCssNodeSetId(el, attr);
                        if (!strcmp(tag, "a") && href[0])
                            NsCssNodeSetLink(el, true);
                        if (NsTagAttr(src, j, k, "style", attr, sizeof(attr)))
                            NsCssNodeSetInline(el, attr);
                        NsCssSelect(el);
                        stack[depth++] = el;
                        NsRunFromTop(stack, depth);
                    }
                }
                if (!strcmp(tag, "br")) {
                    NsEmitNewlines(dst, dstsz, &di, 1);
                } else if (!strcmp(tag, "p") || !strcmp(tag, "div") || !strcmp(tag, "tr") ||
                           !strcmp(tag, "ul") || !strcmp(tag, "ol") || isH ||
                           !strcmp(tag, "title")) {
                    NsEmitNewlines(dst, dstsz, &di, 1);
                    if (!strcmp(tag, "title"))
                        NsEmitStr(dst, dstsz, &di, "# ");
                } else if (!strcmp(tag, "li")) {
                    NsEmitNewlines(dst, dstsz, &di, 1);
                    NsEmitStr(dst, dstsz, &di, "- ");
                } else if (!strcmp(tag, "hr")) {
                    NsEmitNewlines(dst, dstsz, &di, 1);
                    NsEmitStr(dst, dstsz, &di, "---");
                    NsEmitNewlines(dst, dstsz, &di, 1);
                }
            } else {
                /* Pop the element; author vertical margins add spacing. */
                int mlines = 0;
                if (depth > 1) {
                    mlines = NsCssNodeMarginLines(stack[depth - 1]);
                    NsCssNodeDestroy(stack[depth - 1]);
                    depth--;
                    NsRunFromTop(stack, depth);
                }
                if (!strcmp(tag, "p") || !strcmp(tag, "div") || isH || !strcmp(tag, "title") ||
                    !strcmp(tag, "tr")) {
                    NsEmitNewlines(dst, dstsz, &di, 2);
                } else if (!strcmp(tag, "li") || !strcmp(tag, "ul") || !strcmp(tag, "ol")) {
                    NsEmitNewlines(dst, dstsz, &di, 1);
                } else if (!strcmp(tag, "a")) {
                    if (href[0]) {
                        NsEmitStr(dst, dstsz, &di, " (");
                        NsEmitStr(dst, dstsz, &di, href);
                        NsEmitStr(dst, dstsz, &di, ")");
                        href[0] = 0;
                    }
                }
                for (int t = 0; t < mlines; t++)
                    NsEmitChar(dst, dstsz, &di, '\n');
            }
            i = (k < srclen) ? k + 1 : srclen;
            continue;
        }
        if (skip) {
            i++;
            continue;
        }
        if (c == '&') {
            int used = NsEntity(src, srclen, i, dst, dstsz, &di);
            if (used > 0) {
                i += (size_t)used;
                continue;
            }
        }
        if (c == '\r' || c == '\t') {
            i++;
            continue;
        }
        if (c == '\n') {
            NsEmitNewlines(dst, dstsz, &di, 1);
            i++;
            continue;
        }
        NsEmitChar(dst, dstsz, &di, c);
        i++;
    }
    while (depth > 0)
        NsCssNodeDestroy(stack[--depth]);
    dst[di] = 0;
}


static int NsFetch(const char* url) {
    char host[128];
    char path[256];
    uint16_t port = 80;
    memset(host, 0, sizeof(host));
    memset(path, 0, sizeof(path));

    int pu = NsParseUrl(url, host, sizeof(host), path, sizeof(path), &port);
    if (pu == -2) {
        NsShowStatus("HTTPS is not supported yet.\n\nXeneva has no TLS stack; "
                     "the mbedTLS port lands before encrypted fetch.\nTry an http:// URL.");
        return -1;
    }
    if (pu < 0 || !host[0]) {
        NsShowStatus("Bad URL.\n\nTry: example.com or http://example.com/");
        return -1;
    }

    char status[NS_URL_MAX + 32];
    snprintf(status, sizeof(status), "Fetching http://%s%s ...", host, path);
    NsShowStatus(status);

    hostent* ent = gethostbyname(host);
    if (!ent) {
        snprintf(status, sizeof(status), "DNS failed for %s.\n\nIs netmngr running?", host);
        NsShowStatus(status);
        return -1;
    }
    /* gethostbyname falls back to IPv6 (16-byte h_addr) when no A record
     * exists. Our socket path is IPv4-only: refuse cleanly instead of
     * connecting to the first 4 bytes of an IPv6 address. */
    if (ent->h_addrtype != AF_INET || ent->h_length != 4 || !ent->h_addr_list[0]) {
        snprintf(status, sizeof(status), "No IPv4 address for %s.\n\nIPv6-only hosts are not reachable yet.", host);
        NsShowStatus(status);
        return -1;
    }
    uint32_t ipaddr = *(uint32_t*)ent->h_addr_list[0];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        NsShowStatus("socket() failed.");
        return -1;
    }

    sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    memcpy(&dest.sin_addr, &ipaddr, sizeof(uint32_t));

    if (connect(sock, (sockaddr_*)&dest, sizeof(dest)) < 0) {
        snprintf(status, sizeof(status), "Connect to %s:%u failed.", host, port);
        NsShowStatus(status);
        _KeCloseFile(sock);
        return -1;
    }
    _KePrint("[css] connected\n");

    char req[512];
    memset(req, 0, sizeof(req));
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: netsurf/xeneva\r\n"
             "Accept: */*\r\nConnection: close\r\n\r\n",
             path, host);

    if (sendto(sock, req, strlen(req), 0, (sockaddr*)&dest, sizeof(dest)) < 0) {
        NsShowStatus("Send failed.");
        _KeCloseFile(sock);
        return -1;
    }

    memset(s_body, 0, sizeof(s_body));
    size_t got = 0;
    int idle = 0;
    int got_any = 0;
    char chunk[2048];
    while (idle < 80 && got < NS_BODY_MAX) {
        memset(chunk, 0, sizeof(chunk));
        size_t want = sizeof(chunk) - 1;
        if (want > NS_BODY_MAX - got)
            want = NS_BODY_MAX - got;
        int n = recvfrom(sock, chunk, want, 0, NULL, NULL);
        if (n > 0) {
            got_any = 1;
            idle = 0;
            memcpy(s_body + got, chunk, (size_t)n);
            got += (size_t)n;
            continue;
        }
        if (n == 0 && got_any)
            break;
        _KeProcessSleep(100);
        idle++;
    }
    _KeCloseFile(sock);

    if (!got_any) {
        NsShowStatus("No data (timeout).");
        return -1;
    }
    s_body[got] = 0;

    /* Skip HTTP headers. */
    const char* body = s_body;
    const char* hdr_end = strstr(s_body, "\r\n\r\n");
    if (hdr_end)
        body = hdr_end + 4;

    NsCollectCss(body, got - (size_t)(body - s_body));
    _KePrint("[css] author sheet %d bytes\n", (int)strlen(s_cssBuf));
    NsCssBegin();
    _KePrint("[css] begin ok\n");
    NsCssAddSheet(s_cssBuf, strlen(s_cssBuf));
    _KePrint("[css] sheet added\n");
    NsHtmlToText(body, got - (size_t)(body - s_body), s_text, sizeof(s_text));
    _KePrint("[css] rendered %d bytes\n", (int)strlen(s_text));
    NsCssEnd();
    if (!s_text[0])
        strcpy(s_text, "(empty page)");
    NsShowStatus(s_text);
    return 0;
}

static void NsGoHandler(ChWidget* wid, ChWindow* win) {
    (void)wid;
    (void)win;
    NsFetch(s_url);
}

static void NsUrlClampCursor(void) {
    size_t len = strlen(s_url);
    if (s_urlCursor > len)
        s_urlCursor = len;
}

static void NsHandleKey(int raw) {
    /* URL line navigation. Up/Down/PgUp/PgDn scroll the page instead. */
    if (raw == NS_KEY_LEFT || raw == NS_KEY_HOME) {
        NsUrlClampCursor();
        if (raw == NS_KEY_HOME)
            s_urlCursor = 0;
        else if (s_urlCursor > 0)
            s_urlCursor--;
        NsShowUrl();
        return;
    }
    if (raw == NS_KEY_RIGHT || raw == NS_KEY_END) {
        NsUrlClampCursor();
        if (raw == NS_KEY_END)
            s_urlCursor = strlen(s_url);
        else if (s_urlCursor < strlen(s_url))
            s_urlCursor++;
        NsShowUrl();
        return;
    }
    if (raw == NS_KEY_UP) {
        NsPageScroll(-1);
        return;
    }
    if (raw == NS_KEY_DOWN) {
        NsPageScroll(1);
        return;
    }
    if (raw == NS_KEY_PGUP) {
        NsPageScroll(-(NS_PAGE_LINES - 1));
        return;
    }
    if (raw == NS_KEY_PGDN) {
        NsPageScroll(NS_PAGE_LINES - 1);
        return;
    }
    /* Terminal's pattern: GetKeyPress for special keys, KeyToASCII for
     * printables. Releases fall through to 0 and are ignored. */
    char rawkey = ChitralekhaGetKeyPress(raw);
    if (rawkey == KEY_RETURN) { /* Enter -> fetch */
        NsFetch(s_url);
        return;
    }
    if (rawkey == KEY_BACKSPACE) {
        NsUrlClampCursor();
        if (s_urlCursor > 0) {
            size_t len = strlen(s_url);
            memmove(s_url + s_urlCursor - 1, s_url + s_urlCursor, len - s_urlCursor + 1);
            s_urlCursor--;
            NsShowUrl();
        }
        return;
    }
    int c = ChitralekhaKeyToASCII(raw);
    if (c >= 32 && c < 127) {
        NsUrlClampCursor();
        size_t len = strlen(s_url);
        if (len + 1 < sizeof(s_url)) {
            memmove(s_url + s_urlCursor + 1, s_url + s_urlCursor, len - s_urlCursor + 1);
            s_url[s_urlCursor] = (char)c;
            s_urlCursor++;
            NsShowUrl();
        }
    }
}

static void WindowHandleMessage(PostEvent* e) {
    switch (e->type) {
    case DEODHAI_REPLY_MOUSE_EVENT: {
        int handle = e->dword4;
        ChWindow* mouseWin = ChGetWindowByHandle(s_mainWin, handle);
        ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
        memset(e, 0, sizeof(PostEvent));
        break;
    }
    case DEODHAI_REPLY_KEY_EVENT: {
        int code = e->dword;
        ChitralekhaProcessKey(code);
        NsHandleKey(code);
        memset(e, 0, sizeof(PostEvent));
        break;
    }
    case DEODHAI_REPLY_FOCUS_CHANGED: {
        int focus_val = e->dword;
        int handle = e->dword2;
        ChWindow* focWin = ChGetWindowByHandle(s_mainWin, handle);
        if (focWin)
            ChWindowHandleFocus(focWin, focus_val, handle);
        memset(e, 0, sizeof(PostEvent));
        break;
    }
    default:
        memset(e, 0, sizeof(PostEvent));
        break;
    }
}

int main(int argc, char* argv[]) {
    /* Headless pipeline self-test: fetch + CSS + render with serial
     * markers, no GUI. Xeneva drops the program name: the app sees only
     * user args, so scan everything (argv[0] is already "--selftest"
     * under init's TERM runner). */
    for (int a = 0; a < argc; a++) {
        if (argv[a] && !strcmp(argv[a], "--selftest") && a + 1 < argc && argv[a + 1] &&
            argv[a + 1][0]) {
            char turl[NS_URL_MAX];
            strncpy(turl, argv[a + 1], sizeof(turl) - 1);
            turl[sizeof(turl) - 1] = 0;
            _KePrint("[cssselftest] fetch %s\n", turl);
            int trc = -1;
            for (int retry = 0; retry < 6 && trc != 0; retry++) {
                if (retry > 0) {
                    _KePrint("[cssselftest] retry %d\n", retry);
                    _KeProcessSleep(2000);
                }
                trc = NsFetch(turl);
            }
            if (trc != 0) {
                _KePrint("[cssselftest] FAIL: fetch never completed\n");
                return 1;
            }
            _KePrint("[cssselftest] done, text %d bytes, first 120 chars:\n",
                     (int)strlen(s_text));
            {
                char head[121];
                strncpy(head, s_text, 120);
                head[120] = 0;
                _KePrint("%s\n", head);
            }
            _KePrint("[cssselftest] PASS\n");
            return 0;
        }
    }
    for (int a = 0; a < argc; a++) {
        if (argv[a] && argv[a][0] && argv[a][0] != '-') {
            strncpy(s_url, argv[a], sizeof(s_url) - 1);
            s_url[sizeof(s_url) - 1] = 0;
            break;
        }
    }
    s_urlCursor = strlen(s_url);

    s_app = ChitralekhaStartApp(argc, argv);
    s_mainWin = ChCreateWindow(s_app, WINDOW_FLAG_MOVABLE, "NetSurf", 100, 80, 640, 480);
    s_mainWin->color = 0xFF252525;

    s_urlBox = ChCreateTextBox(s_mainWin, 4, 2, 640 - 92, 24);
    s_urlBox->editable = true;
    s_urlBox->textBackgroundColor = 0xFFFFFFFF;
    s_urlBox->textColor = 0xFF000000;
    ChFont* font = ChInitialiseFont(CALIBRI);
    ChTextBoxSetFont(s_urlBox, font);
    ChTextBoxSetFontSize(s_urlBox, 14);
    s_urlBox->wid.ChPaintHandler = NsUrlPaint;
    ChWindowAddWidget(s_mainWin, (ChWidget*)s_urlBox);

    ChButton* go = ChCreateButton(640 - 82, 2, 74, 24, (char*)"Go");
    go->base.ChActionHandler = NsGoHandler;
    ChWindowAddWidget(s_mainWin, (ChWidget*)go);

    s_pageBox = ChCreateTextBox(s_mainWin, 4, 30, 640 - 8, 480 - 60);
    s_pageBox->editable = false;
    s_pageBox->textBackgroundColor = s_mainWin->color;
    s_pageBox->textColor = WHITE;
    ChTextBoxSetFont(s_pageBox, font);
    ChTextBoxSetFontSize(s_pageBox, 14);
    s_pageBox->wid.ChPaintHandler = NsPagePaint;
    ChWindowAddWidget(s_mainWin, (ChWidget*)s_pageBox);

    /* Second size bucket for CSS-enlarged runs (headings). Per-object
     * atlases, so sizes coexist safely. */
    s_fontBig = ChInitialiseFont(CALIBRI);
    if (s_fontBig)
        ChFontSetSize(s_fontBig, 20);

    NsShowUrl();
    NsShowStatus("NetSurf for XenevaOS (bootstrap).\n\nType a host like example.com,\n"
                 "press Enter or Go.\n\nLeft/Right/Home/End move in the URL bar.\n"
                 "Up/Down/PgUp/PgDn scroll the page.\n\nHTTP only -- HTTPS needs the TLS port.");

    ChWindowPaint(s_mainWin);

    PostEvent e;
    memset(&e, 0, sizeof(PostEvent));

    setjmp(s_mainWin->jump);
    while (1) {
        int err = _KeFileIoControl(s_app->postboxfd, POSTBOX_GET_EVENT, &e);
        WindowHandleMessage(&e);
        if (err == POSTBOX_NO_EVENT)
            _KePauseThread();
    }
    return 0;
}
