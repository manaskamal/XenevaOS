//
// dwhcirootport.cpp
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include "uspi/dwhcirootport.h"
#include "uspi/dwhcidevice.h"
#include "uspi/usbdevicefactory.h"
#include "uspi/usbstandardhub.h"
#include "uspi/assert.h"
#include "uspios.h"
#include <Drivers/uart.h>

static const char FromDWHCIRoot[] = "dwroot";

void DWHCIRootPort (TDWHCIRootPort *pThis, struct TDWHCIDevice *pHost)
{
	assert (pThis != 0);

	pThis->m_pHost = pHost;
	pThis->m_pDevice = 0;

	assert (pThis->m_pHost != 0);
}

void _DWHCIRootPort (TDWHCIRootPort *pThis)
{
	assert (pThis != 0);

	if (pThis->m_pDevice != 0)
	{
		_USBDevice (pThis->m_pDevice);
		free (pThis->m_pDevice);
		pThis->m_pDevice = 0;
	}

	pThis->m_pHost = 0;
}

boolean DWHCIRootPortInitialize (TDWHCIRootPort *pThis)
{
	assert (pThis != 0);

	assert (pThis->m_pHost != 0);
	TUSBSpeed Speed = DWHCIDeviceGetPortSpeed (pThis->m_pHost);
	if (Speed == USBSpeedUnknown)
	{
		LogWrite (FromDWHCIRoot, LOG_ERROR, "Cannot detect port speed");

		return FALSE;
	}
	
	// first create default device
	assert (pThis->m_pDevice == 0);
	pThis->m_pDevice = (TUSBDevice *) malloc (sizeof (TUSBDevice));
	assert (pThis->m_pDevice != 0);
	USBDevice (pThis->m_pDevice, pThis->m_pHost, Speed, FALSE, 0, 1);

	if (!USBDeviceInitialize (pThis->m_pDevice))
	{
		UARTDebugOut("Failed to initialize USB device \r\n");
		_USBDevice (pThis->m_pDevice);
		free (pThis->m_pDevice);
		pThis->m_pDevice = 0;

		return FALSE;
	}

	UARTDebugOut("USB Device initialized successfully \r\n");
	if (!USBDeviceConfigure (pThis->m_pDevice))
	{
		LogWrite (FromDWHCIRoot, LOG_ERROR, "Cannot configure device");

		_USBDevice (pThis->m_pDevice);
		free (pThis->m_pDevice);
		pThis->m_pDevice = 0;

		return FALSE;
	}

	UARTDebugOut("Configured properly \r\n");
	LogWrite (FromDWHCIRoot, LOG_DEBUG, "Device configured");

	// check for over-current
	if (DWHCIDeviceOvercurrentDetected (pThis->m_pHost))
	{
		LogWrite (FromDWHCIRoot, LOG_ERROR, "Over-current condition");

		DWHCIDeviceDisableRootPort (pThis->m_pHost);

		_USBDevice (pThis->m_pDevice);
		free (pThis->m_pDevice);
		pThis->m_pDevice = 0;

		return FALSE;
	}

	return TRUE;
}
