/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2024, Manas Kamal Choudhury
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

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t htonl(uint32_t hostlong) {
	return ((((hostlong) & 0xFF) << 24) | (((hostlong) & 0xFF00) << 8) | (((hostlong) & 0xFF0000) >> 8) |
		(((hostlong) & 0xFF000000) >> 24));
}

uint16_t htons(uint16_t hostshort) {
	return ((((hostshort) & 0xFF) << 8) | (((hostshort) & 0xFF00) >> 8));
}

uint32_t ntohl(uint32_t netlong) {
	return htonl(netlong);
}

uint16_t ntohs(uint16_t netshort) {
	return htons(netshort);
}

in_addr_t inet_addr(const char* in) {
	char ip[16];
	char* c = ip;
	uint32_t out[4];
	char* i;
	memcpy(ip,(void*)in, strlen(in) < 15 ? strlen(in) + 1 : 15);
	ip[15] = '\0';

	i = (char*)strchr(c, '.');
	*i = '\0';
	out[0] = atoi(c);
	c += strlen(c) + 1;

	i = (char*)strchr(c, '.');
	*i = '\0';
	out[1] = atoi(c);
	c += strlen(c) + 1;

	i = (char*)strchr(c, '.');
	*i = '\0';
	out[2] = atoi(c);
	c += strlen(c) + 1;

	out[3] = atoi(c);

	return htonl((out[0] << 24) | (out[1] << 16) | (out[2] << 8) | (out[3]));
}
extern char* inet_ntoa(struct in_addr in) {
	static char buf[17];
	uint32_t hostOrder = ntohl(in.s_addr);
	snprintf(buf, 17, "%d.%d.%d.%d", (hostOrder >> 24) & 0xFF,
		(hostOrder >> 16) & 0xFF,
		(hostOrder >> 8) & 0xFF,
		(hostOrder >> 0) & 0xFF);

	return buf;
}