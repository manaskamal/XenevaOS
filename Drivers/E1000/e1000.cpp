/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#include "e1000.h"
#include <Mm\kmalloc.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <pcie.h>
#include <_null.h>
#include <aucon.h>
#include <string.h>
#include <aurora.h>

#pragma pack(push,1)
typedef struct _e1000_nic_ {
	uint64_t mmio_addr;
	e1000_rx_desc *rx;
	e1000_tx_desc *tx;
	uint64_t rx_phys;
	uint64_t tx_phys;
	uint8_t* rx_virt[E1000_NUM_RX_DESC];
	uint8_t* tx_virt[E1000_NUM_TX_DESC];
	uint8_t irq;
	int rx_index;
	int tx_index;
	int link_status;
	bool has_eeprom;
	uint8_t mac[6];
}E1000NIC;
#pragma pack(pop)

E1000NIC* e1000_nic;

#define INTS (ICR_LSC | ICR_RXO | ICR_RXT0 | ICR_TXQE | ICR_TXDW | ICR_ACK | ICR_RXDMT0 | ICR_SRPD)
#define CTRL_PHY_RST  (1UL << 31UL)
#define CTRL_RST      (1UL << 26UL)
#define CTRL_SLU      (1UL << 6UL)
#define CTRL_LRST     (1UL << 3UL)

/*
 * MMIORead32 -- reads a 32 bit value from 
 * the given address
 * @param addr -- address from where to read
 */
uint32_t MMIORead32(uint64_t addr) {
	return *((volatile uint32_t*)(addr));
}

/*
 * MMIOWrite32 -- write a 32 bit value to given
 * address
 * @param addr -- address where to write
 * @param val -- value to write
 */
void MMIOWrite32(uint64_t addr, uint32_t val) {
	(*((volatile uint32_t*)(addr))) = val;
}

/*
 * E1000WriteCmd -- write a command to given offset
 * from mmio address
 * @param dev -- pointer to e1000 device
 * @param addr -- offset from mmio address
 * @param val -- value to write
 */
void E1000WriteCmd(E1000NIC* dev, uint16_t addr, uint32_t val) {
	MMIOWrite32(dev->mmio_addr + addr, val);
}

/*
 * E1000ReadCmd -- read a command from given offset from
 * mmio address
 * @param dev -- pointer to e1000 device
 * @param addr -- offset from mmio address
 */
uint32_t E1000ReadCmd(E1000NIC* dev, uint16_t addr) {
	return MMIORead32(dev->mmio_addr + addr);
}

void E1000EEPROMDetect(E1000NIC* dev) {
	E1000WriteCmd(dev, E1000_REG_EEPROM, 1);

	for (int i = 0; i < 10000 && !dev->has_eeprom; ++i) {
		uint32_t val = E1000ReadCmd(dev, E1000_REG_EEPROM);
		if (val & 0x10) dev->has_eeprom = 1;
	}
}

uint16_t E1000EEPROMRead(E1000NIC* dev, uint8_t addr) {
	uint32_t temp = 0;
	E1000WriteCmd(dev, E1000_REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
	while (!((temp = E1000ReadCmd(dev, E1000_REG_EEPROM)) & (1 << 4)));
	return (uint16_t)((temp >> 16) & 0xFFFF);
}

void E1000WriteMAC(E1000NIC* dev) {
	uint32_t low, high;
	memcpy(&low, &dev->mac[0], 4);
	memcpy(&high, &dev->mac[4], 2);
	memset((uint8_t*)&high + 2, 0, 2);
	high |= 0x80000000;
	E1000WriteCmd(dev, E1000_REG_RXADDR + 0, low);
	E1000WriteCmd(dev, E1000_REG_RXADDR + 4, high);
}

void E1000DisableInterrupt(E1000NIC *dev) {
	E1000WriteCmd(dev, E1000_REG_IMC, 0xFFFFFFFF);
	E1000WriteCmd(dev, E1000_REG_ICR, 0xFFFFFFFF);
	E1000ReadCmd(dev, E1000_REG_STATUS);
}

void E1000InitRX(E1000NIC* dev) {
	E1000WriteCmd(dev, E1000_REG_RXDESCLO, V2P(dev->rx_phys));
	E1000WriteCmd(dev, E1000_REG_RXDESCHI, 0);
	E1000WriteCmd(dev, E1000_REG_RXDESCLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc));
	E1000WriteCmd(dev, E1000_REG_RXDESCHEAD, 0);
	E1000WriteCmd(dev, E1000_REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);

	dev->rx_index = 0;

	E1000WriteCmd(dev, E1000_REG_RCTRL, RCTL_EN |
		(1 << 2) |
		(1 << 4) |
		(1 << 15) |
		(1 << 25) |
		(3 << 16) |
		(1 << 26));
}

void E1000InitTX(E1000NIC *dev) {
	E1000WriteCmd(dev, E1000_REG_TXDESCLO, V2P(dev->tx_phys));
	E1000WriteCmd(dev, E1000_REG_TXDESCHI, 0);
	E1000WriteCmd(dev, E1000_REG_TXDESCLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc));
	E1000WriteCmd(dev, E1000_REG_TXDESCHEAD, 0);
	E1000WriteCmd(dev, E1000_REG_TXDESCTAIL, 0);

	dev->tx_index = 0;
	uint32_t tctl = E1000ReadCmd(dev, E1000_REG_TCTRL);

	/* collision threshold */
	tctl &= ~(0xFF << 4);
	tctl |= (15 << 4);

	/* turn it on */
	tctl |= TCTL_EN;
	tctl |= TCTL_PSP;
	tctl |= (1 << 24);

	E1000WriteCmd(dev, E1000_REG_TCTRL, tctl);
}

void E1000SendPacket(E1000NIC* dev, uint8_t* payload, size_t payload_sz) {
	int tx_tail = E1000ReadCmd(dev, E1000_REG_TXDESCTAIL);
	int tx_head = E1000ReadCmd(dev, E1000_REG_TXDESCHEAD);

	memcpy(dev->tx_virt[dev->tx_index], payload, payload_sz);
	dev->tx[dev->tx_index].length = payload_sz;
	dev->tx[dev->tx_index].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
	dev->tx[dev->tx_index].status = 0;

	if (++dev->tx_index == E1000_NUM_TX_DESC)
		dev->tx_index = 0;

	E1000WriteCmd(dev, E1000_REG_TXDESCTAIL, dev->tx_index);
	E1000ReadCmd(dev, E1000_REG_STATUS);
}

/*
* AuDriverUnload -- Frees and clear up everthing from the
* driver
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {
	return 0;
}


AU_EXTERN AU_EXPORT int AuDriverMain() {
	AuTextOut("Starting e1000 driver \n");
	int bus, dev_, func = 0;
	bool nic_thread_required = false;

	uint32_t device = AuPCIEScanClass(0x02, 0x00, &bus, &dev_, &func);
	if (device == 0xFFFFFFFF) {
		AuTextOut("e1000 driver not found \n");
		return 1;
	}

	e1000_nic = (E1000NIC*)kmalloc(sizeof(E1000NIC));
	memset(e1000_nic, 0, sizeof(E1000NIC));

	return 1;
}