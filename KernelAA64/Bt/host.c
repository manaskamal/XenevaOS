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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <_null.h>
#include <aucon.h>
#include <Bt/bt.h>
#include <Fs/vfs.h>
#include <Fs/Dev/devfs.h>
#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/sched.h>
#include <Hal/AA64/aa64cpu.h>
#include <Sound/sound.h>
#include <Drivers/uart.h>
#include <pcie.h>
#include <Bt/bt.h>
#include "crypto.h"
#include "h4.h"
#include "parse.h"
#include "lc3.h"

extern uint64_t AA64GetPhysicalTimerCount(void);

#define HCI_RESET 0x0C03
#define HCI_SET_EVENT_MASK 0x0C01
#define HCI_WRITE_LE_HOST 0x0C6D
#define HCI_READ_VER 0x1001
#define HCI_READ_BD 0x1009
#define HCI_READ_BUF 0x1005
#define HCI_DISCONNECT 0x0406
#define LE_SET_EVENT_MASK 0x2001
#define LE_READ_BUF 0x2002
#define LE_READ_FEAT 0x2003
#define LE_SET_SCAN_PAR 0x200B
#define LE_SET_SCAN_EN 0x200C
#define LE_CREATE_CONN 0x200D
#define HCI_INQUIRY 0x0401
#define HCI_CREATE_CONN 0x0405
#define HCI_AUTH_REQ 0x0411
#define HCI_SET_ENCRYPT 0x0413
#define HCI_LINK_KEY_REPLY 0x040B
#define HCI_LINK_KEY_NEG 0x040C
#define HCI_IO_CAP_REPLY 0x042B
#define HCI_USER_CONFIRM 0x042C
#define HCI_WRITE_INQ_MODE 0x0C45
#define LE_START_ENC 0x2019
#define LE_LTK_REPLY 0x201A
#define LE_LTK_NEG 0x201B
#define LE_READ_BUF_V2 0x2060
#define LE_SET_CIG 0x2062
#define LE_CREATE_CIS 0x2064
#define LE_SETUP_ISO 0x206E
#define LE_SET_HOST_FEAT 0x2074

#define MAX_BONDS 8

typedef struct {
	uint8_t addr[6];
	uint8_t type;
	uint8_t lesc;
	uint8_t key_size;
	uint8_t irk_ok;
	uint16_t ediv;
	uint8_t randn[8];
	uint8_t ltk[16];
	uint8_t irk[16];
} Bond;

static AuBtUsbOps usb;
static int have_usb;
static BtH4 h4;
static int h4_on;

/*
 * H4 UART transport (Zephyr-style serial bridge).
 *
 * Host runs BlueZ btproxy as a UNIX-socket server:
 *   sudo hciconfig hci0 down
 *   sudo btproxy -u -i 0        # listens on /tmp/bt-server-bredr
 * QEMU maps that socket to its second PL011:
 *   -serial stdio -serial unix:/tmp/bt-server-bredr
 * Guest UART1 (0x09040000, IRQ 8) then speaks raw H4:
 *   0x01 CMD host->ctrl, 0x02 ACL bidir, 0x03 SCO bidir,
 *   0x04 EVT ctrl->host, 0x05 ISO bidir.
 * This replaces the old /dev/vhci + virtio-serial bridge, which fought
 * BlueZ for the adapter and had no QEMU wiring. Probe runs first; if
 * UART1 answers HCI_RESET we use the real controller, else mock.
 */
#ifdef __TARGET_BOARD_QEMU_VIRT__
#define BT_UART1_PHYS 0x09040000u
#else
#define BT_UART1_PHYS 0u
#endif
static volatile uint32_t* bt_uart;
static int have_h4uart;

static uint8_t bd[6];
static uint8_t hci_ver;
static uint16_t acl_mtu = 27;
static uint16_t acl_credits = 1;
static uint16_t acl_max = 1;
static uint16_t iso_mtu;
static uint8_t le_feat[8];
static uint16_t handle;
static int connected;
static int encrypted;
static uint8_t peer[6];
static uint8_t peer_type;
static char peer_name[64];
static BtScanEnt scans[BT_MAX_SCAN];
static int nscan;
static uint8_t scan_psrm[BT_MAX_SCAN];
static uint16_t scan_clk[BT_MAX_SCAN];
static int inq_done;
static int pair_bredr;

static Bond bonds[MAX_BONDS];
static int nbonds;

static uint8_t acl_asm[1024];
static uint16_t acl_got;
static uint16_t acl_need;

static uint8_t cmd_rsp[80];
static uint8_t cmd_rsplen;
static uint16_t wait_op;
static int cmd_done;
static int cmd_status;

static int conn_done;
static int conn_status;
static int enc_done;

static uint8_t att_rsp[180];
static uint8_t att_len;
static int att_done;

static int smp_on;
static int smp_sc;
static uint8_t preq[7];
static uint8_t pres[7];
static uint8_t prnd[16];
static uint8_t rrnd[16];
static uint8_t pcnf[16];
static uint8_t local_pk[64];
static uint8_t local_sk[32];
static uint8_t remote_pk[64];
static uint8_t dhkey[32];
static uint8_t mackey[16];
static uint8_t ltk_new[16];
static uint32_t passkey;
static int need_confirm;
static int user_ok;
static int pair_done;
static int pair_fail;
static int keys_sent;
static uint16_t cis_handle;
static int audio_on;
static int card_id = -1;
static int16_t pcm_acc[LC3_SAMPLES];
static int pcm_n;
static uint16_t iso_seq;
static AuSound ble_card;

static int job;
static BtInfo* job_info;
static BtInfo job_buf;
static volatile int job_done;
static int job_status;
static int bt_started;
static uint8_t snoop[8192];
static uint32_t snoop_len;

static void snoop_open(void);
static void bond_load(void);
static void bond_save(void);
static int hci(uint16_t op, const uint8_t* param, uint8_t plen, uint8_t* out, uint8_t* outlen);
static void hci_send_now(uint16_t op, const uint8_t* param, uint8_t plen);
static void on_event(const uint8_t* ev, uint16_t len);
static void on_acl(const uint8_t* acl, uint16_t len);
static int l2cap_send(uint16_t cid, const uint8_t* data, uint16_t len);
static void poll_usb(void);
static int perform(int code, BtInfo* info);

static void spin_ms(uint32_t ms) {
	uint64_t freq = get_cntfrq_el0();
	uint64_t ticks = (freq / 1000ull) * (uint64_t)ms;
	uint64_t start = get_cntpct_el0();
	if (ticks == 0)
		ticks = 1;
	while ((get_cntpct_el0() - start) < ticks)
		;
}

static void put16(uint8_t* p, uint16_t v) {
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
}

static uint16_t get16(const uint8_t* p) {
	return (uint16_t)(p[0] | (p[1] << 8));
}

static void addr_str(const uint8_t* a, char* o, int n) {
	static const char* hexd = "0123456789ABCDEF";
	int i, p = 0;
	for (i = 5; i >= 0 && p + 3 < n; i--) {
		o[p++] = hexd[a[i] >> 4];
		o[p++] = hexd[a[i] & 0xf];
		if (i)
			o[p++] = ':';
	}
	o[p] = 0;
}

static void snoop_add(int from_ctrl, const uint8_t* p, uint16_t len) {
	uint8_t rec[16];
	uint32_t i;
	if (snoop_len == 0) {
		memcpy(snoop, "btsnoop", 7);
		snoop[7] = 0;
		snoop[8] = 0;
		snoop[9] = 0;
		snoop[10] = 0;
		snoop[11] = 1;
		snoop[12] = 0;
		snoop[13] = 0;
		snoop[14] = 0x03;
		snoop[15] = 0xea;
		snoop_len = 16;
	}
	if ((uint32_t)len + 16 + snoop_len > sizeof snoop)
		return;
	rec[0] = 0;
	rec[1] = 0;
	rec[2] = (uint8_t)(len >> 8);
	rec[3] = (uint8_t)len;
	rec[4] = rec[0];
	rec[5] = rec[1];
	rec[6] = rec[2];
	rec[7] = rec[3];
	rec[8] = 0;
	rec[9] = 0;
	rec[10] = 0;
	rec[11] = from_ctrl ? 1 : 0;
	memset(rec + 12, 0, 4);
	memcpy(snoop + snoop_len, rec, 16);
	snoop_len += 16;
	for (i = 0; i < len; i++)
		snoop[snoop_len++] = p[i];
}

static void snoop_flush(void) {
	AuVFSNode* root;
	AuVFSNode* file;
	if (snoop_len < 16)
		return;
	root = AuVFSFind("/");
	if (!root)
		return;
	AuVFSCreateDir(root, "/bt");
	file = AuVFSOpen("/bt/btsnoop.log");
	if (!file)
		file = AuVFSCreateFile(root, "/bt/btsnoop.log");
	if (!file)
		return;
	file->current = file->first_block;
	file->eof = 0;
	file->pos = 0;
	AuVFSNodeWrite(root, file, (uint64_t*)snoop, snoop_len);
}

static void bond_load(void) {
	AuVFSNode* root = AuVFSFind("/");
	AuVFSNode* file;
	uint8_t buf[sizeof(BtBondRec) * MAX_BONDS];
	size_t n;
	int i;
	nbonds = 0;
	if (!root)
		return;
	file = AuVFSOpen("/bt/bonds.bin");
	if (!file)
		return;
	memset(buf, 0, sizeof buf);
	n = AuVFSNodeRead(root, file, (uint64_t*)buf, sizeof buf);
	if (n > sizeof buf)
		n = sizeof buf;
	for (i = 0; i + (int)sizeof(BtBondRec) <= (int)n && nbonds < MAX_BONDS; i += (int)sizeof(BtBondRec)) {
		BtBondRec* r = (BtBondRec*)(buf + i);
		if (r->magic != BT_BOND_MAGIC)
			break;
		memcpy(bonds[nbonds].addr, r->addr, 6);
		bonds[nbonds].type = r->addr_type;
		bonds[nbonds].lesc = r->lesc;
		bonds[nbonds].key_size = r->key_size;
		bonds[nbonds].irk_ok = r->irk_ok;
		bonds[nbonds].ediv = r->ediv;
		memcpy(bonds[nbonds].randn, r->rand, 8);
		memcpy(bonds[nbonds].ltk, r->ltk, 16);
		memcpy(bonds[nbonds].irk, r->irk, 16);
		nbonds++;
	}
}

static void bond_save(void) {
	AuVFSNode* root = AuVFSFind("/");
	AuVFSNode* file;
	uint8_t buf[sizeof(BtBondRec) * MAX_BONDS];
	int i;
	if (!root)
		return;
	AuVFSCreateDir(root, "/bt");
	file = AuVFSOpen("/bt/bonds.bin");
	if (!file)
		file = AuVFSCreateFile(root, "/bt/bonds.bin");
	if (!file)
		return;
	memset(buf, 0, sizeof buf);
	for (i = 0; i < nbonds; i++) {
		BtBondRec* r = (BtBondRec*)(buf + i * sizeof(BtBondRec));
		r->magic = BT_BOND_MAGIC;
		memcpy(r->addr, bonds[i].addr, 6);
		r->addr_type = bonds[i].type;
		r->lesc = bonds[i].lesc;
		r->key_size = bonds[i].key_size ? bonds[i].key_size : 16;
		r->irk_ok = bonds[i].irk_ok;
		r->ediv = bonds[i].ediv;
		memcpy(r->rand, bonds[i].randn, 8);
		memcpy(r->ltk, bonds[i].ltk, 16);
		memcpy(r->irk, bonds[i].irk, 16);
	}
	file->current = file->first_block;
	file->eof = 0;
	file->pos = 0;
	AuVFSNodeWrite(root, file, (uint64_t*)buf, (uint32_t)(nbonds * sizeof(BtBondRec)));
}

static int bond_find(const uint8_t* addr, uint8_t type) {
	int i;
	for (i = 0; i < nbonds; i++) {
		if (bonds[i].type == type && memcmp(bonds[i].addr, addr, 6) == 0)
			return i;
	}
	return -1;
}

static int bond_resolve(const uint8_t* addr) {
	int i;
	uint8_t hash[3];
	if ((addr[5] & 0xc0) != 0x40)
		return -1;
	for (i = 0; i < nbonds; i++) {
		if (!bonds[i].irk_ok)
			continue;
		bt_ah(bonds[i].irk, addr + 3, hash);
		if (memcmp(hash, addr, 3) == 0)
			return i;
	}
	return -1;
}

static void fill_rand(uint8_t* p, int n) {
	int i;
	for (i = 0; i < n; i++)
		p[i] = (uint8_t)(rand() ^ (AA64GetPhysicalTimerCount() >> (i & 7)));
}

/* ---- PL011 UART1 helpers (polled, console UART0 untouched) ---- */
static void bt_uart_init_hw(void) {
#ifdef __TARGET_BOARD_QEMU_VIRT__
	if (BT_UART1_PHYS == 0 || bt_uart)
		return;
	bt_uart = (volatile uint32_t*)AuMapMMIO(BT_UART1_PHYS, 1);
	if (!bt_uart)
		return;
	bt_uart[UART_CR / 4] = 0;
	bt_uart[UART_IBRD / 4] = 13;
	bt_uart[UART_FBRD / 4] = 1;
	bt_uart[UART_LCR_H / 4] = (uint32_t)(UART_LCR_H_WLEN_8BIT | UART_LCR_H_FEN);
	bt_uart[UART_CR / 4] = (uint32_t)(UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
	bt_uart[UART_ICR / 4] = 0xffff;
#endif
}

static void bt_uart_tx_bytes(const uint8_t* d, uint16_t n) {
	uint16_t i;
	if (!bt_uart)
		return;
	for (i = 0; i < n; i++) {
		while (bt_uart[UART_FR / 4] & UART_FR_TXFF)
			;
		bt_uart[UART_DR / 4] = d[i];
	}
}

static void bt_uart_tx_h4(uint8_t kind, const uint8_t* payload, uint16_t len) {
	uint8_t wrapped[600];
	int n = bt_h4_wrap(kind, payload, len, wrapped, sizeof wrapped);
	if (n > 0)
		bt_uart_tx_bytes(wrapped, (uint16_t)n);
}

static void poll_h4uart(void) {
	uint8_t kind = 0;
	uint8_t out[512];
	uint16_t olen = 0;
	if (!have_h4uart || !bt_uart)
		return;
	/* Drain PL011 RX FIFO; each byte feeds the H4 reassembler. */
	for (;;) {
		uint8_t b;
		if (bt_uart[UART_FR / 4] & UART_FR_RXFE)
			break;
		b = (uint8_t)(bt_uart[UART_DR / 4] & 0xff);
		if (!bt_h4_feed(&h4, &b, 1, &kind, out, &olen, sizeof out))
			continue;
		if (kind == H4_EVT) {
			snoop_add(1, out, olen);
			on_event(out, olen);
		} else if (kind == H4_ACL) {
			snoop_add(1, out, olen);
			on_acl(out, olen);
		}
		/* H4_SCO / H4_ISO RX not used by this host; consume silently. */
		kind = 0;
		olen = 0;
	}
}

static int transport_cmd(const uint8_t* bytes, uint16_t len) {
	snoop_add(0, bytes, len);
	if (have_usb)
		return usb.control(usb.ctx, 0x20, 0, 0, 0, (uint8_t*)bytes, len, 0);
	if (have_h4uart) {
		bt_uart_tx_h4(H4_CMD, bytes, len);
		return 0;
	}
	return -1;
}

static int transport_acl(const uint8_t* bytes, uint16_t len) {
	snoop_add(0, bytes, len);
	if (have_usb)
		return usb.bulk_out(usb.ctx, bytes, len);
	if (have_h4uart) {
		bt_uart_tx_h4(H4_ACL, bytes, len);
		return 0;
	}
	return -1;
}

static int transport_iso(const uint8_t* bytes, uint16_t len) {
	snoop_add(0, bytes, len);
	if (have_usb) {
		if (usb.iso_out)
			return usb.iso_out(usb.ctx, bytes, len);
		return -1;
	}
	if (have_h4uart) {
		bt_uart_tx_h4(H4_ISO, bytes, len);
		return 0;
	}
	return -1;
}

static void poll_usb(void) {
	uint8_t buf[512];
	int n;
	if (have_h4uart)
		poll_h4uart();
	if (!have_usb)
		return;
	n = usb.intr(usb.ctx, buf, sizeof buf);
	if (n > 0) {
		snoop_add(1, buf, (uint16_t)n);
		on_event(buf, (uint16_t)n);
	}
	n = usb.bulk_in(usb.ctx, buf, sizeof buf);
	if (n > 0) {
		snoop_add(1, buf, (uint16_t)n);
		on_acl(buf, (uint16_t)n);
	}
}

static int wait_cmd_done(void) {
	int spins = 0;
	while (!cmd_done && spins++ < 300) {
		poll_usb();
		spin_ms(2);
	}
	return cmd_done ? 0 : -1;
}

/* Reply from inside an event handler. hci() would replace the command
 * the caller is still waiting on. */
static void hci_send_now(uint16_t op, const uint8_t* param, uint8_t plen) {
	uint8_t cmd[80];
	int n = bt_hci_cmd(cmd, op, param, plen);
	if (n > 0)
		transport_cmd(cmd, (uint16_t)n);
}

static int scan_find(const uint8_t* addr) {
	int i;
	for (i = 0; i < nscan; i++) {
		if (!memcmp(scans[i].addr, addr, 6))
			return i;
	}
	return -1;
}

static void scan_add(const uint8_t* addr, uint8_t type, int8_t rssi, const char* name,
					 uint8_t psrm, uint16_t clk) {
	int idx = scan_find(addr);
	BtScanEnt* s;
	if (idx < 0) {
		if (nscan >= BT_MAX_SCAN)
			return;
		idx = nscan++;
		memset(&scans[idx], 0, sizeof scans[idx]);
		memcpy(scans[idx].addr, addr, 6);
	}
	s = &scans[idx];
	/* A classic result for an address we already saw as LE replaces it.
	 * The -LE advert is the assistant side, not the audio device. */
	if (type == BT_ADDR_BREDR || s->addr_type != BT_ADDR_BREDR)
		s->addr_type = type;
	s->rssi = rssi;
	if (name && name[0]) {
		int n = 0;
		while (name[n] && n < BT_NAME_LEN - 1) {
			s->name[n] = name[n];
			n++;
		}
		s->name[n] = 0;
	}
	scan_psrm[idx] = psrm;
	scan_clk[idx] = clk;
}

static int hci(uint16_t op, const uint8_t* param, uint8_t plen, uint8_t* out, uint8_t* outlen) {
	uint8_t cmd[260];
	int n = bt_hci_cmd(cmd, op, param, plen);
	cmd_done = 0;
	cmd_status = -1;
	cmd_rsplen = 0;
	wait_op = op;
	if (transport_cmd(cmd, (uint16_t)n))
		return -1;
	if (wait_cmd_done())
		return -1;
	if (out && outlen) {
		uint8_t c = cmd_rsplen;
		if (c > *outlen)
			c = *outlen;
		memcpy(out, cmd_rsp, c);
		*outlen = c;
	}
	return cmd_status;
}

static int acl_send(uint16_t h, uint8_t pb, const uint8_t* data, uint16_t len) {
	uint8_t pkt[600];
	uint16_t hf;
	int spins = 0;
	while (acl_credits == 0 && spins++ < 100) {
		poll_usb();
		spin_ms(2);
	}
	if (acl_credits == 0)
		return -1;
	if ((uint32_t)len + 4 > sizeof pkt)
		return -1;
	hf = (uint16_t)(h | ((uint16_t)pb << 12));
	put16(pkt, hf);
	put16(pkt + 2, len);
	memcpy(pkt + 4, data, len);
	if (transport_acl(pkt, (uint16_t)(len + 4)))
		return -1;
	acl_credits--;
	return 0;
}

static int l2cap_send(uint16_t cid, const uint8_t* data, uint16_t len) {
	uint8_t pkt[520];
	uint16_t room = acl_mtu ? acl_mtu : 27;
	uint16_t first;
	uint16_t off;
	if ((uint32_t)len + 4 > sizeof pkt)
		return -1;
	put16(pkt, len);
	put16(pkt + 2, cid);
	memcpy(pkt + 4, data, len);
	first = (uint16_t)(len + 4);
	if (first > room)
		first = room;
	if (acl_send(handle, 0x00, pkt, first))
		return -1;
	off = first;
	while (off < len + 4) {
		uint16_t chunk = (uint16_t)(len + 4 - off);
		if (chunk > room)
			chunk = room;
		if (acl_send(handle, 0x01, pkt + off, chunk))
			return -1;
		off = (uint16_t)(off + chunk);
	}
	return 0;
}

static void remember_bond(int lesc) {
	int idx = bond_find(peer, peer_type);
	Bond* b;
	if (idx < 0) {
		if (nbonds >= MAX_BONDS)
			idx = 0;
		else
			idx = nbonds++;
	}
	b = &bonds[idx];
	memset(b, 0, sizeof *b);
	memcpy(b->addr, peer, 6);
	b->type = peer_type;
	b->lesc = (uint8_t)lesc;
	b->key_size = 16;
	memcpy(b->ltk, ltk_new, 16);
	if (!lesc) {
		b->ediv = (uint16_t)(rand() & 0xffff);
		fill_rand(b->randn, 8);
	}
	fill_rand(b->irk, 16);
	b->irk_ok = 1;
	bond_save();
}

static void smp_send(const uint8_t* p, uint16_t len) {
	l2cap_send(0x0006, p, len);
}

static void smp_send_keys(void) {
	uint8_t p[17];
	if (keys_sent)
		return;
	keys_sent = 1;
	p[0] = 0x06;
	memcpy(p + 1, ltk_new, 16);
	smp_send(p, 17);
	if (!smp_sc) {
		uint8_t id[11];
		int idx = bond_find(peer, peer_type);
		id[0] = 0x07;
		if (idx >= 0) {
			put16(id + 1, bonds[idx].ediv);
			memcpy(id + 3, bonds[idx].randn, 8);
		} else {
			memset(id + 1, 0, 10);
		}
		smp_send(id, 11);
	}
	p[0] = 0x08;
	{
		int idx = bond_find(peer, peer_type);
		if (idx >= 0)
			memcpy(p + 1, bonds[idx].irk, 16);
		else
			memset(p + 1, 0, 16);
	}
	smp_send(p, 17);
	p[0] = 0x09;
	p[1] = 0;
	memcpy(p + 2, bd, 6);
	smp_send(p, 8);
	pair_done = 1;
}

static void start_enc(const uint8_t* randn, uint16_t ediv, const uint8_t* ltk) {
	uint8_t p[28];
	put16(p, handle);
	memcpy(p + 2, randn, 8);
	put16(p + 10, ediv);
	memcpy(p + 12, ltk, 16);
	enc_done = 0;
	hci(LE_START_ENC, p, 28, NULL, NULL);
}

static void lesc_after_random(void) {
	uint8_t cfm[16];
	uint8_t a1[7], a2[7];
	uint8_t sk[32], rx[32], ry[32], dh[32];
	bt_f4(remote_pk, local_pk, rrnd, 0, cfm);
	if (memcmp(cfm, pcnf, 16) != 0) {
		pair_fail = 1;
		return;
	}
	bt_rev(local_sk, sk, 32);
	bt_rev(remote_pk, rx, 32);
	bt_rev(remote_pk + 32, ry, 32);
	if (bt_p256_dh(sk, rx, ry, dh)) {
		pair_fail = 1;
		return;
	}
	bt_rev(dh, dhkey, 32);
	memcpy(a1, peer, 6);
	a1[6] = peer_type;
	memcpy(a2, bd, 6);
	a2[6] = 0;
	/* Central is A. N1 is our random, N2 is the peripheral random. */
	bt_f5(dhkey, prnd, rrnd, a2, a1, mackey, ltk_new);
	passkey = bt_g2(local_pk, remote_pk, prnd, rrnd);
	need_confirm = 1;
	UARTDebugOut("[aurora]: bt passkey %d\r\n", passkey);
}

static void lesc_dhcheck(void) {
	uint8_t e[17];
	uint8_t r[16];
	uint8_t iocap[3];
	uint8_t a1[7], a2[7];
	memset(r, 0, 16);
	memcpy(iocap, preq + 1, 3);
	memcpy(a2, bd, 6);
	a2[6] = 0;
	memcpy(a1, peer, 6);
	a1[6] = peer_type;
	e[0] = 0x0d;
	bt_f6(mackey, prnd, rrnd, r, iocap, a2, a1, e + 1);
	smp_send(e, 17);
}

static void on_smp(const uint8_t* p, uint16_t len) {
	if (len < 1)
		return;
	switch (p[0]) {
	case 0x02:
		if (len < 7)
			return;
		memcpy(pres, p, 7);
		smp_sc = (p[3] & 0x08) != 0;
		if (smp_sc) {
			uint8_t pkt[65];
			uint8_t sk[32], x[32], y[32];
			fill_rand(local_sk, 32);
			bt_rev(local_sk, sk, 32);
			if (bt_p256_mul_g(sk, x, y))
				return;
			bt_rev(x, local_pk, 32);
			bt_rev(y, local_pk + 32, 32);
			pkt[0] = 0x0c;
			memcpy(pkt + 1, local_pk, 64);
			smp_send(pkt, 65);
		} else {
			uint8_t cfm[17];
			uint8_t zero[16];
			fill_rand(prnd, 16);
			memset(zero, 0, 16);
			cfm[0] = 0x03;
			bt_c1(zero, prnd, preq, pres, 0, bd, peer_type, peer, cfm + 1);
			smp_send(cfm, 17);
		}
		break;
	case 0x03:
		if (len < 17)
			return;
		memcpy(pcnf, p + 1, 16);
		if (!smp_sc) {
			uint8_t rnd[17];
			rnd[0] = 0x04;
			memcpy(rnd + 1, prnd, 16);
			smp_send(rnd, 17);
		}
		break;
	case 0x04:
		if (len < 17)
			return;
		memcpy(rrnd, p + 1, 16);
		if (smp_sc) {
			lesc_after_random();
		} else {
			uint8_t cfm[16];
			uint8_t stk[16];
			uint8_t zero[16];
			memset(zero, 0, 16);
			bt_c1(zero, rrnd, preq, pres, 0, bd, peer_type, peer, cfm);
			if (memcmp(cfm, pcnf, 16) != 0) {
				pair_fail = 1;
				return;
			}
			bt_s1(zero, rrnd, prnd, stk);
			memcpy(ltk_new, stk, 16);
			start_enc(zero, 0, stk);
		}
		break;
	case 0x0c:
		if (len < 65)
			return;
		memcpy(remote_pk, p + 1, 64);
		{
			uint8_t cfm[17];
			fill_rand(prnd, 16);
			cfm[0] = 0x03;
			bt_f4(local_pk, remote_pk, prnd, 0, cfm + 1);
			smp_send(cfm, 17);
		}
		break;
	case 0x0d:
		if (user_ok || !need_confirm) {
			uint8_t zero[8];
			memset(zero, 0, 8);
			remember_bond(1);
			start_enc(zero, 0, ltk_new);
		}
		break;
	case 0x06:
		if (len >= 17)
			memcpy(ltk_new, p + 1, 16);
		break;
	case 0x07:
		if (len >= 11) {
			int idx;
			remember_bond(0);
			idx = bond_find(peer, peer_type);
			if (idx >= 0) {
				bonds[idx].ediv = get16(p + 1);
				memcpy(bonds[idx].randn, p + 3, 8);
				memcpy(bonds[idx].ltk, ltk_new, 16);
				bond_save();
			}
		}
		break;
	case 0x08:
		if (len >= 17) {
			int idx = bond_find(peer, peer_type);
			if (idx >= 0) {
				memcpy(bonds[idx].irk, p + 1, 16);
				bonds[idx].irk_ok = 1;
				bond_save();
			}
		}
		break;
	case 0x05:
		pair_fail = 1;
		break;
	default:
		break;
	}
}

static void on_acl(const uint8_t* acl, uint16_t len) {
	uint16_t hf, dlen, pb, h;
	const uint8_t* data;
	if (len < 4)
		return;
	hf = get16(acl);
	dlen = get16(acl + 2);
	h = hf & 0x0fff;
	pb = (hf >> 12) & 3;
	if (h != handle)
		return;
	if (dlen > len - 4)
		dlen = (uint16_t)(len - 4);
	data = acl + 4;
	if (pb != 1) {
		acl_got = 0;
		acl_need = 0;
	}
	if (acl_got + dlen > sizeof acl_asm)
		return;
	memcpy(acl_asm + acl_got, data, dlen);
	acl_got = (uint16_t)(acl_got + dlen);
	if (acl_need == 0 && acl_got >= 4)
		acl_need = (uint16_t)(get16(acl_asm) + 4);
	if (acl_need && acl_got >= acl_need) {
		uint16_t cid = get16(acl_asm + 2);
		uint16_t plen = get16(acl_asm);
		const uint8_t* pdu = acl_asm + 4;
		if (cid == 0x0004) {
			att_len = plen > sizeof att_rsp ? sizeof att_rsp : (uint8_t)plen;
			memcpy(att_rsp, pdu, att_len);
			att_done = 1;
			if (plen >= 1 && pdu[0] == 0x0b && plen > 1) {
				int c = plen - 1;
				if (c > 63)
					c = 63;
				memcpy(peer_name, pdu + 1, (size_t)c);
				peer_name[c] = 0;
			}
		} else if (cid == 0x0006) {
			on_smp(pdu, plen);
		}
		acl_got = 0;
		acl_need = 0;
	}
}

static void on_event(const uint8_t* ev, uint16_t len) {
	uint16_t elen;
	if (len < 2)
		return;
	/* USB delivers a full max-packet buffer. The HCI event is only
	 * param-length + 2 bytes; the rest is padding. */
	elen = (uint16_t)(ev[1] + 2);
	if (elen < len)
		len = elen;
	if (ev[0] == 0x0e && len >= 6) {
		uint16_t op = get16(ev + 3);
		if (op == wait_op) {
			cmd_status = ev[5];
			cmd_rsplen = (uint8_t)(ev[1] >= 3 ? ev[1] - 3 : 0);
			if (cmd_rsplen > sizeof cmd_rsp)
				cmd_rsplen = sizeof cmd_rsp;
			if (len >= (uint16_t)(6 + cmd_rsplen))
				memcpy(cmd_rsp, ev + 5, cmd_rsplen);
			else
				memcpy(cmd_rsp, ev + 5, len - 5);
			cmd_done = 1;
		}
	} else if (ev[0] == 0x0f && len >= 6) {
		uint16_t op = get16(ev + 4);
		if (op == wait_op) {
			cmd_status = ev[2];
			cmd_rsplen = 1;
			cmd_rsp[0] = ev[2];
			cmd_done = 1;
		}
	} else if (ev[0] == 0x13 && len >= 5) {
		int n = ev[2];
		int i;
		for (i = 0; i < n && 3 + i * 4 + 3 < len; i++) {
			uint16_t c = get16(ev + 5 + i * 4);
			acl_credits = (uint16_t)(acl_credits + c);
			if (acl_credits > acl_max)
				acl_credits = acl_max;
		}
	} else if (ev[0] == 0x01 && len >= 3) {
		inq_done = 1;
	} else if (ev[0] == 0x03 && len >= 13) {
		conn_status = ev[2];
		handle = get16(ev + 3);
		memcpy(peer, ev + 5, 6);
		peer_type = BT_ADDR_BREDR;
		connected = conn_status == 0;
		conn_done = 1;
	} else if (ev[0] == 0x05) {
		connected = 0;
		encrypted = 0;
		handle = 0;
	} else if (ev[0] == 0x08 && len >= 4) {
		encrypted = (len >= 6) ? (ev[5] != 0) : (ev[3] != 0);
		enc_done = 1;
		if (encrypted && smp_on)
			smp_send_keys();
	} else if (ev[0] == 0x17 && len >= 8) {
		uint8_t rep[22];
		int idx;
		memcpy(rep, ev + 2, 6);
		idx = -1;
		{
			int i;
			for (i = 0; i < nbonds; i++) {
				if (bonds[i].type == BT_ADDR_BREDR && !memcmp(bonds[i].addr, ev + 2, 6)) {
					idx = i;
					break;
				}
			}
		}
		if (idx >= 0) {
			memcpy(rep + 6, bonds[idx].ltk, 16);
			hci_send_now(HCI_LINK_KEY_REPLY, rep, 22);
		} else {
			hci_send_now(HCI_LINK_KEY_NEG, rep, 6);
		}
	} else if (ev[0] == 0x18 && len >= 24) {
		memcpy(peer, ev + 2, 6);
		peer_type = BT_ADDR_BREDR;
		memcpy(ltk_new, ev + 8, 16);
		remember_bond(0);
	} else if (ev[0] == 0x2f && len >= 17) {
		char name[BT_NAME_LEN];
		const uint8_t* eir = ev + 17;
		int elen = (int)len - 17;
		name[0] = 0;
		if (elen > 240)
			elen = 240;
		bt_ad_name(eir, elen, name, BT_NAME_LEN);
		scan_add(ev + 3, BT_ADDR_BREDR, (int8_t)ev[16], name, ev[9], get16(ev + 14));
	} else if (ev[0] == 0x31 && len >= 8) {
		uint8_t rep[9];
		memcpy(rep, ev + 2, 6);
		rep[6] = 0x03; /* NoInputNoOutput */
		rep[7] = 0x00;
		rep[8] = 0x01; /* bonding, no MITM */
		hci_send_now(HCI_IO_CAP_REPLY, rep, 9);
	} else if (ev[0] == 0x33 && len >= 12) {
		uint8_t rep[6];
		passkey = (uint32_t)ev[8] | ((uint32_t)ev[9] << 8) | ((uint32_t)ev[10] << 16) |
				  ((uint32_t)ev[11] << 24);
		memcpy(rep, ev + 2, 6);
		hci_send_now(HCI_USER_CONFIRM, rep, 6);
		need_confirm = 0;
		user_ok = 1;
	} else if (ev[0] == 0x36 && len >= 9) {
		pair_fail = ev[2] != 0;
		pair_done = ev[2] == 0;
		if (pair_done && pair_bredr) {
			uint8_t enc[3];
			put16(enc, handle);
			enc[2] = 1;
			hci_send_now(HCI_SET_ENCRYPT, enc, 3);
		}
	} else if (ev[0] == 0x3e && len >= 3) {
		uint8_t sub = ev[2];
		if (sub == 0x02 && len >= 12) {
			const uint8_t* r = ev + 3;
			uint8_t num = r[0];
			int off = 1;
			int i;
			(void)num;
			for (i = 0; i < num && off + 8 < (int)len - 3; i++) {
				uint8_t elen;
				{
					char nm[BT_NAME_LEN];
					int8_t rssi = 0;
					nm[0] = 0;
					elen = r[off + 8];
					if (off + 9 + elen < (int)len - 3) {
						bt_ad_name(r + off + 9, elen, nm, BT_NAME_LEN);
						rssi = (int8_t)r[off + 9 + elen];
					}
					scan_add(r + off + 2, r[off + 1], rssi, nm, 0, 0);
				}
				off += 10 + elen;
			}
		} else if ((sub == 0x01 || sub == 0x0a) && len >= 14) {
			conn_status = ev[3];
			handle = get16(ev + 4);
			peer_type = ev[6];
			memcpy(peer, ev + 7, 6);
			connected = conn_status == 0;
			conn_done = 1;
		} else if (sub == 0x05 && len >= 13) {
			uint8_t rep[18];
			int idx = bond_find(peer, peer_type);
			put16(rep, handle);
			if (idx >= 0) {
				memcpy(rep + 2, bonds[idx].ltk, 16);
				hci(LE_LTK_REPLY, rep, 18, NULL, NULL);
			} else {
				hci(LE_LTK_NEG, rep, 2, NULL, NULL);
			}
		} else if (sub == 0x19) {
			if (len >= 6 && ev[3] == 0)
				cis_handle = get16(ev + 4);
		}
	}
}

static int att_req(const uint8_t* req, uint16_t len) {
	int spins = 0;
	att_done = 0;
	if (l2cap_send(0x0004, req, len))
		return -1;
	while (!att_done && spins++ < 200) {
		poll_usb();
		spin_ms(2);
	}
	return att_done ? 0 : -1;
}

static int read_peer_name(void) {
	uint8_t req[7];
	req[0] = 0x08;
	put16(req + 1, 0x0001);
	put16(req + 3, 0xffff);
	put16(req + 5, 0x2a00);
	if (att_req(req, 7))
		return -1;
	if (att_len >= 5 && att_rsp[0] == 0x09) {
		uint8_t rlen = att_rsp[1];
		uint16_t vh;
		uint8_t rd[3];
		if (rlen >= 2 && att_len >= 4) {
			vh = get16(att_rsp + 2);
			if (rlen > 2 && att_len > 4) {
				int c = rlen - 2;
				if (c > 63)
					c = 63;
				memcpy(peer_name, att_rsp + 4, (size_t)c);
				peer_name[c] = 0;
				return 0;
			}
			rd[0] = 0x0a;
			put16(rd + 1, vh);
			if (!att_req(rd, 3) && att_len > 1 && att_rsp[0] == 0x0b) {
				int c = att_len - 1;
				if (c > 63)
					c = 63;
				memcpy(peer_name, att_rsp + 1, (size_t)c);
				peer_name[c] = 0;
				return 0;
			}
		}
	}
	return -1;
}

static void copy_info(BtInfo* info) {
	if (!info)
		return;
	memset(info->msg, 0, sizeof info->msg);
	memcpy(info->bd_addr, bd, 6);
	info->hci_ver = hci_ver;
	info->acl_mtu = acl_mtu;
	info->iso_mtu = iso_mtu;
	info->iso = iso_mtu ? 1 : 0;
	info->connected = (uint8_t)connected;
	info->encrypted = (uint8_t)encrypted;
	memcpy(info->peer, peer, 6);
	info->peer_type = peer_type;
	memcpy(info->peer_name, peer_name, sizeof info->peer_name);
	if (nscan < 0)
		nscan = 0;
	if (nscan > BT_MAX_SCAN)
		nscan = BT_MAX_SCAN;
	info->nscan = nscan;
	if (nscan > 0)
		memcpy(info->scan, scans, (size_t)nscan * sizeof(BtScanEnt));
	info->passkey = passkey;
	info->card_id = card_id;
	if (!have_usb && !have_h4uart)
		memcpy(info->msg, "no adapter", 11);
}

static int do_scan(BtInfo* info) {
	uint8_t par[7];
	uint8_t en[2];
	int spins = 0;
	nscan = 0;
	par[0] = 0x01;
	put16(par + 1, 0x0060);
	put16(par + 3, 0x0030);
	par[5] = 0;
	par[6] = 0;
	if (hci(LE_SET_SCAN_PAR, par, 7, NULL, NULL)) {
		copy_info(info);
		if (info)
			memcpy(info->msg, "scan failed", 12);
		return BT_ERR;
	}
	en[0] = 1;
	en[1] = 0;
	if (hci(LE_SET_SCAN_EN, en, 2, NULL, NULL)) {
		copy_info(info);
		if (info)
			memcpy(info->msg, "scan failed", 12);
		return BT_ERR;
	}
	while (spins++ < 150) {
		poll_usb();
		AuSleepThread(AuGetCurrentThread(), 10);
		AuScheduleNext();
	}
	en[0] = 0;
	hci(LE_SET_SCAN_EN, en, 2, NULL, NULL);
	/* Classic inquiry as well. The JBL audio device is "JBL TUNE FLEX".
	 * The advert named "...-LE" is only the assistant side. */
	{
		uint8_t mode = 2;
		uint8_t inq[5];
		int ispins = 0;
		hci(HCI_WRITE_INQ_MODE, &mode, 1, NULL, NULL);
		inq[0] = 0x33;
		inq[1] = 0x8b;
		inq[2] = 0x9e;
		inq[3] = 0x04;
		inq[4] = 0;
		inq_done = 0;
		if (!hci(HCI_INQUIRY, inq, 5, NULL, NULL)) {
			while (!inq_done && ispins++ < 600) {
				poll_usb();
				spin_ms(10);
			}
		}
	}
	copy_info(info);
	return BT_OK;
}

static int do_connect_bredr(BtInfo* info) {
	uint8_t p[13];
	int idx = scan_find(info->peer);
	int spins = 0;
	uint8_t psrm = 1;
	uint16_t clk = 0;
	if (idx >= 0) {
		psrm = scan_psrm[idx];
		clk = scan_clk[idx];
	}
	memcpy(p, info->peer, 6);
	put16(p + 6, 0xcc18);
	p[8] = psrm ? psrm : 1;
	p[9] = 0;
	put16(p + 10, clk);
	p[12] = 1;
	conn_done = 0;
	connected = 0;
	if (hci(HCI_CREATE_CONN, p, 13, NULL, NULL))
		return BT_ERR;
	while (!conn_done && spins++ < 1200) {
		poll_usb();
		spin_ms(5);
	}
	if (!connected)
		return BT_ERR;
	peer_type = BT_ADDR_BREDR;
	copy_info(info);
	return BT_OK;
}

static int do_connect(BtInfo* info) {
	uint8_t p[25];
	int idx;
	int spins = 0;
	if (!info)
		return BT_ERR;
	if (info->peer_type == BT_ADDR_BREDR)
		return do_connect_bredr(info);
	memset(p, 0, sizeof p);
	put16(p, 0x0060);
	put16(p + 2, 0x0030);
	p[4] = 0;
	p[5] = info->peer_type;
	memcpy(p + 6, info->peer, 6);
	p[12] = 0;
	put16(p + 13, 0x0018);
	put16(p + 15, 0x0028);
	put16(p + 17, 0);
	put16(p + 19, 0x01f4);
	put16(p + 21, 0);
	put16(p + 23, 0);
	conn_done = 0;
	if (hci(LE_CREATE_CONN, p, 25, NULL, NULL))
		return BT_ERR;
	while (!conn_done && spins++ < 400) {
		poll_usb();
		spin_ms(5);
	}
	if (!connected)
		return BT_ERR;
	idx = bond_find(peer, peer_type);
	if (idx < 0)
		idx = bond_resolve(peer);
	if (idx >= 0) {
		uint8_t z[8];
		memset(z, 0, 8);
		if (bonds[idx].lesc)
			start_enc(z, 0, bonds[idx].ltk);
		else
			start_enc(bonds[idx].randn, bonds[idx].ediv, bonds[idx].ltk);
		spins = 0;
		while (!enc_done && spins++ < 200) {
			poll_usb();
			spin_ms(5);
		}
	}
	copy_info(info);
	return connected ? BT_OK : BT_ERR;
}

static int do_pair_bredr(BtInfo* info) {
	uint8_t p[2];
	int spins = 0;
	if (!connected || !handle)
		return BT_ERR;
	pair_bredr = 1;
	smp_on = 0;
	pair_done = 0;
	pair_fail = 0;
	encrypted = 0;
	enc_done = 0;
	put16(p, handle);
	if (hci(HCI_AUTH_REQ, p, 2, NULL, NULL)) {
		pair_bredr = 0;
		return BT_ERR;
	}
	while (!pair_fail && !encrypted && spins++ < 500) {
		poll_usb();
		spin_ms(5);
	}
	pair_bredr = 0;
	copy_info(info);
	if (pair_fail)
		return BT_ERR;
	return encrypted ? BT_OK : BT_ERR;
}

static int do_pair(BtInfo* info) {
	uint8_t req[7];
	int spins = 0;
	if (!connected)
		return BT_ERR;
	if (peer_type == BT_ADDR_BREDR || (info && info->peer_type == BT_ADDR_BREDR))
		return do_pair_bredr(info);
	smp_on = 1;
	smp_sc = 0;
	pair_done = 0;
	pair_fail = 0;
	need_confirm = 0;
	user_ok = 0;
	keys_sent = 0;
	memset(ltk_new, 0, 16);
	req[0] = 0x01;
	req[1] = 0x01;
	req[2] = 0x00;
	req[3] = 0x0d;
	req[4] = 16;
	req[5] = 0x02;
	req[6] = 0x02;
	memcpy(preq, req, 7);
	smp_send(req, 7);
	while (!need_confirm && !pair_done && !pair_fail && !encrypted && spins++ < 400) {
		poll_usb();
		spin_ms(5);
	}
	copy_info(info);
	if (need_confirm && !user_ok)
		return BT_NEED_CONFIRM;
	if (pair_fail)
		return BT_ERR;
	return (encrypted || pair_done) ? BT_OK : BT_ERR;
}

static int do_confirm(BtInfo* info) {
	int spins = 0;
	uint8_t zero[8];
	user_ok = 1;
	need_confirm = 0;
	if (info && info->passkey)
		passkey = info->passkey;
	memset(zero, 0, 8);
	lesc_dhcheck();
	remember_bond(1);
	start_enc(zero, 0, ltk_new);
	while (!encrypted && !pair_fail && spins++ < 300) {
		poll_usb();
		spin_ms(5);
	}
	copy_info(info);
	return encrypted ? BT_OK : BT_ERR;
}

static int ble_write(uint8_t* buffer, size_t length) {
	size_t i = 0;
	if (!audio_on || !cis_handle)
		return 0;
	while (i + 1 < length) {
		int16_t s = (int16_t)(buffer[i] | (buffer[i + 1] << 8));
		pcm_acc[pcm_n++] = s;
		i += 2;
		if (pcm_n >= LC3_SAMPLES) {
			uint8_t frame[LC3_BYTES];
			uint8_t pkt[8 + LC3_BYTES];
			uint16_t hf = (uint16_t)(cis_handle | (2u << 12));
			uint16_t dlen = 4 + LC3_BYTES;
			uint16_t sl = LC3_BYTES;
			bt_lc3_encode(pcm_acc, frame);
			put16(pkt, hf);
			put16(pkt + 2, dlen);
			put16(pkt + 4, iso_seq++);
			put16(pkt + 6, sl);
			memcpy(pkt + 8, frame, LC3_BYTES);
			transport_iso(pkt, (uint16_t)(8 + LC3_BYTES));
			pcm_n = 0;
		}
	}
	return (int)length;
}

static int ble_start(void) {
	return 0;
}
static int ble_stop(void) {
	return 0;
}
static int ble_vol(uint8_t v) {
	(void)v;
	return 0;
}
static int ble_ctl(void* d, int c) {
	(void)d;
	(void)c;
	return 0;
}

static int do_audio(BtInfo* info) {
	uint8_t feat[2];
	uint8_t cig[31];
	uint8_t rsp[16];
	uint8_t rlen;
	uint8_t cis[5];
	uint8_t path[13];
	int spins = 0;
	feat[0] = 32;
	feat[1] = 1;
	hci(LE_SET_HOST_FEAT, feat, 2, NULL, NULL);
	if (!iso_mtu) {
		uint8_t brsp[16];
		uint8_t blen = sizeof brsp;
		if (!hci(LE_READ_BUF_V2, NULL, 0, brsp, &blen) && blen >= 6 && brsp[0] == 0)
			iso_mtu = get16(brsp + 4);
	}
	if (!iso_mtu) {
		if (info) {
			memcpy(info->msg, "no LE Audio", 12);
			info->status = BT_NO_AUDIO;
		}
		return BT_NO_AUDIO;
	}
	memset(cig, 0, sizeof cig);
	cig[0] = 0;
	cig[1] = 0x10;
	cig[2] = 0x27;
	cig[3] = 0x00;
	cig[4] = 0x10;
	cig[5] = 0x27;
	cig[6] = 0x00;
	cig[7] = 0;
	cig[8] = 0;
	cig[9] = 0;
	put16(cig + 10, 10);
	put16(cig + 12, 10);
	cig[14] = 1;
	cig[15] = 0;
	put16(cig + 16, 40);
	put16(cig + 18, 0);
	/* LE 1M PHY. 2M is optional and a controller that only has 1M
	 * rejects the CIG, which btctl reports as "no LE Audio". */
	cig[20] = 0x01;
	cig[21] = 0x01;
	cig[22] = 2;
	cig[23] = 2;
	rlen = sizeof rsp;
	if (hci(LE_SET_CIG, cig, 24, rsp, &rlen) || rsp[0] != 0) {
		if (info)
			memcpy(info->msg, "no LE Audio", 12);
		return BT_NO_AUDIO;
	}
	if (rlen >= 5)
		cis_handle = get16(rsp + 3);
	cis[0] = 1;
	put16(cis + 1, cis_handle);
	put16(cis + 3, handle);
	hci(LE_CREATE_CIS, cis, 5, NULL, NULL);
	while (spins++ < 200) {
		poll_usb();
		spin_ms(5);
	}
	memset(path, 0, sizeof path);
	put16(path, cis_handle);
	path[2] = 0;
	path[3] = 0;
	path[4] = 0x06;
	hci(LE_SETUP_ISO, path, 13, NULL, NULL);
	memset(&ble_card, 0, sizeof ble_card);
	memcpy(ble_card.name, "ble0", 5);
	ble_card.write = ble_write;
	ble_card.start_output = ble_start;
	ble_card.stop_output = ble_stop;
	ble_card.set_vol = ble_vol;
	ble_card.control = ble_ctl;
	ble_card._force_write = 1;
	if (AuSoundRegisterCard(&ble_card) == 0)
		card_id = au_sound_last_id;
	audio_on = 1;
	copy_info(info);
	if (info) {
		memcpy(info->msg, "ble0", 5);
		info->card_id = card_id;
	}
	return BT_OK;
}

static int perform(int code, BtInfo* info) {
	int st = BT_ERR;
	switch (code) {
	case BT_GET_INFO:
		copy_info(info);
		st = (have_usb || have_h4uart) ? BT_OK : BT_ERR;
		break;
	case BT_SCAN:
		st = do_scan(info);
		break;
	case BT_CONNECT:
		st = do_connect(info);
		break;
	case BT_DISCONNECT: {
		uint8_t p[3];
		put16(p, handle);
		p[2] = 0x13;
		st = hci(HCI_DISCONNECT, p, 3, NULL, NULL) ? BT_ERR : BT_OK;
		copy_info(info);
		break;
	}
	case BT_READ_NAME:
		st = read_peer_name() ? BT_ERR : BT_OK;
		copy_info(info);
		break;
	case BT_PAIR:
		st = do_pair(info);
		break;
	case BT_CONFIRM:
	case BT_PASSKEY:
		st = do_confirm(info);
		break;
	case BT_AUDIO:
		st = do_audio(info);
		break;
	default:
		st = BT_ERR;
		break;
	}
	/* TEMP-BT-TEST (revert before commit): mirror ioctl results to serial
	 * since btctl stdout goes to the framebuffer console in TERM runs. */
	UARTDebugOut("[bt-test]: cmd=%d st=%d nscan=%d conn=%d enc=%d\r\n", code, st,
				 info ? info->nscan : -1, connected, encrypted);
	if (info && code == BT_SCAN) {
		int si;
		for (si = 0; si < info->nscan && si < BT_MAX_SCAN; si++) {
			char ab[24];
			addr_str(info->scan[si].addr, ab, sizeof ab);
			UARTDebugOut("[bt-test]: scan %s type=%d rssi=%d %s\r\n", ab,
						 info->scan[si].addr_type, info->scan[si].rssi,
						 info->scan[si].name);
		}
	}
	if (info && code == BT_READ_NAME)
		UARTDebugOut("[bt-test]: name=%s\r\n", info->peer_name);
	if (info)
		info->status = st;
	snoop_flush();
	return st;
}

static void bt_thread(uint64_t arg) {
	(void)arg;
	for (;;) {
		if (job && !job_done) {
			/* TEMP-BT-TEST (revert before commit) */
			UARTDebugOut("[bt-test]: pickup job=%d\r\n", job);
			job_status = perform(job, job_info);
			job = 0;
			/* perform() writes job_buf. The waiter runs on another thread
			 * and must not copy that buffer until those stores are done.
			 * Do not point job_info at the caller's BtInfo or switch TTBR0
			 * to the caller's table: do_scan yields, and the scheduler
			 * reloads this thread's root table before copy_info runs. */
			__asm__ volatile("dmb ish" ::: "memory");
			job_done = 1;
		} else {
			poll_usb();
		}
		AuSleepThread(AuGetCurrentThread(), 10);
		AuScheduleNext();
	}
}

static int submit(int code, BtInfo* info) {
	int spins = 0;
	int st;
	if (!info)
		return BT_ERR;
	/* TEMP-BT-TEST (revert before commit) */
	UARTDebugOut("[bt-test]: submit code=%d started=%d\r\n", code, bt_started);
	/* bthost runs on the root page table. Its low half is empty, so a
	 * user BtInfo pointer faults there. Copy through kernel memory and
	 * write the result back on this thread, which still has the caller. */
	if (!bt_started)
		return perform(code, info);
	memcpy(&job_buf, info, sizeof job_buf);
	job_info = &job_buf;
	job_done = 0;
	job_status = BT_ERR;
	__asm__ volatile("dmb ish" ::: "memory");
	job = code;
	while (!job_done && spins++ < 800) {
		AuSleepThread(AuGetCurrentThread(), 20);
		AuScheduleNext();
	}
	st = job_done ? job_status : BT_ERR;
	__asm__ volatile("dmb ish" ::: "memory");
	memcpy(info, &job_buf, sizeof job_buf);
	if (!job_done && info->msg[0] == 0)
		memcpy(info->msg, "timed out", 10);
	return st;
}

static size_t bt_read(AuVFSNode* fs, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
	char text[128];
	char addr[24];
	uint32_t n;
	(void)fs;
	(void)file;
	addr_str(bd, addr, sizeof addr);
	text[0] = 0;
	{
		int i = 0;
		const char* s = addr;
		while (*s && i < 40)
			text[i++] = *s++;
		text[i++] = ' ';
		text[i++] = 'i';
		text[i++] = 's';
		text[i++] = 'o';
		text[i++] = '=';
		text[i++] = iso_mtu ? '1' : '0';
		text[i++] = '\n';
		text[i] = 0;
		n = (uint32_t)i;
	}
	if (n > length)
		n = length;
	memcpy(buffer, text, n);
	return n;
}

static int bt_ioctl(AuVFSNode* node, int code, void* arg) {
	(void)node;
	return submit(code, (BtInfo*)arg);
}

static void publish_dev(void) {
	AuVFSNode* dev = AuVFSFind("/dev");
	AuVFSNode* node;
	if (!dev)
		return;
	node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	if (!node)
		return;
	memset(node, 0, sizeof *node);
	memcpy(node->filename, "bt0", 4);
	node->flags = FS_FLAG_DEVICE;
	node->read = bt_read;
	node->iocontrol = bt_ioctl;
	AuDevFSAddFile(dev, "/bt0", node);
}

static void identity(void) {
	uint8_t mask[8];
	uint8_t lehost[2];
	uint8_t feat[2];
	uint8_t rsp[32];
	uint8_t rlen;
	char shown[24];
	memset(mask, 0xff, 8);
	hci(HCI_SET_EVENT_MASK, mask, 8, NULL, NULL);
	hci(HCI_RESET, NULL, 0, NULL, NULL);
	hci(HCI_SET_EVENT_MASK, mask, 8, NULL, NULL);
	lehost[0] = 1;
	lehost[1] = 0;
	hci(HCI_WRITE_LE_HOST, lehost, 2, NULL, NULL);
	hci(LE_SET_EVENT_MASK, mask, 8, NULL, NULL);
	rlen = sizeof rsp;
	if (!hci(HCI_READ_VER, NULL, 0, rsp, &rlen) && rlen >= 2)
		hci_ver = rsp[1];
	rlen = sizeof rsp;
	if (!hci(HCI_READ_BD, NULL, 0, rsp, &rlen) && rlen >= 7)
		memcpy(bd, rsp + 1, 6);
	rlen = sizeof rsp;
	if (!hci(HCI_READ_BUF, NULL, 0, rsp, &rlen) && rlen >= 5) {
		uint16_t m = get16(rsp + 1);
		if (m) {
			acl_mtu = m;
			acl_max = get16(rsp + 3);
			if (acl_max == 0 && rlen >= 5)
				acl_max = rsp[4];
			acl_credits = acl_max ? acl_max : 1;
		}
	}
	rlen = sizeof rsp;
	if (!hci(LE_READ_BUF, NULL, 0, rsp, &rlen) && rlen >= 4 && get16(rsp + 1)) {
		acl_mtu = get16(rsp + 1);
		acl_max = rsp[3];
		acl_credits = acl_max ? acl_max : 1;
	}
	rlen = sizeof rsp;
	if (!hci(LE_READ_FEAT, NULL, 0, rsp, &rlen) && rlen >= 9)
		memcpy(le_feat, rsp + 1, 8);
	/* Bit 32 is Connected Isochronous Stream (Host Support). Controllers
	 * report a zero ISO packet length until the host sets it, which made
	 * btctl audio print "no LE Audio" on adapters that can do LE Audio.
	 * The bit has to be set before any connection exists. */
	feat[0] = 32;
	feat[1] = 1;
	hci(LE_SET_HOST_FEAT, feat, 2, NULL, NULL);
	rlen = sizeof rsp;
	iso_mtu = 0;
	if (!hci(LE_READ_BUF_V2, NULL, 0, rsp, &rlen) && rlen >= 6 && rsp[0] == 0)
		iso_mtu = get16(rsp + 4);
	if (!iso_mtu) {
		rlen = sizeof rsp;
		if (!hci(LE_READ_BUF_V2, NULL, 0, rsp, &rlen) && rlen >= 6 && rsp[0] == 0)
			iso_mtu = get16(rsp + 4);
	}
	addr_str(bd, shown, sizeof shown);
	UARTDebugOut("[aurora]: bt %s iso=%d\r\n", shown, iso_mtu ? 1 : 0);
	/* Boot-time LE scan — results go to the QEMU serial console. */
	{
		uint8_t par[7];
		uint8_t en[2];
		int spins = 0;
		par[0] = 0x01;
		put16(par + 1, 0x0060);
		put16(par + 3, 0x0030);
		par[5] = 0;
		par[6] = 0;
		nscan = 0;
		if (!hci(LE_SET_SCAN_PAR, par, 7, NULL, NULL)) {
			en[0] = 1;
			en[1] = 0;
			if (!hci(LE_SET_SCAN_EN, en, 2, NULL, NULL)) {
				while (spins++ < 300) {
					poll_usb();
					spin_ms(10);
				}
				en[0] = 0;
				hci(LE_SET_SCAN_EN, en, 2, NULL, NULL);
			}
		}
		if (nscan > 0) {
			int i;
			for (i = 0; i < nscan && i < BT_MAX_SCAN; i++) {
				char ab[24];
				addr_str(scans[i].addr, ab, sizeof ab);
				UARTDebugOut("[btscan]: %s type=%d rssi=%d %s\r\n", ab, scans[i].addr_type,
							 scans[i].rssi, scans[i].name);
			}
		} else {
			UARTDebugOut("[btscan]: no advertisers found\r\n");
		}
	}
	publish_dev();
	bond_load();
	snoop_open();
	snoop_flush();
}

static void snoop_open(void) {
	snoop_len = 0;
}

void AuBtUsbReady(const AuBtUsbOps* ops) {
	AA64Thread* thr;
	if (!ops)
		return;
	usb = *ops;
	have_usb = 1;
	identity();
	thr = AuCreateKthread(bt_thread, AuGetRootPageTable(), "bthost");
	(void)thr;
	bt_started = 1;
}

void AuBtInitialize(void) {
	bt_h4_reset(&h4);
#ifdef __TARGET_BOARD_QEMU_VIRT__
	/* Zephyr-style serial bridge first: UART1 <-> -serial unix:/tmp/bt-server-bredr
	 * <-> btproxy <-> BlueZ HCI_USER. If UART1 answers, the real host
	 * adapter owns the stack and the mock is skipped. */
	{
		bt_uart_init_hw();
		if (bt_uart) {
			bt_h4_reset(&h4);
			have_h4uart = 1;
			if (!hci(HCI_RESET, NULL, 0, NULL, NULL)) {
				AA64Thread* thr;
				UARTDebugOut("[bt]: H4 UART1 bridge detected, using host adapter\r\n");
				identity();
				thr = AuCreateKthread(bt_thread, AuGetRootPageTable(), "bthost");
				(void)thr;
				bt_started = 1;
				return;
			}
			have_h4uart = 0;
			bt_h4_reset(&h4);
			UARTDebugOut("[bt]: no H4 UART1 controller, falling back\r\n");
		}
	}
#endif
	{
		int b, d, f;
		for (b = 0; b < 256 && !have_usb; b++)
			for (d = 0; d < 32 && !have_usb; d++)
				for (f = 0; f < 8 && !have_usb; f++) {
					uint64_t addr = AuPCIEGetDevice(0, b, d, f);
					if (!addr)
						continue;
					uint16_t vend = AuPCIERead(addr, PCI_VENDOR_ID, b, d, f);
					uint16_t did = AuPCIERead(addr, PCI_DEVICE_ID, b, d, f);
					if (vend == 0x1AF4 && did == 0x1043)
						AuVirtioBtInitialize(addr, b, d, f);
				}
	}
	if (!have_usb) {
		/* No virtio-serial PCI device: use the mock controller. */
		AuVirtioBtInitialize(0, 0, 0, 0);
	}
	if (!have_usb && !have_h4uart)
		UARTDebugOut("[btscan]: no adapter\r\n");
}

/* H4 byte pipe. A second UART calls this; the console UART is not used. */
void AuBtH4Push(uint8_t byte) {
	uint8_t kind = 0;
	uint8_t out[512];
	uint16_t olen = 0;
	uint8_t b = byte;
	h4_on = 1;
	if (!bt_h4_feed(&h4, &b, 1, &kind, out, &olen, sizeof out))
		return;
	if (kind == H4_EVT)
		on_event(out, olen);
	else if (kind == H4_ACL)
		on_acl(out, olen);
}
