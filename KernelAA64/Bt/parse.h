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

#ifndef __BT_PARSE_H__
#define __BT_PARSE_H__

#include <stdint.h>

int bt_ad_name(const uint8_t* ad, int len, char* out, int cap);
int bt_hci_cmd(uint8_t* out, uint16_t opcode, const uint8_t* param, uint8_t plen);
int bt_att_read_req(uint8_t* out, uint16_t handle);
int bt_att_read_rsp(const uint8_t* pdu, int len, uint8_t* val, int cap);

#define BT_BOND_MAGIC 0x31444e42u /* 'BND1' little-endian */

typedef struct _bt_bond_rec_ {
	uint32_t magic;
	uint8_t addr[6];
	uint8_t addr_type;
	uint8_t lesc;
	uint8_t key_size;
	uint8_t irk_ok;
	uint16_t ediv;
	uint8_t rand[8];
	uint8_t ltk[16];
	uint8_t irk[16];
} BtBondRec;

#endif
