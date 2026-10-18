/**
* BSD 2-Clause License
*
* Copyright (c) 2026, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

/*
 * HTTP/1.1 GET and HEAD over XEClib TCP, with TLS 1.2/1.3 when the URL
 * is https. Certificate chain and hostname are checked. ALPN is http/1.1.
 */

#include "https.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/netdb.h>
#include <arpa/inet.h>
#include <sys/_kefile.h>
#include <sys/_keproc.h>

#include "mbedtls/build_info.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"

extern "C" {
extern const unsigned char xe_cacert_pem[];
extern const unsigned int xe_cacert_pem_len;
}

/* XEClib exports receive(), not a C recv(). */
static int xe_recv(int fd, void* buf, size_t len) {
	iovec iov;
	msghdr hdr;
	iov.iov_base = buf;
	iov.iov_len = len;
	hdr.msg_name = NULL;
	hdr.msg_namelen = 0;
	hdr.msg_iov = &iov;
	hdr.msg_iovlen = 1;
	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;
	return receive(fd, &hdr, 0);
}

#define XE_CLOCK_MIN 1577836800 /* 2020-01-01; epoch means the clock was never set */
#define XE_IDLE_MAX 150         /* 100 ms sleeps: 15 s */
#define XE_HDR_RAW 8192
#define XE_MAX_REDIRECTS 5

struct xe_url {
	int https;
	char host[256];
	char path[1024];
	uint16_t port;
};

struct xe_reader {
	int tls;
	int fd;
	mbedtls_ssl_context* ssl;
	unsigned char buf[2048];
	int len;
	int pos;
	int eof;
	int err;
};

static mbedtls_entropy_context g_entropy;
static mbedtls_ctr_drbg_context g_drbg;
static mbedtls_x509_crt g_cacert;
static int g_rng_ready;
static int g_ca_ready;

static void set_err(xe_http_response* out, const char* msg) {
	snprintf(out->error, sizeof(out->error), "%s", msg);
}

void xe_http_response_free(xe_http_response* out) {
	if (!out)
		return;
	free(out->body);
	out->body = NULL;
	out->body_len = 0;
}

static int ensure_rng(xe_http_response* out) {
	int ret;
	if (g_rng_ready)
		return 0;
	mbedtls_entropy_init(&g_entropy);
	mbedtls_ctr_drbg_init(&g_drbg);
	ret = mbedtls_ctr_drbg_seed(&g_drbg, mbedtls_entropy_func, &g_entropy,
								(const unsigned char*)"xeneva-https", 12);
	if (ret != 0) {
		set_err(out, "entropy unavailable");
		return -1;
	}
	g_rng_ready = 1;
	return 0;
}

static int ensure_ca(xe_http_response* out) {
	int ret;
	if (g_ca_ready)
		return 0;
	mbedtls_x509_crt_init(&g_cacert);
	ret = mbedtls_x509_crt_parse(&g_cacert, xe_cacert_pem, xe_cacert_pem_len);
	if (ret < 0 || g_cacert.raw.p == NULL) {
		set_err(out, "trust anchors unusable");
		return -1;
	}
	g_ca_ready = 1;
	return 0;
}

static int parse_url(const char* url, struct xe_url* u) {
	const char* p = url;
	const char* slash;
	const char* colon;
	size_t hlen;
	memset(u, 0, sizeof(*u));
	u->port = 80;
	strcpy(u->path, "/");
	if (!p || !p[0])
		return -1;
	if (!strncmp(p, "https://", 8)) {
		u->https = 1;
		u->port = 443;
		p += 8;
	} else if (!strncmp(p, "http://", 7)) {
		p += 7;
	}
	if (p[0] == '[')
		return -2;
	slash = strchr(p, '/');
	colon = strchr(p, ':');
	if (colon && (!slash || colon < slash)) {
		const char* d = colon + 1;
		int prt = 0;
		hlen = (size_t)(colon - p);
		while (*d >= '0' && *d <= '9') {
			prt = prt * 10 + (*d - '0');
			d++;
		}
		if (prt <= 0 || prt > 65535)
			return -1;
		u->port = (uint16_t)prt;
		slash = (*d == '/') ? d : NULL;
	} else {
		hlen = slash ? (size_t)(slash - p) : strlen(p);
	}
	if (hlen == 0 || hlen >= sizeof(u->host))
		return -1;
	memcpy(u->host, (void*)p, hlen);
	u->host[hlen] = 0;
	if (slash && slash[0]) {
		if (strlen(slash) >= sizeof(u->path))
			return -1;
		strcpy(u->path, slash);
	}
	return 0;
}

static void format_abs(const struct xe_url* u, const char* path, char* dst, size_t n) {
	int deflt = (u->https && u->port == 443) || (!u->https && u->port == 80);
	if (deflt)
		snprintf(dst, n, "%s://%s%s", u->https ? "https" : "http", u->host, path);
	else
		snprintf(dst, n, "%s://%s:%u%s", u->https ? "https" : "http", u->host, u->port, path);
}

static int join_location(const struct xe_url* cur, const char* loc, char* dst, size_t n) {
	if (!strncmp(loc, "https://", 8) || !strncmp(loc, "http://", 7)) {
		snprintf(dst, n, "%s", loc);
		return 0;
	}
	if (loc[0] == '/' && loc[1] == '/') {
		snprintf(dst, n, "%s:%s", cur->https ? "https" : "http", loc);
		return 0;
	}
	if (loc[0] == '/') {
		format_abs(cur, loc, dst, n);
		return 0;
	}
	{
		char base[1024];
		char joined[1200];
		const char* slash = strrchr(cur->path, '/');
		size_t dir = slash ? (size_t)(slash - cur->path + 1) : 1;
		if (dir >= sizeof(base))
			return -1;
		memcpy(base, (void*)cur->path, dir);
		base[dir] = 0;
		snprintf(joined, sizeof(joined), "%s%s", base, loc);
		format_abs(cur, joined, dst, n);
		return 0;
	}
}

static int io_read(struct xe_reader* r, unsigned char* buf, size_t len) {
	int idle = 0;
	while (idle < XE_IDLE_MAX) {
		int n;
		if (r->tls)
			n = mbedtls_ssl_read(r->ssl, buf, len);
		else
			n = xe_recv(r->fd, buf, len);
		if (n > 0)
			return n;
		if (n == 0 || n == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
			return 0;
		if (r->tls && n != MBEDTLS_ERR_SSL_WANT_READ && n != MBEDTLS_ERR_SSL_WANT_WRITE &&
			n != MBEDTLS_ERR_SSL_RECEIVED_NEW_SESSION_TICKET && n != MBEDTLS_ERR_SSL_TIMEOUT)
			return n;
		_KeProcessSleep(100);
		idle++;
	}
	return MBEDTLS_ERR_SSL_TIMEOUT;
}

static int io_write_all(struct xe_reader* r, const unsigned char* buf, size_t len) {
	size_t off = 0;
	int idle = 0;
	while (off < len && idle < XE_IDLE_MAX) {
		int n;
		if (r->tls)
			n = mbedtls_ssl_write(r->ssl, buf + off, len - off);
		else
			n = (int)sendto(r->fd, buf + off, len - off, 0, NULL, 0);
		if (n > 0) {
			off += (size_t)n;
			idle = 0;
			continue;
		}
		if (r->tls && n != MBEDTLS_ERR_SSL_WANT_READ && n != MBEDTLS_ERR_SSL_WANT_WRITE)
			return -1;
		_KeProcessSleep(100);
		idle++;
	}
	return off == len ? 0 : -1;
}

static int tls_send(void* ctx, const unsigned char* buf, size_t len) {
	struct xe_reader* r = (struct xe_reader*)ctx;
	int idle = 0;
	while (idle < XE_IDLE_MAX) {
		int n = (int)sendto(r->fd, buf, len, 0, NULL, 0);
		if (n > 0)
			return n;
		if (n == 0)
			return MBEDTLS_ERR_SSL_CONN_EOF;
		_KeProcessSleep(100);
		idle++;
	}
	return MBEDTLS_ERR_SSL_TIMEOUT;
}

static int tls_recv(void* ctx, unsigned char* buf, size_t len) {
	struct xe_reader* r = (struct xe_reader*)ctx;
	int idle = 0;
	while (idle < XE_IDLE_MAX) {
		int n = xe_recv(r->fd, buf, len);
		if (n > 0)
			return n;
		if (n == 0)
			return 0;
		_KeProcessSleep(100);
		idle++;
	}
	return MBEDTLS_ERR_SSL_TIMEOUT;
}

static int rd_fill(struct xe_reader* r) {
	int n;
	if (r->pos < r->len)
		return 0;
	if (r->eof)
		return 0;
	n = io_read(r, r->buf, sizeof(r->buf));
	if (n == 0) {
		r->eof = 1;
		return 0;
	}
	if (n < 0) {
		r->err = 1;
		return -1;
	}
	r->len = n;
	r->pos = 0;
	return 0;
}

static int rd_copy(struct xe_reader* r, unsigned char* dst, size_t n, size_t* got) {
	*got = 0;
	while (*got < n) {
		size_t have;
		size_t take;
		if (rd_fill(r) < 0)
			return -1;
		if (r->pos >= r->len)
			return 0;
		have = (size_t)(r->len - r->pos);
		take = n - *got;
		if (take > have)
			take = have;
		if (dst)
			memcpy(dst + *got, r->buf + r->pos, take);
		r->pos += (int)take;
		*got += take;
	}
	return 0;
}

static int rd_line(struct xe_reader* r, char* dst, size_t dstsz) {
	size_t i = 0;
	if (dstsz)
		dst[0] = 0;
	for (;;) {
		size_t one = 0;
		unsigned char c;
		if (rd_copy(r, &c, 1, &one) < 0)
			return -1;
		if (one == 0)
			return -1;
		if (c == '\n') {
			if (i < dstsz)
				dst[i] = 0;
			return 0;
		}
		if (c != '\r' && i + 1 < dstsz)
			dst[i++] = (char)c;
	}
}

static int hex_len(const char* s, size_t* out) {
	size_t v = 0;
	int any = 0;
	if (!s)
		return -1;
	while (*s == ' ' || *s == '\t')
		s++;
	while ((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F')) {
		int d = (*s >= '0' && *s <= '9') ? *s - '0' :
				(*s >= 'a' && *s <= 'f') ? *s - 'a' + 10 : *s - 'A' + 10;
		v = (v << 4) + (size_t)d;
		any = 1;
		s++;
	}
	if (!any)
		return -1;
	*out = v;
	return 0;
}

static const char* hdr_value(const char* hdr, const char* key) {
	size_t klen = strlen(key);
	const char* p = hdr;
	while (p && *p) {
		if (strncasecmp(p, key, klen) == 0 && p[klen] == ':') {
			p += klen + 1;
			while (*p == ' ' || *p == '\t')
				p++;
			return p;
		}
		p = strstr(p, "\r\n");
		if (!p)
			break;
		p += 2;
	}
	return NULL;
}

static void copy_token(const char* src, char* dst, size_t n) {
	size_t i = 0;
	while (src[i] && src[i] != '\r' && src[i] != '\n' && i + 1 < n) {
		dst[i] = src[i];
		i++;
	}
	dst[i] = 0;
}

static void tls_error(int ret, uint32_t flags, xe_http_response* out) {
	if (flags & MBEDTLS_X509_BADCERT_CN_MISMATCH)
		set_err(out, "certificate name mismatch");
	else if (flags & MBEDTLS_X509_BADCERT_EXPIRED)
		set_err(out, "certificate expired");
	else if (flags & MBEDTLS_X509_BADCERT_FUTURE)
		set_err(out, "certificate not yet valid");
	else if (flags & MBEDTLS_X509_BADCERT_NOT_TRUSTED)
		set_err(out, "certificate not trusted");
	else if (flags)
		set_err(out, "certificate rejected");
	else
		snprintf(out->error, sizeof(out->error), "TLS handshake failed (%d)", ret);
}

/* ssl keeps a pointer to its config, so both live until the response is read. */
static mbedtls_ssl_context g_ssl;
static mbedtls_ssl_config g_conf;

static void tls_free(struct xe_reader* r) {
	if (!r || !r->ssl)
		return;
	mbedtls_ssl_free(&g_ssl);
	mbedtls_ssl_config_free(&g_conf);
	r->ssl = NULL;
	r->tls = 0;
}

static int tls_handshake(struct xe_reader* r, const char* host, xe_http_response* out) {
	int ret;
	uint32_t flags = 0;
	static const char* alpn[] = { "http/1.1", NULL };

	if (time(NULL) < XE_CLOCK_MIN) {
		set_err(out, "clock not set");
		return -1;
	}
	if (ensure_rng(out) < 0 || ensure_ca(out) < 0)
		return -1;

	mbedtls_ssl_init(&g_ssl);
	mbedtls_ssl_config_init(&g_conf);
	r->ssl = &g_ssl;
	r->tls = 1;
	ret = mbedtls_ssl_config_defaults(&g_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
									 MBEDTLS_SSL_PRESET_DEFAULT);
	if (ret != 0) {
		set_err(out, "TLS config failed");
		goto fail;
	}
	mbedtls_ssl_conf_authmode(&g_conf, MBEDTLS_SSL_VERIFY_REQUIRED);
	mbedtls_ssl_conf_ca_chain(&g_conf, &g_cacert, NULL);
	mbedtls_ssl_conf_rng(&g_conf, mbedtls_ctr_drbg_random, &g_drbg);
	if (mbedtls_ssl_conf_alpn_protocols(&g_conf, (const char**)alpn) != 0) {
		set_err(out, "ALPN rejected");
		goto fail;
	}
	if (mbedtls_ssl_setup(&g_ssl, &g_conf) != 0) {
		set_err(out, "TLS setup failed");
		goto fail;
	}
	if (mbedtls_ssl_set_hostname(&g_ssl, host) != 0) {
		set_err(out, "SNI rejected");
		goto fail;
	}
	mbedtls_ssl_set_bio(&g_ssl, r, tls_send, tls_recv, NULL);
	while ((ret = mbedtls_ssl_handshake(&g_ssl)) != 0) {
		if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;
		flags = mbedtls_ssl_get_verify_result(&g_ssl);
		tls_error(ret, flags, out);
		goto fail;
	}
	flags = mbedtls_ssl_get_verify_result(&g_ssl);
	if (flags != 0) {
		tls_error(ret, flags, out);
		goto fail;
	}
	return 0;
fail:
	tls_free(r);
	return -1;
}

static int connect_host(const struct xe_url* u, xe_http_response* out) {
	hostent* ent;
	int sock;
	sockaddr_in dest;
	uint32_t ip;
	ent = gethostbyname(u->host);
	if (!ent) {
		snprintf(out->error, sizeof(out->error), "DNS failed for %s", u->host);
		return -1;
	}
	if (ent->h_addrtype != AF_INET || ent->h_length != 4 || !ent->h_addr_list[0]) {
		snprintf(out->error, sizeof(out->error), "No IPv4 address for %s", u->host);
		return -1;
	}
	ip = *(uint32_t*)ent->h_addr_list[0];
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		set_err(out, "socket() failed");
		return -1;
	}
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(u->port);
	memcpy(&dest.sin_addr, &ip, sizeof(ip));
	if (connect(sock, (sockaddr_*)&dest, sizeof(dest)) < 0) {
		snprintf(out->error, sizeof(out->error), "Connect to %s:%u failed", u->host, u->port);
		_KeCloseFile(sock);
		return -1;
	}
	return sock;
}

static int read_headers(struct xe_reader* r, char* hdr, size_t hdrsz) {
	size_t n = 0;
	hdr[0] = 0;
	while (n + 1 < hdrsz) {
		size_t one = 0;
		unsigned char c;
		if (rd_copy(r, &c, 1, &one) < 0)
			return -1;
		if (one == 0)
			return -1;
		hdr[n++] = (char)c;
		hdr[n] = 0;
		if (n >= 4 && hdr[n - 4] == '\r' && hdr[n - 3] == '\n' && hdr[n - 2] == '\r' &&
			hdr[n - 1] == '\n')
			return 0;
	}
	return -1;
}

static int status_code(const char* hdr) {
	const char* p;
	int code = 0;
	if (strncmp(hdr, "HTTP/", 5) != 0)
		return -1;
	p = strchr(hdr, ' ');
	if (!p)
		return -1;
	p++;
	while (*p >= '0' && *p <= '9') {
		code = code * 10 + (*p - '0');
		p++;
	}
	return code;
}

static int read_body(struct xe_reader* r, int chunked, long content_len, int want_body,
					 xe_http_response* out, size_t max_body) {
	size_t cap = max_body ? max_body : 1;
	unsigned char* body = (unsigned char*)malloc(cap + 1);
	size_t got = 0;
	if (!body)
		return -1;
	if (!want_body) {
		body[0] = 0;
		out->body = (char*)body;
		out->body_len = 0;
		return 0;
	}
	if (chunked) {
		for (;;) {
			char line[64];
			size_t sz = 0;
			size_t took = 0;
			if (rd_line(r, line, sizeof(line)) < 0)
				break;
			if (hex_len(line, &sz) < 0)
				break;
			if (sz == 0)
				break;
			while (sz) {
				size_t room = (got < cap) ? (cap - got) : 0;
				size_t ask = sz;
				size_t n = 0;
				if (ask > 2048)
					ask = 2048;
				if (rd_copy(r, room ? body + got : NULL, ask, &n) < 0 || n == 0) {
					sz = 0;
					break;
				}
				if (room) {
					size_t keep = n < room ? n : room;
					got += keep;
				}
				sz -= n;
				(void)took;
			}
			rd_line(r, line, sizeof(line));
		}
	} else if (content_len >= 0) {
		size_t left = (size_t)content_len;
		while (left) {
			size_t room = (got < cap) ? (cap - got) : 0;
			size_t ask = left > 2048 ? 2048 : left;
			size_t n = 0;
			if (rd_copy(r, room ? body + got : NULL, ask, &n) < 0 || n == 0)
				break;
			if (room)
				got += (n < room ? n : room);
			left -= n;
		}
	} else {
		for (;;) {
			size_t room = (got < cap) ? (cap - got) : 0;
			size_t n = 0;
			unsigned char tmp[2048];
			unsigned char* dst = room ? body + got : tmp;
			size_t ask = room ? (room > 2048 ? 2048 : room) : sizeof(tmp);
			if (rd_copy(r, dst, ask, &n) < 0 || n == 0)
				break;
			if (room)
				got += n;
			if (!room && r->eof)
				break;
		}
	}
	body[got] = 0;
	out->body = (char*)body;
	out->body_len = got;
	return 0;
}

static int fetch_once(const struct xe_url* u, const char* method, const char* ua,
					  xe_http_response* out, size_t max_body, char* location, size_t locsz) {
	struct xe_reader r;
	int fd;
	int head_only = method && strcmp(method, "HEAD") == 0;
	char req[1600];
	char hdr[XE_HDR_RAW];
	const char* te;
	const char* cl;
	const char* loc;
	int chunked = 0;
	long content_len = -1;
	char hosthdr[300];

	location[0] = 0;
	memset(&r, 0, sizeof(r));
	fd = connect_host(u, out);
	if (fd < 0)
		return -1;
	r.fd = fd;
	out->tls = u->https;
	if (u->https && tls_handshake(&r, u->host, out) < 0) {
		_KeCloseFile(fd);
		return -1;
	}
	if ((u->https && u->port == 443) || (!u->https && u->port == 80))
		snprintf(hosthdr, sizeof(hosthdr), "%s", u->host);
	else
		snprintf(hosthdr, sizeof(hosthdr), "%s:%u", u->host, u->port);
	snprintf(req, sizeof(req),
			 "%s %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: */*\r\n"
			 "Connection: close\r\n\r\n",
			 head_only ? "HEAD" : "GET", u->path, hosthdr, ua ? ua : "xeneva");
	if (io_write_all(&r, (const unsigned char*)req, strlen(req)) < 0) {
		set_err(out, "Send failed");
		goto fail;
	}
	if (read_headers(&r, hdr, sizeof(hdr)) < 0) {
		set_err(out, "No HTTP response");
		goto fail;
	}
	memcpy(out->headers, hdr, sizeof(out->headers) - 1);
	out->headers[sizeof(out->headers) - 1] = 0;
	out->status = status_code(hdr);
	if (out->status < 0) {
		set_err(out, "Bad HTTP status");
		goto fail;
	}
	te = hdr_value(hdr, "Transfer-Encoding");
	if (te && strncasecmp(te, "chunked", 7) == 0)
		chunked = 1;
	cl = hdr_value(hdr, "Content-Length");
	if (cl) {
		content_len = 0;
		while (*cl >= '0' && *cl <= '9') {
			content_len = content_len * 10 + (*cl - '0');
			cl++;
		}
	}
	loc = hdr_value(hdr, "Location");
	if (loc)
		copy_token(loc, location, locsz);
	if (read_body(&r, chunked, content_len, !head_only, out, max_body) < 0) {
		set_err(out, "Body read failed");
		goto fail;
	}
	if (r.ssl)
		mbedtls_ssl_close_notify(r.ssl);
	tls_free(&r);
	_KeCloseFile(fd);
	return 0;
fail:
	tls_free(&r);
	_KeCloseFile(fd);
	return -1;
}

int xe_http_get(const char* url, const char* method, const char* user_agent,
				xe_http_response* out, size_t max_body) {
	char current[1600];
	int hops;
	if (!out)
		return -1;
	memset(out, 0, sizeof(*out));
	if (!url || !url[0]) {
		set_err(out, "Bad URL");
		return -1;
	}
	snprintf(current, sizeof(current), "%s", url);
	for (hops = 0; hops <= XE_MAX_REDIRECTS; hops++) {
		struct xe_url u;
		char location[1200];
		char next[1600];
		int pu = parse_url(current, &u);
		int rc;
		xe_http_response_free(out);
		out->error[0] = 0;
		out->headers[0] = 0;
		out->status = 0;
		if (pu == -2) {
			set_err(out, "IPv6-only hosts are not reachable yet");
			return -1;
		}
		if (pu < 0) {
			set_err(out, "Bad URL");
			return -1;
		}
		rc = fetch_once(&u, method, user_agent, out, max_body, location, sizeof(location));
		if (rc < 0)
			return -1;
		if (out->status != 301 && out->status != 302 && out->status != 303 && out->status != 307 &&
			out->status != 308)
			break;
		if (!location[0])
			break;
		if (hops == XE_MAX_REDIRECTS) {
			set_err(out, "Too many redirects");
			return -1;
		}
		if (join_location(&u, location, next, sizeof(next)) < 0) {
			set_err(out, "Bad redirect");
			return -1;
		}
		if (u.https && !strncmp(next, "http://", 7)) {
			out->https_to_http = 1;
			_KePrint("[https] redirect left TLS for %s\r\n", next);
		}
		snprintf(current, sizeof(current), "%s", next);
		xe_http_response_free(out);
	}
	if (out->status >= 400) {
		snprintf(out->error, sizeof(out->error), "HTTP %d", out->status);
		return 22;
	}
	return 0;
}
