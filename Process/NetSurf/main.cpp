/**
 * NetSurf bootstrap browser for XenevaOS.
 *
 * Staging frontend, not upstream NetSurf core yet: a Chitralekha window with
 * a URL bar + Go button, HTTP/1.1 fetch (TLS on https://), and crude
 * HTML-to-text rendering into a textbox.
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
#include "js.h"

#include "pngdec.h"
#include "svgdec.h"
#include <nanojpg.h>
#include <new>
#include <pthread.h>
#include <https.h>

extern "C" void xe_heap_check(const char* tag);

#define NS_URL_MAX	256
#define NS_BODY_MAX (256 * 1024)
/* Watch-page player JSON starts after ~780 KiB. The text view stays smaller. */
#define NS_RAW_MAX (1400 * 1024)
/* Visible page lines: 420px box at the painter's 19px line stride. */
#define NS_PAGE_LINES 22

/* Extended key codes (0xE0-prefixed AT Set-1, as the virtio-kbd driver
 * forwards them -- same encoding Doom's platform layer decodes). */
#define NS_KEY_LEFT	 0xE04B
#define NS_KEY_RIGHT 0xE04D
#define NS_KEY_UP	 0xE048
#define NS_KEY_DOWN	 0xE050
#define NS_KEY_HOME	 0xE047
#define NS_KEY_END	 0xE04F
#define NS_KEY_PGUP	 0xE049
#define NS_KEY_PGDN	 0xE051

static ChitralekhaApp* s_app = NULL;
static ChWindow* s_mainWin = NULL;
static ChTextBox* s_urlBox = NULL;
static ChTextBox* s_pageBox = NULL;

static char s_url[NS_URL_MAX] = "https://example.com/";
static size_t s_urlCursor = 0;
static char s_body[NS_RAW_MAX + 1];
static char s_text[NS_BODY_MAX + 1];
static char s_view[NS_BODY_MAX + 1];
static int s_pageTop = 0;
static size_t s_viewStart = 0;
static ChFont* s_fontBig = NULL;
/* Per-character style for s_text, recorded during the HTML pass. */
static uint32_t s_col[NS_BODY_MAX + 1];
static uint32_t s_bg[NS_BODY_MAX + 1];
static uint8_t s_sz[NS_BODY_MAX + 1];
static uint8_t s_fl[NS_BODY_MAX + 1];
/* Author CSS (inline <style> + fetched <link rel=stylesheet>) collected
 * before the render pass. */
#define NS_CSS_MAX (192 * 1024)
static char s_cssBuf[NS_CSS_MAX];
/* Current emission run, refreshed from the element stack top. */
static uint32_t g_runColor = 0xff202124u;
static uint32_t g_runBg = 0;
static uint8_t g_runSize = 14;
static uint8_t g_runDecor = 0;
static uint8_t g_runAlign = 0;
static uint8_t g_runIndent = 0;
static uint8_t g_runLine = 19;
static uint8_t g_runCase = 0;
static uint8_t g_runPre = 0;
static uint8_t g_runNowrap = 0;
static uint16_t g_runWidth = 0;
static uint32_t g_runBorder = 0;
static int g_linePx = 0;
static int s_liNo = 0;

static uint32_t NsMixOpacity(uint32_t fg, uint32_t bg, uint8_t a) {
	int i;
	uint32_t out = 0xff000000u;
	if (a == 255)
		return fg;
	for (i = 0; i < 3; i++) {
		int shift = i * 8;
		int f = (int)((fg >> shift) & 255);
		int b = (int)((bg >> shift) & 255);
		int m = (f * a + b * (255 - a)) / 255;
		out |= (uint32_t)m << shift;
	}
	return out;
}
static int g_runCenter = 0;
static uint32_t g_pageBg = 0xffffffffu;
static int g_pageCenter = 0;
static int s_googleHome = 0;
#define NS_IMG_MAX 12
static uint32_t* s_imgPx[NS_IMG_MAX];
static int s_imgW[NS_IMG_MAX];
static int s_imgH[NS_IMG_MAX];
static char s_imgUrl[NS_IMG_MAX][512];
static int s_nimg = 0;

/* English stand-in for a link whose visible text is not Latin (hl=hi, …). */
static const char* s_latinLink = NULL;
static int s_latinUsed = 0;
/* Painted search field. Click focuses it; typing does not go to the URL bar. */
static char s_query[240];
static size_t s_qCursor = 0;
static int s_qFocus = 0;
static int s_qHave = 0;
static int s_qX, s_qY, s_qW, s_qH;
static int s_btnHave = 0;
static int s_btnX, s_btnY, s_btnW, s_btnH;
static int s_luckyHave = 0;
static int s_luckyX, s_luckyY, s_luckyW, s_luckyH;
/* Anchors in s_text. x/y/w/h are filled while painting the visible slice. */
#define NS_LINK_MAX 48
static struct {
	size_t a, b;
	char href[320];
	int x, y, w, h, have;
} s_links[NS_LINK_MAX];
static int s_nlinks = 0;
static int NsFetch(char* url);
static void NsResolveUrl(const char* base, const char* href, char* out, size_t outsz);
static void NsEncodeQuery(const char* in, char* out, size_t outsz);
static const char kNsUa[] =
	"Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";

/* ---------------- JS host plumbing (Ports/quickjs via js.c) ----------------
 * js.c calls the sink for document.write and console lines. Mid-parse the
 * output lands at the live parse position under the current run style;
 * after the parse (timers) it appends to the page text end. */
static char* s_outDst = NULL;
static size_t s_outSz = 0;
static size_t* s_outDi = NULL;
static bool s_outLive = false;  /* NsHtmlToText emission target is live */
static bool s_postPage = false; /* parse finished: sink appends instead */
static bool g_inWrite = false;  /* parsing a document.write fragment */
static int g_writeDepth = 0;    /* fragment recursion guard (max 3) */

/* Style-array indirection: a fragment parse styles into a scratch buffer
 * instead of clobbering page prefix styles, since NsEmitChar indexes the
 * arrays with the local (restart-at-zero) di. */
static uint32_t* g_colCur = s_col;
static uint32_t* g_bgCur = s_bg;
static uint8_t* g_szCur = s_sz;
static uint8_t* g_flCur = s_fl;
static size_t g_colMaxCur = NS_BODY_MAX;

/* Console colors (0xAARRGGBB, matching the CSS pass values). */
#define NS_JS_COL_LOG 0xffaaaaaau
#define NS_JS_COL_WARN 0xffe0a030u
#define NS_JS_COL_ERR 0xffff5050u

/* document.write fragment conversion buffers, one pair per depth level:
 * nested writes parse while the outer parse is suspended, so static
 * buffers must not be shared across levels. */
#define NS_FRAG_MAX 8192
static char s_fragSrc[3][NS_FRAG_MAX + 1];
static char s_fragOut[3][NS_FRAG_MAX + 1];
/* Fragment parses write styles here; the converted output is re-emitted
 * under the run style, so the per-char styles are never read back. */
static uint32_t s_fragJunkCol[NS_FRAG_MAX + 1];
static uint32_t s_fragJunkBg[NS_FRAG_MAX + 1];
static uint8_t s_fragJunkFl[NS_FRAG_MAX + 1];
static uint8_t s_fragJunkSz[NS_FRAG_MAX + 1];

/* Lines that fit in the page box. The painter steps 19px from a baseline
 * 19px under the box top (56px when the Google wordmark is drawn). */
static int NsVisibleLines(void) {
	int origin = s_googleHome ? 56 : 19;
	int h;
	int n;
	if (!s_pageBox)
		return NS_PAGE_LINES;
	h = s_pageBox->wid.h - origin;
	if (h < 19)
		h = 19;
	n = h / 19;
	if (n < 1)
		n = 1;
	return n;
}

static void NsPageRender(void) {
	if (!s_pageBox || !s_mainWin)
		return;
	/* Clamp the first visible line, then slice one viewport. */
	int vis = NsVisibleLines();
	int total = 0;
	if (s_text[0]) {
		total = 1;
		for (const char* p = s_text; *p; p++)
			if (*p == '\n')
				total++;
	}
	int maxTop = total - vis;
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
		if (shown >= vis)
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
	uint32_t bg = g_pageBg ? g_pageBg : 0xffffffffu;
	s_qHave = 0;
	s_btnHave = 0;
	s_luckyHave = 0;
	for (int li = 0; li < s_nlinks; li++)
		s_links[li].have = 0;
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
	if (s_googleHome && tb->font) {
		static const char* letters = "Google";
		static const uint32_t cols[6] = {0xff4285f4u, 0xffea4335u, 0xfffbbc05u,
										 0xff4285f4u, 0xff34a853u, 0xffea4335u};
		int tw = 0;
		int i;
		for (i = 0; letters[i]; i++)
			tw += ChFontGetWidthChar(s_fontBig ? s_fontBig : tb->font, letters[i]);
		int x = tb->wid.x + (tb->wid.w - tw) / 2;
		int y = tb->wid.y + 36;
		ChFont* logo = s_fontBig ? s_fontBig : tb->font;
		for (i = 0; letters[i]; i++) {
			ChFontDrawCharClipped(win->canv, logo, letters[i], x, y, cols[i], &clip);
			x += ChFontGetWidthChar(logo, letters[i]);
		}
		peny = tb->wid.y + 56;
	}
	int lineMax = 0;
	int lineX0 = penx;
	for (size_t k = 0; s_view[k]; k++) {
		char c = s_view[k];
		size_t si = s_viewStart + k;
		uint32_t col = (si <= NS_BODY_MAX) ? s_col[si] : 0xff202124u;
		uint32_t runBg = (si <= NS_BODY_MAX) ? s_bg[si] : 0;
		uint8_t szpx = (si <= NS_BODY_MAX) ? (uint8_t)(s_sz[si] & 0x7f) : 14;
		uint8_t fl = (si <= NS_BODY_MAX) ? s_fl[si] : 0;
		if ((k == 0 || s_view[k - 1] == '\n') && c != '\n') {
			uint8_t raw0 = (si <= NS_BODY_MAX) ? s_sz[si] : 0;
			int indent = 0;
			int align = (fl >> 5) & 3;
			if (k > 0 && si > 0 && si - 1 <= NS_BODY_MAX) {
				indent = s_sz[si - 1];
				if (indent > 120)
					indent = 0;
				lineMax = (int)(s_col[si - 1] >> 8);
			} else {
				lineMax = 0;
			}
			int lineCenter = g_pageCenter || (raw0 & 0x80) || align == 1;
			if (lineCenter) {
				int width = 0;
				for (size_t t = k; s_view[t] && s_view[t] != '\n'; t++) {
					size_t si2 = s_viewStart + t;
					uint8_t sz2 = (si2 <= NS_BODY_MAX) ? (uint8_t)(s_sz[si2] & 0x7f) : 14;
					ChFont* f2 = (sz2 >= 17 && s_fontBig) ? s_fontBig : tb->font;
					width += ChFontGetWidthChar(f2, s_view[t]);
				}
				penx = tb->wid.x + (tb->wid.w - width) / 2;
				if (penx < tb->wid.x + 4)
					penx = tb->wid.x + 4;
			} else if (align == 2) {
				int width = 0;
				for (size_t t = k; s_view[t] && s_view[t] != '\n'; t++) {
					size_t si2 = s_viewStart + t;
					uint8_t sz2 = (si2 <= NS_BODY_MAX) ? (uint8_t)(s_sz[si2] & 0x7f) : 14;
					ChFont* f2 = (sz2 >= 17 && s_fontBig) ? s_fontBig : tb->font;
					width += ChFontGetWidthChar(f2, s_view[t]);
				}
				penx = tb->wid.x + tb->wid.w - width - 4;
				if (penx < tb->wid.x + 4 + indent)
					penx = tb->wid.x + 4 + indent;
			} else {
				penx = tb->wid.x + 4 + indent;
			}
			lineX0 = penx;
			{
				const char* line = s_view + k;
				size_t llen = 0;
				while (line[llen] && line[llen] != '\n')
					llen++;
				if (s_googleHome && llen == 12 && !strncmp(line, "Gmail Images", 12)) {
					int width = 0;
					int x;
					for (size_t t = 0; t < llen; t++)
						width += ChFontGetWidthChar(tb->font, line[t]);
					x = tb->wid.x + tb->wid.w - width - 88;
					if (x < tb->wid.x + 4)
						x = tb->wid.x + 4;
					for (size_t t = 0; t < llen; t++) {
						ChFontDrawCharClipped(win->canv, tb->font, line[t], x, tb->wid.y + 22,
											  0xff202124u, &clip);
						x += ChFontGetWidthChar(tb->font, line[t]);
					}
					k += llen;
					continue;
				}
				if (s_googleHome && llen == 7 && !strncmp(line, "Sign in", 7)) {
					int bw = 72;
					int bx = tb->wid.x + tb->wid.w - bw - 8;
					ChRect chip;
					ChDrawRect(win->canv, bx, tb->wid.y + 6, bw, 22, 0xff1a73e8u);
					chip.x = bx;
					chip.y = tb->wid.y + 6;
					chip.w = bw;
					chip.h = 22;
					ChFontDrawTextClipped(win->canv, tb->font, "Sign in", bx + 12, tb->wid.y + 22,
										  0xffffffffu, &chip);
					k += llen;
					continue;
				}
				if (llen == 17 && !strncmp(line, "[ Search Google ]", 17)) {
					int bw = tb->wid.w - 120;
					int bx;
					if (bw < 40)
						bw = tb->wid.w > 8 ? tb->wid.w - 8 : tb->wid.w;
					bx = tb->wid.x + (tb->wid.w - bw) / 2;
					ChDrawRect(win->canv, bx, peny - 16, bw, 26, 0xffffffffu);
					ChDrawRectUnfilled(win->canv, bx, peny - 16, bw, 26,
									   s_qFocus ? 0xff4285f4u : 0xffdadce0u);
					s_qX = bx;
					s_qY = peny - 16;
					s_qW = bw;
					s_qH = 26;
					s_qHave = 1;
					if (s_query[0] && tb->font) {
						ChRect field;
						field.x = bx + 8;
						field.y = peny - 16;
						field.w = bw - 16;
						field.h = 26;
						ChFontDrawTextClipped(win->canv, tb->font, s_query, bx + 8, peny,
											  0xff202124u, &field);
					}
					if (s_qFocus && tb->font) {
						int cw = 0;
						size_t ci;
						for (ci = 0; ci < s_qCursor && s_query[ci]; ci++)
							cw += ChFontGetWidthChar(tb->font, s_query[ci]);
						if (cw > bw - 20)
							cw = bw - 20;
						ChDrawRect(win->canv, bx + 8 + cw, peny - 14, 2, 18, 0xff202124u);
					}
					k += llen;
					peny += 19;
					continue;
				}
				if (llen >= 5 && line[0] == '[' && line[1] == ' ') {
					/* One centered row of "[ label ]" chips, Google Search
					 * beside I'm Feeling Lucky instead of a stack. */
					int total = 0;
					int nchip = 0;
					size_t scan = 0;
					while (scan < llen) {
						while (scan < llen && line[scan] == ' ')
							scan++;
						if (scan >= llen || line[scan] != '[')
							break;
						size_t e = scan;
						int tw = 0;
						while (e < llen && line[e] != ']')
							e++;
						if (e >= llen)
							break;
						for (size_t t = scan + 2; t + 1 < e; t++)
							tw += ChFontGetWidthChar(tb->font, line[t]);
						total += tw + 24;
						nchip++;
						scan = e + 1;
					}
					if (nchip > 0) {
						int gap = 8;
						int row = total + gap * (nchip - 1);
						int bx = tb->wid.x + (tb->wid.w - row) / 2;
						if (bx < tb->wid.x + 4)
							bx = tb->wid.x + 4;
						scan = 0;
						while (scan < llen) {
							char label[80];
							int tw = 0;
							size_t t;
							size_t e;
							int bw;
							ChRect chip;
							while (scan < llen && line[scan] == ' ')
								scan++;
							if (scan >= llen || line[scan] != '[')
								break;
							e = scan;
							while (e < llen && line[e] != ']')
								e++;
							if (e >= llen)
								break;
							t = 0;
							for (size_t u = scan + 2; u + 1 < e && t + 1 < sizeof(label); u++) {
								label[t++] = line[u];
								tw += ChFontGetWidthChar(tb->font, line[u]);
							}
							label[t] = 0;
							bw = tw + 24;
							ChDrawRect(win->canv, bx, peny - 16, bw, 24, 0xfff8f9fau);
							ChDrawRectUnfilled(win->canv, bx, peny - 16, bw, 24, 0xffdadce0u);
							chip.x = bx;
							chip.y = peny - 16;
							chip.w = bw;
							chip.h = 24;
							ChFontDrawTextClipped(win->canv, tb->font, label, bx + 12, peny,
												  0xff3c4043u, &chip);
							if (!strcmp(label, "Google Search")) {
								s_btnX = bx;
								s_btnY = peny - 16;
								s_btnW = bw;
								s_btnH = 24;
								s_btnHave = 1;
							} else if (!strcmp(label, "I'm Feeling Lucky")) {
								s_luckyX = bx;
								s_luckyY = peny - 16;
								s_luckyW = bw;
								s_luckyH = 24;
								s_luckyHave = 1;
							}
							bx += bw + gap;
							scan = e + 1;
						}
						k += llen;
						peny += 19;
						continue;
					}
				}
			}
		}
		if (c == '\x01' && s_view[k + 1] >= '0' && s_view[k + 1] < '0' + NS_IMG_MAX) {
			int ix = s_view[k + 1] - '0';
			k++;
			if (ix < s_nimg && s_imgPx[ix] && s_imgW[ix] > 0 && s_imgH[ix] > 0) {
				int dw = tb->wid.w - 16;
				int dh = s_imgH[ix];
				int row, col;
				if (dw > s_imgW[ix])
					dw = s_imgW[ix];
				if (dw > 480)
					dw = 480;
				if (dh > 280)
					dh = 280;
				for (row = 0; row < dh; row++) {
					int sy = row * s_imgH[ix] / dh;
					uint32_t* dstp = win->canv->buffer +
									 (uint32_t)(peny + row) * win->canv->canvasWidth + (tb->wid.x + 8);
					if (peny + row >= tb->wid.y + tb->wid.h)
						break;
					for (col = 0; col < dw; col++) {
						int sx = col * s_imgW[ix] / dw;
						dstp[col] = s_imgPx[ix][sy * s_imgW[ix] + sx];
					}
				}
				peny += dh + 4;
			}
			penx = tb->wid.x + 4;
			continue;
		}
		if (c == '\n') {
			int step = 19;
			if (si <= NS_BODY_MAX) {
				int packed = (int)(s_col[si] & 0xff);
				if (packed >= 14 && packed <= 36)
					step = packed;
				if (s_bg[si])
					ChDrawRect(win->canv, tb->wid.x + 4, peny + 2, tb->wid.w - 8, 1, s_bg[si]);
			}
			penx = tb->wid.x + 4;
			peny += step;
			if (peny > tb->wid.y + tb->wid.h)
				break;
			continue;
		}
		/* White glyphs on the white page were the "HTTPS only" screen. */
		{
			uint32_t bgc = runBg ? runBg : (g_pageBg ? g_pageBg : 0xffffffffu);
			int cr = (int)((col >> 16) & 255), cg = (int)((col >> 8) & 255), cb = (int)(col & 255);
			int br = (int)((bgc >> 16) & 255), bgg = (int)((bgc >> 8) & 255), bb = (int)(bgc & 255);
			int cl = (cr * 3 + cg * 6 + cb) / 10;
			int bl = (br * 3 + bgg * 6 + bb) / 10;
			int d = cl - bl;
			if (d < 0)
				d = -d;
			if (d < 48)
				col = bl > 140 ? 0xff202124u : 0xffffffffu;
		}
		{
			ChFont* f = (szpx >= 17 && s_fontBig) ? s_fontBig : tb->font;
			int cw = ChFontGetWidthChar(f, c);
			if (runBg && cw > 0) {
				int by = peny - 14;
				if (by < tb->wid.y)
					by = tb->wid.y;
				ChDrawRect(win->canv, penx, by, cw, 18, runBg);
			}
			if (lineMax > 0 && !(fl & 0x80) && penx > lineX0 && penx + cw > lineX0 + lineMax) {
				penx = lineX0;
				peny += 19;
				if (peny > tb->wid.y + tb->wid.h)
					break;
			}
			if (fl & NS_CSS_ITALIC)
				ChFontDrawCharClipped(win->canv, f, c, penx + 1, peny, col, &clip);
			ChFontDrawCharClipped(win->canv, f, c, penx, peny, col, &clip);
			if (fl & NS_CSS_BOLD)
				ChFontDrawCharClipped(win->canv, f, c, penx + 1, peny, col, &clip);
			if (fl & NS_CSS_UNDER)
				ChDrawRect(win->canv, penx, peny + 2, cw > 0 ? cw : 1, 1, col);
			if (fl & NS_CSS_STRIKE)
				ChDrawRect(win->canv, penx, peny - 5, cw > 0 ? cw : 1, 1, col);
			if (fl & NS_CSS_OVER)
				ChDrawRect(win->canv, penx, peny - 12, cw > 0 ? cw : 1, 1, col);
			for (int li = 0; li < s_nlinks; li++) {
				if (si < s_links[li].a || si >= s_links[li].b)
					continue;
				ChDrawRect(win->canv, penx, peny + 2, cw > 0 ? cw : 1, 1, 0xff1a0dabu);
				if (!s_links[li].have) {
					s_links[li].x = penx;
					s_links[li].y = peny - 14;
					s_links[li].w = cw;
					s_links[li].h = 18;
					s_links[li].have = 1;
				} else if (penx + cw > s_links[li].x + s_links[li].w &&
						   peny - 14 < s_links[li].y + s_links[li].h) {
					s_links[li].w = penx + cw - s_links[li].x;
				}
			}
			penx += cw;
		}
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
			s_bg[f] = 0;
			s_sz[f] = 14;
			s_fl[f] = 0;
		}
		s_nlinks = 0;
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
	ChFontDrawTextClipped(
		win->canv, tb->font, tb->text, tb->wid.x + 4, tb->wid.y + 19, tb->textColor, &clip);
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

/* Readability-mode HTML to text (NOT a layout engine): block tags become
 * newlines, list items get bullets, links keep their URLs, script/style
 * bodies are dropped, entities (named + numeric) are decoded. */
static void NsEmitChar(char* dst, size_t dstsz, size_t* di, char c) {
	if (*di + 1 >= dstsz)
		return;
	if (c != '\n' && g_runWidth && !g_runNowrap && g_linePx >= (int)g_runWidth && g_linePx > 0) {
		g_linePx = 0;
		NsEmitChar(dst, dstsz, di, '\n');
	}
	if (g_runCase == 1 && c >= 'a' && c <= 'z')
		c = (char)(c - 32);
	else if (g_runCase == 2 && c >= 'A' && c <= 'Z')
		c = (char)(c + 32);
	dst[*di] = c;
	if (*di <= g_colMaxCur) {
		uint8_t sz = g_runSize;
		if (sz > 127)
			sz = 127;
		if (g_runCenter && c != '\n')
			sz = (uint8_t)(sz | 0x80);
		g_colCur[*di] = g_runColor;
		g_bgCur[*di] = g_runBg;
		g_szCur[*di] = sz;
		g_flCur[*di] = (uint8_t)(g_runDecor | (g_runAlign << 5) | (g_runNowrap ? 0x80 : 0));
		if (c == '\n') {
			g_szCur[*di] = g_runIndent;
			g_colCur[*di] = g_runLine | ((uint32_t)g_runWidth << 8);
			g_bgCur[*di] = g_runBorder;
			g_linePx = 0;
		} else {
			g_linePx += 8;
		}
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

static void NsImgReset(void) {
	int i;
	for (i = 0; i < s_nimg; i++) {
		free(s_imgPx[i]);
		s_imgPx[i] = NULL;
	}
	s_nimg = 0;
}

/* 1 = png, 2 = jpeg/jpg. The query string is ignored. SVG is neither. */
static int NsImgKind(const char* href) {
	const char* q = strchr(href, '?');
	const char* end = q ? q : href + strlen(href);
	const char* s = end;
	char tail[8];
	int n = 0;
	int i;
	while (s > href && n < 7) {
		s--;
		n++;
	}
	for (i = 0; i < n; i++) {
		char c = s[i];
		if (c >= 'A' && c <= 'Z')
			c = (char)(c + 32);
		tail[i] = c;
	}
	tail[n] = 0;
	if (n >= 4 && !strcmp(tail + n - 4, ".png"))
		return 1;
	if (n >= 4 && !strcmp(tail + n - 4, ".jpg"))
		return 2;
	if (n >= 5 && !strcmp(tail + n - 5, ".jpeg"))
		return 2;
	if (n >= 4 && !strcmp(tail + n - 4, ".svg"))
		return 3;
	return 0;
}

static void* NsImgAlloc(uint32_t n) {
	return malloc(n);
}

int NsJpegDecode(const unsigned char* data, size_t len, unsigned** out_px, int* out_w,
						int* out_h) {
	void* mem;
	Jpeg::Decoder* d;
	unsigned char* src;
	unsigned* px;
	int w, h, i, n;
	*out_px = NULL;
	if (!data || len < 2 || data[0] != 0xFF || data[1] != 0xD8)
		return -1;
	mem = malloc(sizeof(Jpeg::Decoder));
	if (!mem)
		return -1;
	d = new (mem) Jpeg::Decoder(data, (uint32_t)len, NsImgAlloc, free);
	if (d->GetResult() != Jpeg::Decoder::OK || d->GetWidth() <= 0 || d->GetHeight() <= 0 ||
		d->GetWidth() > 1920 || d->GetHeight() > 1920) {
		d->~Decoder();
		free(mem);
		return -1;
	}
	w = d->GetWidth();
	h = d->GetHeight();
	n = w * h;
	px = (unsigned*)malloc((size_t)n * 4);
	src = d->GetImage();
	if (!px || !src) {
		free(px);
		d->~Decoder();
		free(mem);
		return -1;
	}
	if (d->IsColor()) {
		for (i = 0; i < n; i++)
			px[i] = 0xff000000u | ((unsigned)src[i * 3] << 16) | ((unsigned)src[i * 3 + 1] << 8) |
					src[i * 3 + 2];
	} else {
		for (i = 0; i < n; i++)
			px[i] = 0xff000000u | ((unsigned)src[i] << 16) | ((unsigned)src[i] << 8) | src[i];
	}
	d->~Decoder();
	free(mem);
	*out_px = px;
	*out_w = w;
	*out_h = h;
	return 0;
}

static int NsImgQueue(const char* href) {
	char abs[NS_URL_MAX * 2];
	int ix;
	if (!href || !href[0] || s_nimg >= NS_IMG_MAX)
		return -1;
	if (!NsImgKind(href))
		return -1;
	NsResolveUrl(s_url, href, abs, sizeof(abs));
	if (strncmp(abs, "http://", 7) && strncmp(abs, "https://", 8))
		return -1;
	ix = s_nimg++;
	snprintf(s_imgUrl[ix], sizeof(s_imgUrl[ix]), "%s", abs);
	s_imgPx[ix] = NULL;
	s_imgW[ix] = s_imgH[ix] = 0;
	return ix;
}

static void NsImgFill(int ix) {
	xe_http_response resp;
	unsigned* px = NULL;
	int w = 0, h = 0, kind, ok;
	size_t cap;
	kind = NsImgKind(s_imgUrl[ix]);
	if (!kind)
		return;
	memset(&resp, 0, sizeof(resp));
	cap = (kind == 3) ? (2u * 1024u * 1024u) : (512u * 1024u);
	if (xe_http_get(s_imgUrl[ix], "GET", kNsUa, &resp, cap) != 0 || !resp.body)
		goto done;
	if (kind == 1)
		ok = ns_png_decode((const unsigned char*)resp.body, resp.body_len, &px, &w, &h);
	else if (kind == 2)
		ok = NsJpegDecode((const unsigned char*)resp.body, resp.body_len, &px, &w, &h);
	else
		ok = ns_svg_decode(resp.body, resp.body_len, &px, &w, &h);
	if (ok != 0 || !px || w <= 0 || h <= 0)
		goto done;
	s_imgPx[ix] = (uint32_t*)px;
	s_imgW[ix] = w;
	s_imgH[ix] = h;
	px = NULL;
done:
	free(px);
	xe_http_response_free(&resp);
}

static void NsImgFetchAll(void) {
	int i;
	/* Worker threads fault: the subthread returns to ELR 0, and dlmalloc
	 * is not safe on more than one thread. Fetch on this thread. */
	if (s_nimg <= 0)
		return;
	for (i = 0; i < s_nimg; i++)
		NsImgFill(i);
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
	} kEnts[] = {{"amp", "&"},
				 {"lt", "<"},
				 {"gt", ">"},
				 {"quot", "\""},
				 {"nbsp", " "},
				 {"copy", "(c)"},
				 {"reg", "(R)"},
				 {"mdash", "-"},
				 {"ndash", "-"},
				 {"hellip", "..."},
				 {"trade", "(TM)"},
				 {NULL, NULL}};
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
		else if (s_latinLink && !s_latinUsed) {
			NsEmitStr(dst, dstsz, di, s_latinLink);
			s_latinUsed = 1;
		} else if (!s_latinLink)
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
static bool
NsTagAttr(const char* src, size_t j, size_t k, const char* name, char* out, size_t outsz) {
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
		g_runColor = 0xff202124u;
		g_runBg = 0;
		g_runSize = 14;
		g_runCenter = 0;
		g_runDecor = 0;
		g_runAlign = 0;
		g_runIndent = 0;
		g_runLine = 19;
		g_runCase = 0;
		g_runPre = 0;
		g_runNowrap = 0;
		g_runWidth = 0;
		g_runBorder = 0;
		return;
	}
	g_runColor = NsCssNodeColor(top);
	g_runBg = 0;
	for (int bi = depth - 1; bi >= 0; bi--) {
		uint32_t b = NsCssNodeBackground(stack[bi]);
		if (!b || b == g_pageBg)
			continue;
		g_runBg = b;
		break;
	}
	g_runSize = (uint8_t)NsCssNodeFontPx(top);
	{
		NsCssUsed u;
		NsCssNodeUsed(top, &u);
		g_runDecor = u.decor;
		g_runAlign = u.align;
		g_runIndent = u.indent;
		g_runLine = u.line ? u.line : 19;
		g_runCase = u.upper;
		g_runPre = u.pre;
		g_runNowrap = u.nowrap;
		g_runWidth = u.width;
		g_runBorder = u.border;
		if (u.opacity < 255) {
			uint32_t onto = g_runBg ? g_runBg : g_pageBg;
			g_runColor = NsMixOpacity(g_runColor, onto, u.opacity);
		}
	}
	g_runCenter = 0;
	for (int ci = 0; ci < depth; ci++) {
		if (!strcmp(stack[ci]->tag, "center") || NsCssNodeCenter(stack[ci]))
			g_runCenter = 1;
	}
	if (!top->parent) {
		uint32_t page = NsCssNodeBackground(top);
		if (page)
			g_pageBg = page;
	}
	if (!strcmp(top->tag, "html") || !strcmp(top->tag, "body")) {
		uint32_t page = NsCssNodeBackground(top);
		if (page)
			g_pageBg = page;
		if (NsCssNodeCenter(top))
			g_pageCenter = 1;
	}
}

/* libcss drops var(). Google's sheet is almost entirely var(--token),
 * so text and background never resolved. Substitute a single-token
 * custom property, or the fallback, when the replacement fits. */
/* Last `--name:` whose value is a single token with no function call. */
static int NsCssVarValue(const char* name, size_t nl, const char** val, size_t* vl,
						  const char* gap_lo, const char* gap_hi) {
	const char* p = s_cssBuf;
	int found = 0;
	*val = NULL;
	*vl = 0;
	while ((p = strstr(p, "--")) != NULL) {
		if (p >= gap_lo && p < gap_hi) {
			p = gap_hi;
			continue;
		}
		const char* v;
		const char* e;
		size_t n;
		char prev;
		prev = (p == s_cssBuf) ? ';' : p[-1];
		if (prev != '{' && prev != ';' && prev != ' ' && prev != '\t' &&
			prev != '\n' && prev != '\r') {
			p += 2;
			continue;
		}
		if (strncmp(p + 2, name, nl) != 0 || p[2 + nl] != ':') {
			p += 2;
			continue;
		}
		v = p + 2 + nl + 1;
		while (*v == ' ' || *v == '\t')
			v++;
		e = v;
		while (*e && *e != ';' && *e != '}' && *e != '{')
			e++;
		n = (size_t)(e - v);
		while (n && (v[n - 1] == ' ' || v[n - 1] == '\t'))
			n--;
		if (n && n < 48 && !memchr(v, '(', n)) {
			*val = v;
			*vl = n;
			found = 1;
		}
		p += 2;
	}
	return found;
}

static void NsCssResolveVars(void) {
	char* r = s_cssBuf;
	char* w = s_cssBuf;
	while (*r) {
		if (!strncmp(r, "var(--", 6)) {
			char* n = r + 6;
			char* c = n;
			char* end = NULL;
			const char* rep = NULL;
			size_t rl = 0;
			size_t nl;
			while (*c && (isalpha((unsigned char)*c) || isdigit((unsigned char)*c) ||
						  *c == '_' || *c == '-'))
				c++;
			nl = (size_t)(c - n);
			if (nl && (*c == ')' || *c == ',')) {
				if (*c == ')')
					end = c + 1;
				else {
					end = strchr(c, ')');
					if (end)
						end++;
				}
				NsCssVarValue(n, nl, &rep, &rl, w, r);
				if (end && !rep && *c == ',') {
					char* fb = c + 1;
					size_t fl;
					while (*fb == ' ' || *fb == '\t')
						fb++;
					fl = (size_t)((end - 1) - fb);
					if (fl && fl <= (size_t)(end - r)) {
						memmove(w, fb, fl);
						w += fl;
						r = end;
						continue;
					}
				}
				if (end && rep && rl <= (size_t)(end - r)) {
					memmove(w, rep, rl);
					w += rl;
					r = end;
					continue;
				}
			}
		}
		*w++ = *r++;
	}
	*w = 0;
}

/* libcss has no light-dark(). Keep the first color. */
static void NsCssFixLightDark(void) {
	char* r = s_cssBuf;
	char* w = s_cssBuf;
	while (*r) {
		if (!strncmp(r, "light-dark(", 11)) {
			char* comma = strchr(r + 11, ',');
			char* endp = comma ? strchr(comma, ')') : NULL;
			if (comma && endp) {
				size_t n = (size_t)(comma - (r + 11));
				memmove(w, r + 11, n);
				w += n;
				r = endp + 1;
				continue;
			}
		}
		*w++ = *r++;
	}
	*w = 0;
	NsCssResolveVars();
	NsCssResolveVars();
}

/* Collect <style> bodies before the render pass. */
static void NsCollectCss(const char* src, size_t srclen) {
	size_t di = 0;
	size_t i = 0;
	int in_noscript = 0;
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
		if (src[i + 1] == '/') {
			char ctag[12];
			int cl = 0;
			size_t cj = i + 2;
			while (cj < srclen && cl < 11 &&
				   (isalpha((unsigned char)src[cj]) || isdigit((unsigned char)src[cj]))) {
				char tc = src[cj];
				if (tc >= 'A' && tc <= 'Z')
					tc += 32;
				ctag[cl++] = tc;
				cj++;
			}
			ctag[cl] = 0;
			if (!strcmp(ctag, "noscript") && in_noscript > 0)
				in_noscript--;
			i = cj;
			continue;
		}
		if (!strcmp(tag, "noscript")) {
			in_noscript++;
			i = j;
			continue;
		}
		/* The search interstitial hides div/p from inside <noscript>. */
		if (strcmp(tag, "style") != 0 || in_noscript) {
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
	NsCssFixLightDark();
}

/* Case-insensitive substring test (rel="stylesheet"). */
static bool NsContainsCI(const char* h, const char* needle) {
	size_t n = strlen(needle);
	if (!n)
		return true;
	if (!h)
		return false;
	for (; *h; h++) {
		size_t i;
		for (i = 0; i < n; i++) {
			char a = h[i];
			char b = needle[i];
			if (a >= 'A' && a <= 'Z')
				a += 32;
			if (b >= 'A' && b <= 'Z')
				b += 32;
			if (a != b)
				break;
		}
		if (i == n)
			return true;
	}
	return false;
}

/* Resolve a stylesheet href against the page URL. */
static void NsResolveUrl(const char* base, const char* href, char* out, size_t outsz) {
	char clean[NS_URL_MAX];
	char* q;
	char* f;
	const char* scheme;
	const char* host;
	const char* slash;
	size_t origin_len;
	if (!out || outsz == 0)
		return;
	out[0] = 0;
	if (!href || !href[0])
		return;
	if (!strncmp(href, "http://", 7) || !strncmp(href, "https://", 8)) {
		snprintf(out, outsz, "%s", href);
		return;
	}
	if (!strncmp(href, "//", 2)) {
		scheme = strstr(base, "://");
		if (scheme)
			snprintf(out, outsz, "%.*s:%s", (int)(scheme - base), base, href);
		else
			snprintf(out, outsz, "https:%s", href);
		return;
	}
	snprintf(clean, sizeof(clean), "%s", base);
	q = strchr(clean, '?');
	if (q)
		*q = 0;
	f = strchr(clean, '#');
	if (f)
		*f = 0;
	scheme = strstr(clean, "://");
	host = scheme ? scheme + 3 : clean;
	slash = strchr(host, '/');
	origin_len = slash ? (size_t)(slash - clean) : strlen(clean);
	if (href[0] == '/') {
		snprintf(out, outsz, "%.*s%s", (int)origin_len, clean, href);
		return;
	}
	if (slash) {
		const char* last = strrchr(slash, '/');
		size_t dir_len = last ? (size_t)(last - slash) + 1 : 1;
		snprintf(out, outsz, "%.*s%.*s%s", (int)origin_len, clean,
				 (int)dir_len, slash, href);
	} else {
		snprintf(out, outsz, "%.*s/%s", (int)origin_len, clean, href);
	}
}

/* Fetch <link rel=stylesheet> targets and append them to the inline CSS.
 * libcss gets one author sheet, so concatenation is deliberate. */
static void NsCollectCssLinks(const char* src, size_t srclen, const char* base) {
	int fetched = 0;
	size_t i = 0;
	while (i + 7 < srclen && fetched < 2) {
		char href[512];
		char rel[64];
		char media[64];
		char abs[NS_URL_MAX * 2];
		size_t j;
		size_t k;
		bool quote;
		char quote_c;
		if (src[i] != '<' ||
			tolower((unsigned char)src[i + 1]) != 'l' ||
			tolower((unsigned char)src[i + 2]) != 'i' ||
			tolower((unsigned char)src[i + 3]) != 'n' ||
			tolower((unsigned char)src[i + 4]) != 'k') {
			i++;
			continue;
		}
		j = i + 5;
		if (j < srclen && src[j] != ' ' && src[j] != '\t' &&
			src[j] != '\r' && src[j] != '\n' && src[j] != '/' && src[j] != '>') {
			i++;
			continue;
		}
		k = j;
		quote = false;
		quote_c = 0;
		while (k < srclen && (src[k] != '>' || quote)) {
			if (!quote && (src[k] == '"' || src[k] == '\'')) {
				quote = true;
				quote_c = src[k];
			} else if (quote && src[k] == quote_c) {
				quote = false;
			}
			k++;
		}
		if (k >= srclen)
			break;
		href[0] = rel[0] = media[0] = 0;
		if (!NsTagAttr(src, j, k, "href", href, sizeof(href)) ||
			!NsTagAttr(src, j, k, "rel", rel, sizeof(rel)) ||
			!NsContainsCI(rel, "stylesheet") ||
			!href[0] || !strncmp(href, "data:", 5) ||
			!strncmp(href, "javascript:", 11) ||
			(strlen(s_cssBuf) + 2048 >= sizeof(s_cssBuf))) {
			i = k + 1;
			continue;
		}
		if (NsTagAttr(src, j, k, "media", media, sizeof(media)) &&
			NsContainsCI(media, "print") && !NsContainsCI(media, "screen") &&
			!NsContainsCI(media, "all")) {
			i = k + 1;
			continue;
		}
		NsResolveUrl(base, href, abs, sizeof(abs));
		if (!strncmp(abs, "http://", 7) || !strncmp(abs, "https://", 8)) {
			xe_http_response cresp;
			int crc;
			memset(&cresp, 0, sizeof(cresp));
			crc = xe_http_get(abs, "GET", kNsUa, &cresp, sizeof(s_cssBuf) - 2);
			if (crc == 0 && cresp.body && cresp.body_len) {
				size_t at = strlen(s_cssBuf);
				const char* css = cresp.body;
				size_t n = cresp.body_len;
				size_t room = sizeof(s_cssBuf) - at - 2;
				/* A cut sheet fails data_done and the author CSS is dropped,
				 * which is why the page came out as plain UA text. */
				if (n >= 3 && (unsigned char)css[0] == 0xEF &&
					(unsigned char)css[1] == 0xBB && (unsigned char)css[2] == 0xBF) {
					css += 3;
					n -= 3;
				}
				if (n > room)
					n = room;
				if (n) {
					memcpy(s_cssBuf + at, (void*)css, n);
					at += n;
					s_cssBuf[at++] = '\n';
					s_cssBuf[at] = 0;
					fetched++;
				}
			}
			_KePrint("[css] link %s -> %d bytes rc=%d\n", abs,
					 (int)cresp.body_len, crc);
			xe_http_response_free(&cresp);
		}
		i = k + 1;
	}
	if (fetched)
		NsCssFixLightDark();
}

static void NsHtmlToText(const char* src, size_t srclen, char* dst, size_t dstsz) {
	size_t di = 0;
	size_t i = 0;
	int skip = 0; /* 1 = inside <script>, 2 = inside <style>, 3 = <noscript> */
	g_pageCenter = 0;
	g_pageBg = 0xffffffffu;
	if (!g_inWrite) {
		s_nlinks = 0;
		NsImgReset();
	}
	int link_slot = -1;
	size_t link_at = 0;
	char href[512];
	href[0] = 0;
	NsCssNode* stack[NS_CSS_STACK];
	uint8_t skipped[NS_CSS_STACK];
	size_t opened[NS_CSS_STACK];
	int depth = 0;
	int hide = 0;

	/* Mid-parse JS output (document.write/console) must land exactly at
	 * the parse position: publish the emission target for the sink.
	 * Fragment parses keep the outer target untouched. */
	if (!g_inWrite) {
		s_outLive = true;
		s_outDst = dst;
		s_outSz = dstsz;
		s_outDi = &di;
		g_colCur = s_col;
		g_bgCur = s_bg;
		g_szCur = s_sz;
		g_flCur = s_fl;
		g_colMaxCur = NS_BODY_MAX;
		g_linePx = 0;
		s_liNo = 0;
	}

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
					if (skip == 1 && !g_inWrite)
						NsJsScriptChunk(src[i]);
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
								(skip == 2 && !strcmp(tag, "style")))) {
					/* End of an inline <script>: hand the accumulated
					 * body to the engine. Fragments (document.write
					 * output) never run their own <script> tags. */
					if (skip == 1 && !g_inWrite)
						NsJsRunPending();
					skip = 0;
				}
				i = (k < srclen) ? k + 1 : srclen;
				continue;
			}
			if (!closing && (!strcmp(tag, "script") || !strcmp(tag, "style"))) {
				if (!strcmp(tag, "script")) {
					/* External src (no loader): noted when the script
					 * element closes with an empty body. Fragment
					 * scripts are skipped wholesale (never run). */
					if (!g_inWrite) {
						char srcAttr[128];
						char typeAttr[64];
						if (NsTagAttr(src, j, k, "src", srcAttr, sizeof(srcAttr)))
							NsJsScriptSrcAttr(srcAttr);
						else
							NsJsScriptSrcAttr(NULL);
						if (NsTagAttr(src, j, k, "type", typeAttr, sizeof(typeAttr)))
							NsJsScriptType(typeAttr);
						else
							NsJsScriptType(NULL);
					}
				}
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
						if ((h + 4 < k) && (src[h] == 'h' || src[h] == 'H') &&
							(src[h + 1] == 'r' || src[h + 1] == 'R') &&
							(src[h + 2] == 'e' || src[h + 2] == 'E') &&
							(src[h + 3] == 'f' || src[h + 3] == 'F')) {
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
								{
									/* href is not passed through NsEntity. */
									char* a = href;
									char* b = href;
									while (*a) {
										if (!strncmp(a, "&amp;", 5)) {
											*b++ = '&';
											a += 5;
										} else if (!strncmp(a, "&quot;", 6)) {
											*b++ = '"';
											a += 6;
										} else if (!strncmp(a, "&#39;", 5) ||
												   !strncmp(a, "&apos;", 6)) {
											*b++ = '\'';
											a += (*a == '&' && a[1] == '#') ? 5 : 6;
										} else {
											*b++ = *a++;
										}
									}
									*b = 0;
								}
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
						if (!strcmp(tag, "a") && href[0]) {
							NsCssNodeSetLink(el, true);
							if (!g_inWrite && s_nlinks < NS_LINK_MAX) {
								link_slot = s_nlinks;
								link_at = di;
								snprintf(s_links[link_slot].href, sizeof(s_links[link_slot].href), "%s", href);
							}
							s_latinLink = NULL;
							s_latinUsed = 0;
							{
								const char* hl = strstr(href, "hl=");
								static const struct {
									const char* id;
									const char* name;
								} kHl[] = {{"hi", "Hindi"},		{"bn", "Bengali"},
										   {"te", "Telugu"},		{"mr", "Marathi"},
										   {"ta", "Tamil"},		{"gu", "Gujarati"},
										   {"kn", "Kannada"},		{"ml", "Malayalam"},
										   {"pa", "Punjabi"},		{"ur", "Urdu"},
										   {"ar", "Arabic"},		{"ru", "Russian"},
										   {"ja", "Japanese"},	{"ko", "Korean"},
										   {"zh-CN", "Chinese"},	{"zh-TW", "Chinese"},
										   {"de", "German"},		{"fr", "French"},
										   {"es", "Spanish"},		{"pt-BR", "Portuguese"},
										   {"pt", "Portuguese"},	{"it", "Italian"},
										   {"nl", "Dutch"},		{"pl", "Polish"},
										   {"en", "English"},		{NULL, NULL}};
								if (hl) {
									hl += 3;
									for (int hi = 0; kHl[hi].id; hi++) {
										size_t n = strlen(kHl[hi].id);
										if (!strncmp(hl, kHl[hi].id, n) &&
											(hl[n] == '&' || hl[n] == 0 || hl[n] == '#')) {
											s_latinLink = kHl[hi].name;
											break;
										}
									}
								}
							}
						}
						if (NsTagAttr(src, j, k, "style", attr, sizeof(attr)))
							NsCssNodeSetInline(el, attr);
						NsCssSelect(el);
						skipped[depth] = NsCssNodeSkip(el) ? 1 : 0;
						/* link/meta/img never have an end tag. Counting them
						 * as an open display:none region hid the rest of
						 * example.com and the Google form. */
						{
							int void_el = !strcmp(tag, "link") || !strcmp(tag, "meta") ||
										  !strcmp(tag, "img") || !strcmp(tag, "br") ||
										  !strcmp(tag, "hr") || !strcmp(tag, "input") ||
										  !strcmp(tag, "source") || !strcmp(tag, "base") ||
										  !strcmp(tag, "area") || !strcmp(tag, "col") ||
										  !strcmp(tag, "embed") || !strcmp(tag, "wbr");
							if (skipped[depth] && !void_el)
								hide++;
							if (void_el) {
								NsCssNodeDestroy(el);
								el = NULL;
							} else {
								opened[depth] = di;
								stack[depth++] = el;
								NsRunFromTop(stack, depth);
							}
						}
					}
				}
				if (!strcmp(tag, "img") && !g_inWrite) {
					char isrc[512];
					char alt[128];
					int ix;
					isrc[0] = alt[0] = 0;
					NsTagAttr(src, j, k, "src", isrc, sizeof(isrc));
					NsTagAttr(src, j, k, "alt", alt, sizeof(alt));
					ix = NsImgQueue(isrc);
					if (ix >= 0) {
						char mark[3];
						NsEmitNewlines(dst, dstsz, &di, 1);
						mark[0] = '\x01';
						mark[1] = (char)('0' + ix);
						mark[2] = 0;
						NsEmitStr(dst, dstsz, &di, mark);
						NsEmitNewlines(dst, dstsz, &di, 1);
					}
					if (alt[0]) {
						NsEmitStr(dst, dstsz, &di, alt);
						NsEmitChar(dst, dstsz, &di, ' ');
					}
				}
				if (!strcmp(tag, "video") || !strcmp(tag, "source")) {
					char vsrc[256];
					vsrc[0] = 0;
					NsTagAttr(src, j, k, "src", vsrc, sizeof(vsrc));
					NsEmitStr(dst, dstsz, &di, "[video");
					if (vsrc[0]) {
						NsEmitStr(dst, dstsz, &di, " ");
						NsEmitStr(dst, dstsz, &di, vsrc);
					}
					NsEmitStr(dst, dstsz, &di, "]\n");
				}
				if (hide > 0) {
					/* display:none / visibility:hidden: keep the stack,
					 * drop the text. */
					i = (k < srclen) ? k + 1 : srclen;
					continue;
				}
				if (!strcmp(tag, "input") || !strcmp(tag, "textarea")) {
					char typ[16];
					char val[80];
					char name[64];
					typ[0] = val[0] = name[0] = 0;
					NsTagAttr(src, j, k, "type", typ, sizeof(typ));
					NsTagAttr(src, j, k, "value", val, sizeof(val));
					NsTagAttr(src, j, k, "name", name, sizeof(name));
					if (!strcmp(typ, "hidden")) {
						/* not a control the reader can use */
					} else if (val[0] && (!strcmp(typ, "submit") || !strcmp(typ, "button"))) {
						if (di == 0 || dst[di - 1] == '\n')
							NsEmitNewlines(dst, dstsz, &di, 1);
						else
							NsEmitChar(dst, dstsz, &di, ' ');
						NsEmitStr(dst, dstsz, &di, "[ ");
						NsEmitStr(dst, dstsz, &di, val);
						NsEmitStr(dst, dstsz, &di, " ]");
					} else if (!strcmp(name, "q") || !strcmp(typ, "search") ||
							   !strcmp(tag, "textarea")) {
						NsEmitNewlines(dst, dstsz, &di, 1);
						NsEmitStr(dst, dstsz, &di, "[ Search Google ]");
						NsEmitNewlines(dst, dstsz, &di, 1);
					}
				} else if (!strcmp(tag, "br")) {
					NsEmitNewlines(dst, dstsz, &di, 1);
				} else if (depth > 0 && NsCssNodeBlock(stack[depth - 1])) {
					NsEmitNewlines(dst, dstsz, &di, 1);
					if (!strcmp(tag, "ol") || !strcmp(tag, "ul"))
						s_liNo = 0;
					if (!strcmp(tag, "li")) {
						NsCssUsed u;
						char mark[12];
						NsCssNodeUsed(stack[depth - 1], &u);
						mark[0] = 0;
						if (u.list == 4) {
							s_liNo++;
							snprintf(mark, sizeof(mark), "%d. ", s_liNo);
						} else if (u.list == 5) {
							s_liNo++;
							mark[0] = (char)('a' + ((s_liNo - 1) % 26));
							mark[1] = '.';
							mark[2] = ' ';
							mark[3] = 0;
						} else if (u.list == 2) {
							snprintf(mark, sizeof(mark), "o ");
						} else if (u.list == 3) {
							snprintf(mark, sizeof(mark), "# ");
						} else if (u.list != 0) {
							snprintf(mark, sizeof(mark), "- ");
						}
						if (mark[0])
							NsEmitStr(dst, dstsz, &di, mark);
					}
				} else if (depth > 0 && di > 0 && dst[di - 1] != ' ' && dst[di - 1] != '\n') {
					/* Inline / inline-block / table-cell: keep the line,
					 * but don't glue "Gmail" to "Images". */
					NsEmitChar(dst, dstsz, &di, ' ');
				} else if (!strcmp(tag, "hr")) {
					NsEmitNewlines(dst, dstsz, &di, 1);
					NsEmitStr(dst, dstsz, &di, "---");
					NsEmitNewlines(dst, dstsz, &di, 1);
				}
			} else {
				/* Pop the element; author vertical margins add spacing. */
				int mlines = 0;
				int was_skip = 0;
				int was_block = 0;
				size_t opened_at = 0;
				if (depth > 1) {
					was_skip = skipped[depth - 1];
					was_block = NsCssNodeBlock(stack[depth - 1]);
					opened_at = opened[depth - 1];
					if (was_skip && hide > 0)
						hide--;
					mlines = NsCssNodeMarginLines(stack[depth - 1]);
					NsCssNodeDestroy(stack[depth - 1]);
					depth--;
					NsRunFromTop(stack, depth);
				}
				if (!strcmp(tag, "a")) {
					if (!g_inWrite && link_slot >= 0 && di > link_at) {
						s_links[link_slot].a = link_at;
						s_links[link_slot].b = di;
						s_links[link_slot].have = 0;
						s_nlinks++;
					}
					link_slot = -1;
					href[0] = 0;
					s_latinLink = NULL;
					s_latinUsed = 0;
				}
				if (was_skip || hide > 0) {
					i = (k < srclen) ? k + 1 : srclen;
					continue;
				}
				/* An empty wrapper (a div around a div) must not add a blank line. */
				if (was_block && di > opened_at)
					NsEmitNewlines(dst, dstsz, &di, 1);
				for (int t = 0; t < mlines; t++)
					NsEmitChar(dst, dstsz, &di, '\n');
			}
			i = (k < srclen) ? k + 1 : srclen;
			continue;
		}
		if (skip) {
			/* Inline script bodies are fed verbatim to the JS engine,
             * entity/raw char included: JS is not HTML text. */
			if (skip == 1 && !g_inWrite)
				NsJsScriptChunk(c);
			i++;
			continue;
		}
		if (hide > 0) {
			i++;
			continue;
		}
		if ((unsigned char)c >= 0x80) {
			/* UTF-8 text the Latin font cannot draw. A language link
			 * (hl=hi and the rest) is shown by its English name. */
			if (s_latinLink && !s_latinUsed) {
				NsEmitStr(dst, dstsz, &di, s_latinLink);
				s_latinUsed = 1;
			}
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
		if (c == '\r') {
			i++;
			continue;
		}
		if (c == '\t') {
			if (g_runPre)
				NsEmitChar(dst, dstsz, &di, ' ');
			i++;
			continue;
		}
		if (c == '\n') {
			if (g_runPre)
				NsEmitChar(dst, dstsz, &di, '\n');
			else if (di > 0 && dst[di - 1] != ' ' && dst[di - 1] != '\n')
				NsEmitChar(dst, dstsz, &di, ' ');
			i++;
			continue;
		}
		NsEmitChar(dst, dstsz, &di, c);
		i++;
	}
	while (depth > 0)
		NsCssNodeDestroy(stack[--depth]);
	dst[di] = 0;
	if (!g_inWrite) {
		s_outLive = false;
		s_outDi = NULL;
		NsImgFetchAll();
	}
}

/* Emit one raw output chunk (already text) into the current target:
 * mid-parse -> the live parse position; post-parse -> page text end. */
static void NsSinkRaw(const char* s, size_t n, int kind) {
	if (!s || !n || (!s_outLive && !s_postPage))
		return;
	/* Console kinds restyle; save/restore so following text keeps its run. */
	uint32_t sc = g_runColor;
	uint8_t ss = g_runSize;
	if (kind == NS_JS_KIND_LOG)
		g_runColor = NS_JS_COL_LOG;
	else if (kind == NS_JS_KIND_WARN)
		g_runColor = NS_JS_COL_WARN;
	else if (kind == NS_JS_KIND_ERR)
		g_runColor = NS_JS_COL_ERR;
	if (kind != NS_JS_KIND_WRITE)
		g_runSize = 14;

	if (s_outLive) {
		/* Mid-parse: the HTML pass publishes its output target and the
		 * live di; style indirections already point at the page arrays. */
		char* dst = s_outDst;
		size_t dstsz = s_outSz;
		size_t* diP = s_outDi;
		while (n--) {
			char c = *s++;
			if (c == '\n')
				NsEmitNewlines(dst, dstsz, diP, 1);
			else
				NsEmitChar(dst, dstsz, diP, c);
		}
		dst[*diP] = 0;
	} else {
		/* Post-parse (timers): append at the page text end, uniform style
		 * for the whole chunk. */
		char* dst = s_text;
		size_t dstsz = sizeof(s_text);
		size_t di = strlen(dst);
		uint32_t* savedCol = g_colCur;
		uint32_t* savedBg = g_bgCur;
		uint8_t* savedSz = g_szCur;
		uint8_t* savedFl = g_flCur;
		size_t savedMax = g_colMaxCur;
		g_colCur = s_col;
		g_bgCur = s_bg;
		g_szCur = s_sz;
		g_flCur = s_fl;
		g_colMaxCur = NS_BODY_MAX;
		while (n--) {
			char c = *s++;
			if (c == '\n')
				NsEmitNewlines(dst, dstsz, &di, 1);
			else
				NsEmitChar(dst, dstsz, &di, c);
		}
		dst[di] = 0;
		g_colCur = savedCol;
		g_bgCur = savedBg;
		g_szCur = savedSz;
		g_flCur = savedFl;
		g_colMaxCur = savedMax;
	}
	g_runColor = sc;
	g_runSize = ss;
}

/* document.write output carries HTML tags: browsers parse it inline. The
 * reader replays the fragment through the same HTML pass (own parse
 * position and style scratch), then emits the converted text. Written
 * <script> tags are skipped, not run. */
static void NsJsSink(const char* s, size_t n, int kind) {
	if (!s || !n)
		return;
	if (kind != NS_JS_KIND_WRITE) {
		NsSinkRaw(s, n, kind);
		return;
	}
	if (g_writeDepth >= 3) { /* runaway fragment chains: degrade to text */
		NsSinkRaw(s, n, kind);
		return;
	}
	int fidx = g_writeDepth;
	size_t cn = n < NS_FRAG_MAX ? n : NS_FRAG_MAX;
	memcpy(s_fragSrc[fidx], (void*)s, cn);
	s_fragSrc[fidx][cn] = 0;
	/* The fragment parse must not disturb the outer parse's sink target
	 * or run style: guarded, saved, restored. */
	bool savedLive = s_outLive;
	char* savedDst = s_outDst;
	size_t savedSz = s_outSz;
	size_t* savedDi = s_outDi;
	uint32_t* savedCol = g_colCur;
	uint32_t* savedBg = g_bgCur;
	uint8_t* savedSzArr = g_szCur;
	uint8_t* savedFl = g_flCur;
	size_t savedMax = g_colMaxCur;
	uint32_t savedRunC = g_runColor;
	uint32_t savedRunB = g_runBg;
	uint8_t savedRunS = g_runSize;
	uint8_t savedDecor = g_runDecor;
	uint8_t savedAlign = g_runAlign;
	uint8_t savedIndent = g_runIndent;
	uint8_t savedLine = g_runLine;
	uint8_t savedCase = g_runCase;
	uint8_t savedPre = g_runPre;
	uint8_t savedNowrap = g_runNowrap;
	uint16_t savedWidth = g_runWidth;
	uint32_t savedBorder = g_runBorder;
	bool savedInWrite = g_inWrite;
	g_colCur = s_fragJunkCol;
	g_bgCur = s_fragJunkBg;
	g_szCur = s_fragJunkSz;
	g_flCur = s_fragJunkFl;
	g_colMaxCur = NS_FRAG_MAX;
	g_inWrite = true;
	g_writeDepth++;
	NsHtmlToText(s_fragSrc[fidx], cn, s_fragOut[fidx], NS_FRAG_MAX);
	g_writeDepth--;
	g_inWrite = savedInWrite;
	s_outLive = savedLive;
	s_outDst = savedDst;
	s_outSz = savedSz;
	s_outDi = savedDi;
	g_colCur = savedCol;
	g_bgCur = savedBg;
	g_szCur = savedSzArr;
	g_flCur = savedFl;
	g_colMaxCur = savedMax;
	g_runColor = savedRunC;
	g_runBg = savedRunB;
	g_runSize = savedRunS;
	g_runDecor = savedDecor;
	g_runAlign = savedAlign;
	g_runIndent = savedIndent;
	g_runLine = savedLine;
	g_runCase = savedCase;
	g_runPre = savedPre;
	g_runNowrap = savedNowrap;
	g_runWidth = savedWidth;
	g_runBorder = savedBorder;
	NsSinkRaw(s_fragOut[fidx], strlen(s_fragOut[fidx]), NS_JS_KIND_WRITE);
}

/* A host with no scheme is fetched as https://. http:// stays plain HTTP. */
static void NsEnsureHttps(char* url, size_t n) {
	char tmp[NS_URL_MAX];
	if (!url || !url[0] || n < 10)
		return;
	if (!strncmp(url, "https://", 8) || !strncmp(url, "http://", 7))
		return;
	snprintf(tmp, sizeof(tmp), "https://%s", url);
	snprintf(url, n, "%s", tmp);
}

static int NsFetch(char* url) {
	char status[NS_URL_MAX + 64];
	xe_http_response resp;
	const char* body;
	size_t blen;
	int left_tls = 0;
	int used_tls = 0;

	s_query[0] = 0;
	s_qCursor = 0;
	s_qFocus = 0;
	NsEnsureHttps(url, NS_URL_MAX);
	NsShowUrl();
	snprintf(status, sizeof(status), "Fetching %s ...", url);
	NsShowStatus(status);

	memset(&resp, 0, sizeof(resp));
	int rc = xe_http_get(url, "GET", kNsUa, &resp, NS_BODY_MAX);
	if (rc != 0 && (!resp.body || !resp.body_len)) {
		NsShowStatus(resp.error[0] ? resp.error : "Fetch failed.");
		xe_http_response_free(&resp);
		return -1;
	}
	if (!resp.body) {
		NsShowStatus(resp.error[0] ? resp.error : "Empty response.");
		xe_http_response_free(&resp);
		return -1;
	}
	_KePrint("[css] fetched %d bytes tls=%d status=%d\n", (int)resp.body_len, resp.tls, resp.status);
	xe_heap_check("post-fetch");

	blen = resp.body_len;
	if (blen > NS_RAW_MAX)
		blen = NS_RAW_MAX;
	memcpy(s_body, resp.body, blen);
	s_body[blen] = 0;
	body = s_body;
	left_tls = resp.https_to_http;
	used_tls = resp.tls && !left_tls;
	xe_http_response_free(&resp);

	/* Fresh JS context per page; drops the previous page's timers. */
	s_postPage = false;
	NsJsBegin(url);
	NsJsScanTitle(body, blen);

	NsCollectCss(body, blen);
	NsCollectCssLinks(body, blen, url);
	_KePrint("[css] author sheet %d bytes\n", (int)strlen(s_cssBuf));
	xe_heap_check("post-collect");
	NsCssBegin();
	_KePrint("[css] begin ok\n");
	xe_heap_check("post-begin");
	NsCssAddSheet(s_cssBuf, strlen(s_cssBuf));
	_KePrint("[css] sheet added\n");
	xe_heap_check("post-sheet");
	NsShowStatus("Laying out...");
	NsHtmlToText(body, blen, s_text, sizeof(s_text));
	_KePrint("[css] rendered %d bytes\n", (int)strlen(s_text));
	NsCssEnd();
	/* Inline scripts ran during the parse; the context (and any timers
	 * they armed) outlives the fetch until the next page. */
	NsJsFlushConsole();
	_KePrint("[js] %s ok, timers=%d\n", NsJsVersion(), NsJsTimerCount());
	s_postPage = true; /* later document.write appends at page end */
	s_googleHome = strstr(url, "google.") && !strstr(url, "/search");
	if (s_googleHome)
		g_pageCenter = 1;
	if (!s_text[0])
		strcpy(s_text, "(empty page)");
	NsShowStatus(s_text);
	return 0;
}

/* "cats" or "weather london" is a search. example.com and https:// stay URLs. */
static int NsIsSearch(const char* s) {
	if (!s || !s[0])
		return 0;
	if (strstr(s, "://"))
		return 0;
	if (strchr(s, ' ') || strchr(s, '\t'))
		return 1;
	if (!strchr(s, '.'))
		return 1;
	return 0;
}

static void NsEncodeQuery(const char* in, char* out, size_t outsz) {
	size_t o = 0;
	static const char* hex = "0123456789ABCDEF";
	for (; *in && o + 4 < outsz; in++) {
		unsigned char c = (unsigned char)*in;
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
			c == '-' || c == '_' || c == '.') {
			out[o++] = (char)c;
		} else if (c == ' ' || c == '\t') {
			out[o++] = '+';
		} else {
			out[o++] = '%';
			out[o++] = hex[c >> 4];
			out[o++] = hex[c & 15];
		}
	}
	out[o] = 0;
}

static void NsSubmitQuery(int lucky) {
	char q[NS_URL_MAX];
	NsEncodeQuery(s_query, q, sizeof(q));
	if (lucky)
		snprintf(s_url, sizeof(s_url), "https://www.google.com/search?q=%s&btnI=1&gbv=1&hl=en", q);
	else
		snprintf(s_url, sizeof(s_url), "https://www.google.com/search?q=%s&gbv=1&hl=en", q);
	s_urlCursor = strlen(s_url);
	NsShowUrl();
	NsFetch(s_url);
}

static void NsGoHandler(ChWidget* wid, ChWindow* win) {
	char q[NS_URL_MAX];
	(void)wid;
	(void)win;
	if (NsIsSearch(s_url)) {
		NsEncodeQuery(s_url, q, sizeof(q));
		snprintf(s_url, sizeof(s_url), "https://www.google.com/search?q=%s&gbv=1&hl=en", q);
		s_urlCursor = strlen(s_url);
		NsShowUrl();
	}
	NsFetch(s_url);
}

static void NsUrlClampCursor(void) {
	size_t len = strlen(s_url);
	if (s_urlCursor > len)
		s_urlCursor = len;
}

static void NsHandleKey(int raw) {
	if (s_qFocus) {
		size_t len = strlen(s_query);
		if (s_qCursor > len)
			s_qCursor = len;
		if (raw == NS_KEY_LEFT || raw == NS_KEY_HOME) {
			if (raw == NS_KEY_HOME)
				s_qCursor = 0;
			else if (s_qCursor > 0)
				s_qCursor--;
			NsPageRender();
			return;
		}
		if (raw == NS_KEY_RIGHT || raw == NS_KEY_END) {
			if (raw == NS_KEY_END)
				s_qCursor = len;
			else if (s_qCursor < len)
				s_qCursor++;
			NsPageRender();
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
		char rawkey = ChitralekhaGetKeyPress(raw);
		if (rawkey == KEY_RETURN) {
			NsSubmitQuery(0);
			return;
		}
		if (rawkey == KEY_BACKSPACE) {
			if (s_qCursor > 0) {
				memmove(s_query + s_qCursor - 1, s_query + s_qCursor, len - s_qCursor + 1);
				s_qCursor--;
				NsPageRender();
			}
			return;
		}
		int qc = ChitralekhaKeyToASCII(raw);
		if (qc >= 32 && qc < 127 && len + 1 < sizeof(s_query)) {
			memmove(s_query + s_qCursor + 1, s_query + s_qCursor, len - s_qCursor + 1);
			s_query[s_qCursor] = (char)qc;
			s_qCursor++;
			NsPageRender();
		}
		return;
	}
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
		int vis = NsVisibleLines();
		NsPageScroll(vis > 1 ? -(vis - 1) : -1);
		return;
	}
	if (raw == NS_KEY_PGDN) {
		int vis = NsVisibleLines();
		NsPageScroll(vis > 1 ? (vis - 1) : 1);
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
		int mx = e->dword;
		int my = e->dword2;
		int button = e->dword3;
		ChWindow* mouseWin = ChGetWindowByHandle(s_mainWin, handle);
		/* Wheel is button value 5 (up) or 6 (down), not a click bit. */
		if (button == 0x05 || button == 0x06) {
			NsPageScroll(button == 0x06 ? 3 : -3);
			memset(e, 0, sizeof(PostEvent));
			break;
		}
		ChWindowHandleMouse(mouseWin, mx, my, button);
		if (button && s_mainWin && s_mainWin->info) {
			int lx = mx - s_mainWin->info->x;
			int ly = my - s_mainWin->info->y;
			if (s_qHave && lx >= s_qX && lx < s_qX + s_qW && ly >= s_qY && ly < s_qY + s_qH) {
				int rel = lx - s_qX - 8;
				size_t i = 0;
				int w = 0;
				ChFont* f = s_pageBox ? s_pageBox->font : NULL;
				s_qFocus = 1;
				if (rel < 0)
					rel = 0;
				if (f) {
					while (s_query[i]) {
						int cw = ChFontGetWidthChar(f, s_query[i]);
						if (w + cw / 2 > rel)
							break;
						w += cw;
						i++;
					}
				} else {
					i = strlen(s_query);
				}
				s_qCursor = i;
				NsPageRender();
			} else if (s_btnHave && lx >= s_btnX && lx < s_btnX + s_btnW && ly >= s_btnY &&
					   ly < s_btnY + s_btnH) {
				NsSubmitQuery(0);
			} else if (s_luckyHave && lx >= s_luckyX && lx < s_luckyX + s_luckyW &&
					   ly >= s_luckyY && ly < s_luckyY + s_luckyH) {
				NsSubmitQuery(1);
			} else if (button) {
				int hit = -1;
				for (int li = 0; li < s_nlinks; li++) {
					if (!s_links[li].have)
						continue;
					if (lx >= s_links[li].x && lx < s_links[li].x + s_links[li].w &&
						ly >= s_links[li].y && ly < s_links[li].y + s_links[li].h) {
						hit = li;
						break;
					}
				}
				if (hit >= 0) {
					const char* h = s_links[hit].href;
					char abs[NS_URL_MAX];
					if (!strncmp(h, "https://", 8) || !strncmp(h, "http://", 7)) {
						snprintf(abs, sizeof(abs), "%s", h);
					} else if (h[0] == '/' && h[1] == '/') {
						snprintf(abs, sizeof(abs), "https:%s", h);
					} else if (h[0] == '/') {
						const char* sc = strstr(s_url, "://");
						const char* host = sc ? sc + 3 : s_url;
						const char* slash = strchr(host, '/');
						size_t origin = slash ? (size_t)(slash - s_url) : strlen(s_url);
						if (origin + strlen(h) + 1 < sizeof(abs)) {
							memcpy(abs, s_url, origin);
							strcpy(abs + origin, h);
						} else {
							snprintf(abs, sizeof(abs), "%s", h);
						}
					} else {
						snprintf(abs, sizeof(abs), "%s", h);
					}
					snprintf(s_url, sizeof(s_url), "%s", abs);
					s_urlCursor = strlen(s_url);
					NsShowUrl();
					NsFetch(s_url);
				} else if (s_qFocus) {
					s_qFocus = 0;
					NsPageRender();
				}
			}
		}
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
	/* JS sink is process-wide: both the GUI page pass and the headless
	 * --selftest run route console/document.write output through it. */
	NsJsSetSink(NsJsSink);

	/* Headless pipeline self-test: fetch + CSS + render with serial
     * markers, no GUI. Xeneva drops the program name: the app sees only
     * user args, so scan everything (argv[0] is already "--selftest"
     * under init's TERM runner). */
	for (int a = 0; a < argc; a++) {
		if (argv[a] &&
			(!strcmp(argv[a], "--selftest") || !strcmp(argv[a], "selftest")) &&
			a + 1 < argc && argv[a + 1] && argv[a + 1][0]) {
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
			_KePrint("[cssselftest] done, text %d bytes, first 120 chars:\n", (int)strlen(s_text));
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

	ChWindowBroadcastIcon(s_app, "/icons/netsurf.bmp");

	/* Second size bucket for CSS-enlarged runs (headings). Per-object
     * atlases, so sizes coexist safely. */
	s_fontBig = ChInitialiseFont(CALIBRI);
	if (s_fontBig)
		ChFontSetSize(s_fontBig, 20);

	NsShowUrl();
	NsShowStatus("NetSurf for XenevaOS (bootstrap).\n\nType a host like example.com,\n"
				 "press Enter or Go.\n\nLeft/Right/Home/End move in the URL bar.\n"
				 "Up/Down/PgUp/PgDn scroll the page.\n\n"
				 "A name with a dot opens that site.\n"
				 "Words with no dot search Google.\n");

	ChWindowPaint(s_mainWin);

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	setjmp(s_mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(s_app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT) {
			/* Idle: fire due page timers (document.write appends repaint
			 * the view), then a short sleep so the next tick lands. */
			if (NsJsTick())
				NsPageRender();
			_KeProcessSleep(10);
		}
	}
	return 0;
}
