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

#define NS_CSS_MAX_CLASSES 4
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

#ifdef __cplusplus
}
#endif

#endif
