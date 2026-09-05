/**
* Minimal curl: HTTP/1.0 GET/HEAD over TCP (no TLS).
**/

#include <stdint.h>
#include <_xeneva.h>
#include <stdio.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/socket.h>
#include <sys/netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

static int is_prog(const char* s) {
	if (!s || !s[0])
		return 1;
	if (s[0] == '/')
		return 1;
	if (strstr(s, ".exe"))
		return 1;
	return 0;
}

static void usage(void) {
	printf("Usage: curl [options...] <url>\n");
	printf("  curl example.com\n");
	printf("  curl http://example.com/\n");
	printf("  curl -v http://example.com\n");
	printf("  curl -I example.com\n");
	printf("  curl -o out.html example.com\n");
	printf("\nOptions:\n");
	printf("  -v        Verbose\n");
	printf("  -I        HEAD request (headers only)\n");
	printf("  -i        Include response headers\n");
	printf("  -o <file> Write body to file\n");
	printf("  -h        This help\n");
	printf("\nOnly http:// (port 80) is supported. No TLS/https.\n");
}

static int parse_url(const char* url, char* host, size_t hostsz, char* path, size_t pathsz, uint16_t* port) {
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
		if (*d == '/')
			slash = d;
		else
			slash = NULL;
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

int main(int argc, char* argv[]) {
	int verbose = 0;
	int head_only = 0;
	int include_hdr = 0;
	const char* outfile = NULL;
	const char* url = NULL;
	int i;

	for (i = 0; i < argc; i++) {
		const char* a = argv[i];
		if (is_prog(a))
			continue;
		if (!strcmp(a, "-h") || !strcmp(a, "--help")) {
			usage();
			return 0;
		}
		if (!strcmp(a, "-v") || !strcmp(a, "--verbose")) {
			verbose = 1;
			continue;
		}
		if (!strcmp(a, "-I") || !strcmp(a, "--head")) {
			head_only = 1;
			include_hdr = 1;
			continue;
		}
		if (!strcmp(a, "-i") || !strcmp(a, "--include")) {
			include_hdr = 1;
			continue;
		}
		if (!strcmp(a, "-o") || !strcmp(a, "--output")) {
			if (i + 1 < argc && !is_prog(argv[i + 1])) {
				i++;
				outfile = argv[i];
			}
			continue;
		}
		if (a[0] == '-') {
			fprintf(stderr, "curl: option %s: is unknown\n", a);
			return 2;
		}
		if (!url)
			url = a;
	}

	if (!url) {
		usage();
		return 2;
	}

	char host[128];
	char path[256];
	uint16_t port = 80;
	memset(host, 0, sizeof(host));
	memset(path, 0, sizeof(path));
	int pu = parse_url(url, host, sizeof(host), path, sizeof(path), &port);
	if (pu == -2) {
		fprintf(stderr, "curl: (1) Protocol \"https\" not supported\n");
		return 1;
	}
	if (pu < 0 || !host[0]) {
		fprintf(stderr, "curl: (3) URL malformed\n");
		return 3;
	}

	hostent* ent = gethostbyname(host);
	if (!ent) {
		fprintf(stderr, "curl: (6) Could not resolve host: %s\n", host);
		return 6;
	}

	uint32_t ipaddr = *(uint32_t*)ent->h_addr_list[0];
	in_addr in;
	in.s_addr = ipaddr;
	char* ipstr = inet_ntoa(in);

	if (verbose)
		fprintf(stderr, "*   Trying %s:%u...\n", ipstr, port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		fprintf(stderr, "curl: (7) Failed to create socket\n");
		return 7;
	}

	sockaddr_in dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port);
	memcpy(&dest.sin_addr, &ipaddr, sizeof(uint32_t));

	if (connect(sock, (sockaddr_*)&dest, sizeof(dest)) < 0) {
		fprintf(stderr, "curl: (7) Failed to connect to %s port %u\n", host, port);
		_KeCloseFile(sock);
		return 7;
	}

	if (verbose) {
		fprintf(stderr, "* Connected to %s (%s) port %u\n", host, ipstr, port);
		fprintf(stderr, "> %s %s HTTP/1.0\n", head_only ? "HEAD" : "GET", path);
		fprintf(stderr, "> Host: %s\n", host);
		fprintf(stderr, "> User-Agent: curl/xeneva\n");
		fprintf(stderr, "> Accept: */*\n");
		fprintf(stderr, "> Connection: close\n");
		fprintf(stderr, ">\n");
	}

	char req[512];
	memset(req, 0, sizeof(req));
	snprintf(req,
			 sizeof(req),
			 "%s %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: curl/xeneva\r\nAccept: */*\r\nConnection: close\r\n\r\n",
			 head_only ? "HEAD" : "GET",
			 path,
			 host);

	if (sendto(sock, req, strlen(req), 0, (sockaddr*)&dest, sizeof(dest)) < 0) {
		fprintf(stderr, "curl: (55) Failed sending data\n");
		_KeCloseFile(sock);
		return 55;
	}

	FILE* out = stdout;
	if (outfile) {
		out = fopen(outfile, "w+");
		if (!out) {
			fprintf(stderr, "curl: (23) Failed writing body\n");
			_KeCloseFile(sock);
			return 23;
		}
	}

	char* buf = (char*)malloc(2048);
	if (!buf) {
		if (outfile)
			fclose(out);
		_KeCloseFile(sock);
		return 1;
	}

	int idle = 0;
	int got_any = 0;
	int in_body = include_hdr ? 1 : 0;
	char tail[4];
	int tlen = 0;
	memset(tail, 0, sizeof(tail));

	while (idle < 80) {
		memset(buf, 0, 2048);
		int n = recvfrom(sock, buf, 2047, 0, NULL, NULL);
		if (n > 0) {
			got_any = 1;
			idle = 0;
			int off = 0;
			if (!in_body) {
				for (int k = 0; k < n; k++) {
					if (tlen < 4)
						tail[tlen++] = buf[k];
					else {
						tail[0] = tail[1];
						tail[1] = tail[2];
						tail[2] = tail[3];
						tail[3] = buf[k];
					}
					if (tlen >= 4 && tail[0] == '\r' && tail[1] == '\n' && tail[2] == '\r' &&
						tail[3] == '\n') {
						in_body = 1;
						off = k + 1;
						break;
					}
				}
				if (!in_body) {
					continue;
				}
			}
			if (head_only)
				break;
			if (off < n)
				fwrite(buf + off, 1, (size_t)(n - off), out);
			fflush(out);
			continue;
		}
		if (n == 0 && got_any)
			break;
		_KeProcessSleep(100);
		idle++;
	}

	if (verbose && got_any)
		fprintf(stderr, "* Closing connection\n");

	if (!got_any)
		fprintf(stderr, "curl: (28) Operation timed out\n");

	free(buf);
	if (outfile)
		fclose(out);
	_KeCloseFile(sock);
	return got_any ? 0 : 28;
}
