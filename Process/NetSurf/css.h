/**
 * NetSurf CSS wrapper for XenevaOS (Phase 1+2).
 *
 * Thin element tree + libcss select client. The HTML pass builds NsCssNodes
 * as it scans tags; each node is selected against the UA + author sheets on
 * open (ancestors are always selected first, so inheritance composes).
 * Inline style="" attributes parse to per-node author sheets.
 *
 * Exposes only what the readability renderer needs: text color
 * (0xAARRGGBB, libcss css_color layout matches Chitralekha pixels),
 * font size in px, and extra blank lines from vertical margins.
 */

#ifndef NS_CSS_H
#define NS_CSS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NS_CSS_MAX_CLASSES 8
#define NS_CSS_CLASS_LEN 31
#define NS_CSS_ID_LEN 63
#define NS_CSS_TAG_LEN 15

typedef struct NsCssNode {
    struct NsCssNode* parent;
    char tag[NS_CSS_TAG_LEN + 1];
    char cls[NS_CSS_MAX_CLASSES][NS_CSS_CLASS_LEN + 1];
    uint32_t n_cls;
    char id[NS_CSS_ID_LEN + 1];
    bool is_link;
    void* lwcName;
    void* lwcCls[NS_CSS_MAX_CLASSES];
    void* lwcId;
    void* inlineSheet;
    void* selectResults;
} NsCssNode;

/* Per-page lifecycle: begin, add author <style> sheets, end. A UA sheet
 * with reader defaults (body white, headings sized, links blue) is
 * installed by NsCssBegin. */
int NsCssBegin(void);
int NsCssAddSheet(const char* data, size_t len);
void NsCssEnd(void);

NsCssNode* NsCssNodeCreate(NsCssNode* parent, const char* tag);
void NsCssNodeAddClass(NsCssNode* n, const char* cls);
void NsCssNodeSetId(NsCssNode* n, const char* id);
void NsCssNodeSetLink(NsCssNode* n, bool link);
/* Parse a style="" attribute body into a per-node author sheet. */
int NsCssNodeSetInline(NsCssNode* n, const char* style);
/* Select + inherit-compose n. Parent must already be selected. */
int NsCssSelect(NsCssNode* n);
void NsCssNodeDestroy(NsCssNode* n);

uint32_t NsCssNodeColor(NsCssNode* n);
int NsCssNodeFontPx(NsCssNode* n);
int NsCssNodeMarginLines(NsCssNode* n);
/* True when the element must not contribute text: display:none or
 * visibility:hidden/collapse. */
int NsCssNodeSkip(NsCssNode* n);
/* 1 when text-align computes to center. */
int NsCssNodeCenter(NsCssNode* n);
/* 1 when this element starts a new line (block, table, flex, grid).
 * Inline, inline-block, and table-cell stay on the current line.
 * Children of a table-row or flex row also stay on that line. */
int NsCssNodeBlock(NsCssNode* n);
/* 0 when the background is transparent. Otherwise 0xAARRGGBB. */
uint32_t NsCssNodeBackground(NsCssNode* n);

/* Bits the painter applies. Everything else in the computed style either
 * has no face to switch to (font-family) or no box tree (position). */
#define NS_CSS_BOLD 1
#define NS_CSS_ITALIC 2
#define NS_CSS_UNDER 4
#define NS_CSS_STRIKE 8
#define NS_CSS_OVER 16

typedef struct NsCssUsed {
	uint8_t decor;
	uint8_t align; /* 0 left, 1 center, 2 right */
	uint8_t indent;
	uint8_t line; /* row step in px */
	uint8_t list; /* 0 none, 1 disc, 2 circle, 3 square, 4 decimal, 5 alpha */
	uint8_t upper; /* 1 uppercase, 2 lowercase */
	uint8_t pre;
	uint8_t nowrap;
	uint16_t width; /* 0 = auto */
	uint8_t opacity; /* 255 = solid */
	uint32_t border; /* 0, or the bottom-border color */
} NsCssUsed;

void NsCssNodeUsed(NsCssNode* n, NsCssUsed* out);

#ifdef __cplusplus
}
#endif

#endif
