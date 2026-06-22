//
// lan7800.h
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2018  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _uspi_lan7800_h
#define _uspi_lan7800_h

#include "uspi/usbfunction.h"
#include "uspi/usbendpoint.h"
#include "uspi/usbrequest.h"
#include "uspi/macaddress.h"
#include "uspi/types.h"

#define FRAME_BUFFER_SIZE	1600

typedef struct TLAN7800Device
{
	TUSBFunction m_USBFunction;

	TUSBEndpoint *m_pEndpointBulkIn;
	TUSBEndpoint *m_pEndpointBulkOut;

	TMACAddress m_MACAddress;

	u8 *m_pTxBuffer;
}
TLAN7800Device;

void LAN7800Device (TLAN7800Device *pThis, TUSBFunction *pFunction);
void _LAN7800Device (TLAN7800Device *pThis);

boolean LAN7800DeviceConfigure (TUSBFunction *pUSBFunction);

TMACAddress *LAN7800DeviceGetMACAddress (TLAN7800Device *pThis);

boolean LAN7800DeviceSendFrame (TLAN7800Device *pThis, const void *pBuffer, unsigned nLength);

// pBuffer must have size FRAME_BUFFER_SIZE
boolean LAN7800DeviceReceiveFrame (TLAN7800Device *pThis, void *pBuffer, unsigned *pResultLength);

// returns TRUE if PHY link is up
boolean LAN7800DeviceIsLinkUp (TLAN7800Device *pThis);

#endif
