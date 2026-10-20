/**
 * YouTube watch playback.
 *
 * The 2026 player JSON has no media file URL. It has a server-ABR URL whose
 * n= parameter is rejected until the player script transforms it. This file
 * runs that solver (the same Unlicense bundle yt-dlp uses) in QuickJS, POSTs
 * a VideoPlaybackAbrRequest, follows the UMP redirect, and decodes the
 * smallest AVC fragment with h264bsd.
 */

#include "yt.h"
#include "yt_jsc.h"

#include <https.h>
#include <quickjs.h>
#include <h264bsd_decoder.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define YT_UMP_MAX (512 * 1024)
#define YT_ANNEX_MAX (384 * 1024)
#define YT_PLAYER_MAX (4 * 1024 * 1024)

static char s_status[160];
static uint8_t* s_annex;
static size_t s_annex_len;
static size_t s_annex_pos;
static storage_t* s_dec;
static uint32_t* s_frame;
static int s_fw;
static int s_fh;
static int s_playing;

const char* NsYtStatus(void) {
	return s_status;
}

const uint32_t* NsYtFrame(int* w, int* h) {
	if (!s_frame || !s_fw || !s_fh)
		return NULL;
	if (w)
		*w = s_fw;
	if (h)
		*h = s_fh;
	return s_frame;
}

static void yt_status(const char* s) {
	snprintf(s_status, sizeof(s_status), "%s", s ? s : "");
}

static const char* yt_find(const char* h, const char* end, const char* needle) {
	size_t n = strlen(needle);
	const char* p;
	if (!h || h >= end || n == 0)
		return NULL;
	for (p = h; p + n <= end; p++) {
		if (!memcmp(p, needle, n))
			return p;
	}
	return NULL;
}

static int yt_json_str(const char* h, const char* end, const char* key, char* out, size_t outsz) {
	char pat[80];
	const char* p;
	size_t n = 0;
	if (!out || !outsz)
		return 0;
	out[0] = 0;
	snprintf(pat, sizeof(pat), "\"%s\":\"", key);
	p = yt_find(h, end, pat);
	if (!p)
		return 0;
	p += strlen(pat);
	while (p < end && *p != '"' && n + 1 < outsz) {
		if (*p == '\\' && p + 1 < end) {
			p++;
			if (*p == 'u' && p + 4 < end) {
				/* \u0026 and friends used in googlevideo URLs. */
				unsigned v = 0;
				int k;
				for (k = 1; k <= 4; k++) {
					char c = p[k];
					v <<= 4;
					if (c >= '0' && c <= '9')
						v += (unsigned)(c - '0');
					else if (c >= 'a' && c <= 'f')
						v += (unsigned)(c - 'a' + 10);
					else if (c >= 'A' && c <= 'F')
						v += (unsigned)(c - 'A' + 10);
				}
				if (v >= 32 && v < 127)
					out[n++] = (char)v;
				p += 5;
				continue;
			}
			if (*p != '"' && *p != '\\' && *p != '/')
				out[n++] = *p;
			else
				out[n++] = *p;
			p++;
			continue;
		}
		out[n++] = *p++;
	}
	out[n] = 0;
	return n > 0;
}

static char* yt_video_id(const char* url) {
	const char* p;
	static char id[16];
	size_t n = 0;
	if (!url)
		return NULL;
	p = strstr(url, "v=");
	if (!p) {
		p = strstr(url, "youtu.be/");
		if (!p)
			return NULL;
		p += 9;
	} else {
		p += 2;
	}
	while (*p && *p != '&' && *p != '#' && n + 1 < sizeof(id))
		id[n++] = *p++;
	id[n] = 0;
	return n >= 6 ? id : NULL;
}

static void pb_varint(uint8_t** o, size_t* n, size_t cap, uint64_t v) {
	while (*n + 1 < cap) {
		uint8_t b = (uint8_t)(v & 0x7f);
		v >>= 7;
		if (v)
			b |= 0x80;
		(*o)[(*n)++] = b;
		if (!v)
			break;
	}
}

static void pb_key(uint8_t** o, size_t* n, size_t cap, unsigned field, unsigned wire) {
	pb_varint(o, n, cap, ((uint64_t)field << 3) | wire);
}

static void pb_bytes(uint8_t** o, size_t* n, size_t cap, unsigned field, const void* b, size_t len) {
	pb_key(o, n, cap, field, 2);
	pb_varint(o, n, cap, len);
	if (*n + len < cap) {
		memcpy(*o + *n, b, len);
		*n += len;
	}
}

static void pb_u64(uint8_t** o, size_t* n, size_t cap, unsigned field, uint64_t v) {
	pb_key(o, n, cap, field, 0);
	pb_varint(o, n, cap, v);
}

static int b64url_decode(const char* in, uint8_t* out, size_t outcap, size_t* outlen) {
	static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	char tmp[2048];
	size_t n = strlen(in);
	size_t i, o = 0;
	if (n >= sizeof(tmp) - 4)
		n = sizeof(tmp) - 4;
	for (i = 0; i < n; i++) {
		char c = in[i];
		if (c == '-')
			c = '+';
		else if (c == '_')
			c = '/';
		tmp[i] = c;
	}
	while ((n - i) % 4) {
		(void)i;
		break;
	}
	while (n % 4)
		tmp[n++] = '=';
	tmp[n] = 0;
	for (i = 0; i + 4 <= n && o + 1 < outcap; i += 4) {
		int v[4];
		int k;
		unsigned val = 0;
		for (k = 0; k < 4; k++) {
			const char* f = tmp[i + k] == '=' ? NULL : strchr(tbl, tmp[i + k]);
			v[k] = f ? (int)(f - tbl) : 0;
			val = (val << 6) | (unsigned)v[k];
		}
		if (o < outcap)
			out[o++] = (uint8_t)((val >> 16) & 0xff);
		if (tmp[i + 2] != '=' && o < outcap)
			out[o++] = (uint8_t)((val >> 8) & 0xff);
		if (tmp[i + 3] != '=' && o < outcap)
			out[o++] = (uint8_t)(val & 0xff);
	}
	*outlen = o;
	return o > 0;
}

static int solve_n(const char* player, size_t plen, const char* nin, char* nout, size_t noutsz) {
	JSRuntime* rt;
	JSContext* ctx;
	JSValue global, fn, data, reqs, req, ch, out, responses, one, map, val;
	const char* s;
	int ok = 0;
	rt = JS_NewRuntime();
	if (!rt)
		return -1;
	JS_SetMemoryLimit(rt, 64 * 1024 * 1024);
	ctx = JS_NewContext(rt);
	if (!ctx) {
		JS_FreeRuntime(rt);
		return -1;
	}
	{
		JSValue ev = JS_Eval(ctx, yt_jsc_js, yt_jsc_js_len, "<jsc>", JS_EVAL_TYPE_GLOBAL);
		if (JS_IsException(ev)) {
			JS_FreeValue(ctx, ev);
			JS_FreeContext(ctx);
			JS_FreeRuntime(rt);
			return -1;
		}
		JS_FreeValue(ctx, ev);
	}
	global = JS_GetGlobalObject(ctx);
	fn = JS_GetPropertyStr(ctx, global, "jsc");
	data = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, data, "type", JS_NewString(ctx, "player"));
	JS_SetPropertyStr(ctx, data, "player", JS_NewStringLen(ctx, player, plen));
	JS_SetPropertyStr(ctx, data, "output_preprocessed", JS_FALSE);
	reqs = JS_NewArray(ctx);
	req = JS_NewObject(ctx);
	JS_SetPropertyStr(ctx, req, "type", JS_NewString(ctx, "n"));
	ch = JS_NewArray(ctx);
	JS_SetPropertyUint32(ctx, ch, 0, JS_NewString(ctx, nin));
	JS_SetPropertyStr(ctx, req, "challenges", ch);
	JS_SetPropertyUint32(ctx, reqs, 0, req);
	JS_SetPropertyStr(ctx, data, "requests", reqs);
	{
		JSValue args[1];
		args[0] = data;
		out = JS_Call(ctx, fn, JS_UNDEFINED, 1, args);
	}
	if (!JS_IsException(out)) {
		responses = JS_GetPropertyStr(ctx, out, "responses");
		one = JS_GetPropertyUint32(ctx, responses, 0);
		map = JS_GetPropertyStr(ctx, one, "data");
		val = JS_GetPropertyStr(ctx, map, nin);
		s = JS_ToCString(ctx, val);
		if (s && s[0]) {
			snprintf(nout, noutsz, "%s", s);
			ok = 1;
		}
		if (s)
			JS_FreeCString(ctx, s);
		JS_FreeValue(ctx, val);
		JS_FreeValue(ctx, map);
		JS_FreeValue(ctx, one);
		JS_FreeValue(ctx, responses);
	}
	JS_FreeValue(ctx, out);
	JS_FreeValue(ctx, fn);
	JS_FreeValue(ctx, global);
	JS_FreeContext(ctx);
	JS_FreeRuntime(rt);
	return ok ? 0 : -1;
}

static void replace_query(char* url, const char* key, const char* value) {
	char pat[16];
	char* p;
	char* amp;
	size_t klen;
	snprintf(pat, sizeof(pat), "%s=", key);
	p = strstr(url, pat);
	if (!p)
		return;
	/* Avoid matching a suffix of another key. */
	if (p != url && p[-1] != '?' && p[-1] != '&') {
		p = strstr(p + 1, pat);
		if (!p)
			return;
	}
	p += strlen(pat);
	amp = strchr(p, '&');
	klen = strlen(value);
	if (amp) {
		size_t old = (size_t)(amp - p);
		size_t rest = strlen(amp);
		if (klen != old)
			memmove(p + klen, amp, rest + 1);
		memcpy(p, value, klen);
	} else {
		memcpy(p, value, klen + 1);
	}
}

/* Pull SPS/PPS and length-prefixed NALs out of a fragmented MP4. */
static const uint8_t* yt_fourcc(const uint8_t* buf, size_t len, const char* tag, size_t from) {
	size_t i;
	for (i = from; i + 4 <= len; i++) {
		if (!memcmp(buf + i, tag, 4))
			return buf + i;
	}
	return NULL;
}

static int annex_from_ump(const uint8_t* buf, size_t len) {
	const uint8_t* avcc_tag;
	const uint8_t* a;
	const uint8_t* aend = buf + len;
	const uint8_t* mtag;
	uint8_t* out;
	size_t o = 0;
	size_t from = 0;
	int nsps;
	int i_sps;
	if (!yt_fourcc(buf, len, "ftyp", 0))
		return -1;
	avcc_tag = yt_fourcc(buf, len, "avcC", 0);
	if (!avcc_tag || avcc_tag + 8 >= aend)
		return -1;
	out = (uint8_t*)malloc(YT_ANNEX_MAX);
	if (!out)
		return -1;
	/* avcC box payload starts immediately after the fourcc. */
	a = avcc_tag + 4;
	a += 5;
	if (a >= aend) {
		free(out);
		return -1;
	}
	nsps = (*a++) & 0x1f;
	for (i_sps = 0; i_sps < nsps && a + 2 <= aend; i_sps++) {
		unsigned sl = ((unsigned)a[0] << 8) | a[1];
		a += 2;
		if (a + sl > aend || o + 4 + sl > YT_ANNEX_MAX)
			break;
		out[o++] = 0;
		out[o++] = 0;
		out[o++] = 0;
		out[o++] = 1;
		memcpy(out + o, a, sl);
		o += sl;
		a += sl;
	}
	if (a < aend) {
		int npps = *a++;
		int i_pps;
		for (i_pps = 0; i_pps < npps && a + 2 <= aend; i_pps++) {
			unsigned sl = ((unsigned)a[0] << 8) | a[1];
			a += 2;
			if (a + sl > aend || o + 4 + sl > YT_ANNEX_MAX)
				break;
			out[o++] = 0;
			out[o++] = 0;
			out[o++] = 0;
			out[o++] = 1;
			memcpy(out + o, a, sl);
			o += sl;
			a += sl;
		}
	}
	while ((mtag = yt_fourcc(buf, len, "mdat", from)) != NULL) {
		const uint8_t* box;
		const uint8_t* m;
		const uint8_t* mend;
		uint32_t sz;
		if (mtag < buf + 4)
			break;
		box = mtag - 4;
		sz = ((uint32_t)box[0] << 24) | ((uint32_t)box[1] << 16) | ((uint32_t)box[2] << 8) | box[3];
		if (sz < 8 || box + sz > aend)
			break;
		m = mtag + 4;
		mend = box + sz;
		while (m + 4 <= mend && o + 4 < YT_ANNEX_MAX) {
			uint32_t nl = ((uint32_t)m[0] << 24) | ((uint32_t)m[1] << 16) | ((uint32_t)m[2] << 8) | m[3];
			m += 4;
			if (nl < 1 || m + nl > mend)
				break;
			if (o + 4 + nl > YT_ANNEX_MAX)
				break;
			out[o++] = 0;
			out[o++] = 0;
			out[o++] = 0;
			out[o++] = 1;
			memcpy(out + o, m, nl);
			o += nl;
			m += nl;
		}
		from = (size_t)(mend - buf);
	}
	free(s_annex);
	s_annex = out;
	s_annex_len = o;
	s_annex_pos = 0;
	return o > 32 ? 0 : -1;
}

static int grab_avc(const char* html, const char* end, int* itag, unsigned long long* lmt, int* height) {
	const char* p = html;
	*itag = 0;
	*lmt = 0;
	*height = 144;
	while ((p = yt_find(p, end, "avc1")) != NULL) {
		const char* back = p - 400;
		const char* it;
		const char* lm;
		const char* ht;
		if (back < html)
			back = html;
		it = yt_find(back, p, "\"itag\":");
		lm = yt_find(back, p, "\"lastModified\":\"");
		ht = yt_find(back, p, "\"height\":");
		if (it) {
			*itag = atoi(it + 7);
			if (lm) {
				const char* d = lm + 16;
				*lmt = 0;
				while (*d >= '0' && *d <= '9') {
					*lmt = *lmt * 10 + (unsigned)(*d - '0');
					d++;
				}
			}
			if (ht)
				*height = atoi(ht + 9);
			if (*itag > 0)
				return 0;
		}
		p += 4;
	}
	return -1;
}

void NsYtStart(const char* page_url, const char* html, size_t html_len) {
	const char* end;
	const char* player_path;
	char* vid;
	char sabr[3000];
	char ustream_b64[1800];
	char n_in[64];
	char n_out[64];
	char player_url[256];
	uint8_t ustream[1600];
	size_t ustream_len = 0;
	int itag = 0;
	int height = 144;
	unsigned long long lmt = 0;
	xe_http_response resp;
	uint8_t body[1400];
	uint8_t* bp = body;
	size_t bn = 0;
	int hop;

	s_playing = 0;
	s_fw = s_fh = 0;
	free(s_frame);
	s_frame = NULL;
	if (s_dec) {
		h264bsdShutdown(s_dec);
		h264bsdFree(s_dec);
		s_dec = NULL;
	}
	yt_status("");
	vid = yt_video_id(page_url);
	if (!vid || !html)
		return;
	end = html + html_len;
	if (!yt_json_str(html, end, "serverAbrStreamingUrl", sabr, sizeof(sabr))) {
		yt_status("YouTube: no SABR URL in the player");
		return;
	}
	if (!yt_json_str(html, end, "videoPlaybackUstreamerConfig", ustream_b64, sizeof(ustream_b64))) {
		yt_status("YouTube: no ustreamer config");
		return;
	}
	if (grab_avc(html, end, &itag, &lmt, &height) < 0) {
		yt_status("YouTube: no AVC stream");
		return;
	}
	if (!b64url_decode(ustream_b64, ustream, sizeof(ustream), &ustream_len)) {
		yt_status("YouTube: bad ustreamer config");
		return;
	}
	n_in[0] = 0;
	{
		const char* np = strstr(sabr, "n=");
		size_t k = 0;
		if (np) {
			np += 2;
			while (*np && *np != '&' && k + 1 < sizeof(n_in))
				n_in[k++] = *np++;
			n_in[k] = 0;
		}
	}
	player_path = yt_find(html, end, "/s/player/");
	if (!player_path || !n_in[0]) {
		yt_status("YouTube: player script missing");
		return;
	}
	{
		size_t k = 0;
		while (player_path[k] && player_path[k] != '"' && k + 1 < sizeof(player_url) - 24) {
			k++;
		}
		snprintf(player_url, sizeof(player_url), "https://www.youtube.com%.*s", (int)k, player_path);
	}
	yt_status("YouTube: solving player challenge");
	memset(&resp, 0, sizeof(resp));
	if (xe_http_get(player_url, "GET", "Mozilla/5.0", &resp, YT_PLAYER_MAX) != 0 || !resp.body) {
		yt_status(resp.error[0] ? resp.error : "YouTube: player fetch failed");
		xe_http_response_free(&resp);
		return;
	}
	n_out[0] = 0;
	if (solve_n(resp.body, resp.body_len, n_in, n_out, sizeof(n_out)) != 0) {
		yt_status("YouTube: n challenge failed");
		xe_http_response_free(&resp);
		return;
	}
	xe_http_response_free(&resp);
	replace_query(sabr, "n", n_out);

	/* ClientAbrState: stickyResolution=21, playerTimeMs=28, visibility=34,
	 * playbackRate=35 (float), enabledTrackTypesBitfield=40 (2 = video). */
	{
		uint8_t abr[64];
		uint8_t* ap = abr;
		size_t an = 0;
		uint8_t fmt[32];
		uint8_t* fp = fmt;
		size_t fn = 0;
		uint8_t ci[80];
		uint8_t* cp = ci;
		size_t cn = 0;
		uint8_t sc[120];
		uint8_t* sp = sc;
		size_t sn = 0;
		float rate = 1.f;
		pb_u64(&ap, &an, sizeof(abr), 21, (uint64_t)height);
		pb_u64(&ap, &an, sizeof(abr), 28, 0);
		pb_u64(&ap, &an, sizeof(abr), 34, 1);
		pb_key(&ap, &an, sizeof(abr), 35, 5);
		if (an + 4 < sizeof(abr)) {
			memcpy(ap + an, &rate, 4);
			an += 4;
		}
		pb_u64(&ap, &an, sizeof(abr), 40, 2);
		pb_u64(&fp, &fn, sizeof(fmt), 1, (uint64_t)itag);
		if (lmt)
			pb_u64(&fp, &fn, sizeof(fmt), 2, lmt);
		pb_u64(&cp, &cn, sizeof(ci), 16, 1);
		pb_bytes(&cp, &cn, sizeof(ci), 17, "2.20261016.01.00", 16);
		pb_bytes(&sp, &sn, sizeof(sc), 1, ci, cn);
		pb_bytes(&bp, &bn, sizeof(body), 1, abr, an);
		pb_bytes(&bp, &bn, sizeof(body), 17, fmt, fn);
		pb_bytes(&bp, &bn, sizeof(body), 5, ustream, ustream_len);
		pb_bytes(&bp, &bn, sizeof(body), 19, sc, sn);
	}

	for (hop = 0; hop < 4; hop++) {
		const char* redir;
		memset(&resp, 0, sizeof(resp));
		if (xe_http_post(sabr, "Mozilla/5.0", "application/vnd.yt-ump", body, bn, &resp, YT_UMP_MAX) !=
				0 ||
			!resp.body) {
			yt_status(resp.error[0] ? resp.error : "YouTube: SABR post failed");
			xe_http_response_free(&resp);
			return;
		}
		if (annex_from_ump((const uint8_t*)resp.body, resp.body_len) == 0) {
			xe_http_response_free(&resp);
			s_dec = h264bsdAlloc();
			if (!s_dec || h264bsdInit(s_dec, 0) != H264BSD_RDY) {
				yt_status("YouTube: decoder init failed");
				return;
			}
			s_playing = 1;
			snprintf(s_status, sizeof(s_status), "YouTube: playing itag %d", itag);
			return;
		}
		redir = yt_find(resp.body, resp.body + resp.body_len, "https://");
		if (!redir) {
			yt_status("YouTube: SABR returned no media");
			xe_http_response_free(&resp);
			return;
		}
		{
			size_t k = 0;
			while (redir[k] && redir[k] != '"' && (unsigned char)redir[k] >= 32 && k + 1 < sizeof(sabr))
				k++;
			memcpy(sabr, redir, k);
			sabr[k] = 0;
		}
		xe_http_response_free(&resp);
	}
	yt_status("YouTube: too many SABR redirects");
}

int NsYtTick(void) {
	u32 readb = 0;
	u32 pic = 0, idr = 0, err = 0;
	u32* bgra;
	int guard = 0;
	if (!s_playing || !s_dec || !s_annex)
		return 0;
	while (s_annex_pos < s_annex_len && guard < 8) {
		u32 rc = h264bsdDecode(s_dec, s_annex + s_annex_pos,
							   (u32)(s_annex_len - s_annex_pos), 0, &readb);
		if (readb == 0)
			readb = 1;
		s_annex_pos += readb;
		guard++;
		if (rc == H264BSD_PIC_RDY) {
			bgra = h264bsdNextOutputPictureBGRA(s_dec, &pic, &idr, &err);
			if (bgra) {
				u32 w = h264bsdPicWidth(s_dec) * 16;
				u32 h = h264bsdPicHeight(s_dec) * 16;
				size_t n;
				if (w == 0 || h == 0 || w > 640 || h > 360)
					return 0;
				n = (size_t)w * (size_t)h;
				if (!s_frame || s_fw != (int)w || s_fh != (int)h) {
					free(s_frame);
					s_frame = (uint32_t*)malloc(n * sizeof(uint32_t));
					s_fw = (int)w;
					s_fh = (int)h;
				}
				if (s_frame)
					memcpy(s_frame, bgra, n * sizeof(uint32_t));
				return s_frame != NULL;
			}
		}
		if (rc == H264BSD_ERROR || rc == H264BSD_MEMALLOC_ERROR) {
			s_playing = 0;
			yt_status("YouTube: decode error");
			return 0;
		}
	}
	if (s_annex_pos >= s_annex_len)
		s_playing = 0;
	return 0;
}
