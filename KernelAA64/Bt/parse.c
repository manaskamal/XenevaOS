/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include "parse.h"
#include <string.h>

int bt_ad_name(const uint8_t* ad, int len, char* out, int cap) {
	int i = 0;
	int best = 0;
	if (cap <= 0)
		return 0;
	out[0] = 0;
	while (i < len) {
		int fl = ad[i];
		uint8_t type;
		int n, copy, k;
		if (fl == 0 || i + fl >= len)
			break;
		type = ad[i + 1];
		n = fl - 1;
		if ((type == 0x09 || (type == 0x08 && best == 0)) && n > 0) {
			copy = n;
			if (copy > cap - 1)
				copy = cap - 1;
			for (k = 0; k < copy; k++) {
				uint8_t c = ad[i + 2 + k];
				out[k] = (c >= 32 && c < 127) ? (char)c : '.';
			}
			out[copy] = 0;
			best = type == 0x09 ? 2 : 1;
			if (best == 2)
				return copy;
		}
		i += fl + 1;
	}
	return (int)strlen(out);
}

int bt_hci_cmd(uint8_t* out, uint16_t opcode, const uint8_t* param, uint8_t plen) {
	out[0] = (uint8_t)opcode;
	out[1] = (uint8_t)(opcode >> 8);
	out[2] = plen;
	if (plen && param)
		memcpy(out + 3, param, plen);
	return 3 + plen;
}

int bt_att_read_req(uint8_t* out, uint16_t handle) {
	out[0] = 0x0a;
	out[1] = (uint8_t)handle;
	out[2] = (uint8_t)(handle >> 8);
	return 3;
}

int bt_att_read_rsp(const uint8_t* pdu, int len, uint8_t* val, int cap) {
	int n, i;
	if (len < 1 || pdu[0] != 0x0b)
		return -1;
	n = len - 1;
	if (n > cap)
		n = cap;
	for (i = 0; i < n; i++)
		val[i] = pdu[1 + i];
	return n;
}
