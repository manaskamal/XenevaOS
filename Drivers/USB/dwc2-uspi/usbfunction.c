//
// usbfunction.c
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
#include "uspi/usbfunction.h"
#include "uspi/usbdevice.h"
#include "uspi/dwhcidevice.h"
#include "uspi/usbendpoint.h"
#include "uspios.h"
#include "uspi/assert.h"

static const char FromUSBFunction[] = "usbfct";

void USBFunction (TUSBFunction *pThis, TUSBDevice *pDevice, TUSBConfigurationParser *pConfigParser)
{
	assert (pThis != 0);

	pThis->Configure = 0;

	pThis->m_pDevice = pDevice;
	assert (pThis->m_pDevice != 0);

	pThis->m_pConfigParser = (TUSBConfigurationParser *) malloc (sizeof (TUSBConfigurationParser));
	assert (pThis->m_pConfigParser != 0);
	assert (pConfigParser != 0);
	USBConfigurationParserCopy (pThis->m_pConfigParser, pConfigParser);

	pThis->m_pInterfaceDesc = (TUSBInterfaceDescriptor *) USBConfigurationParserGetCurrentDescriptor (pThis->m_pConfigParser);
	assert (pThis->m_pInterfaceDesc != 0);
}

void USBFunctionCopy (TUSBFunction *pThis, TUSBFunction *pFunction)
{
	assert (pThis != 0);
	assert (pFunction != 0);

	pThis->Configure = pFunction->Configure;

	pThis->m_pDevice = pFunction->m_pDevice;
	assert (pThis->m_pDevice != 0);

	pThis->m_pConfigParser = (TUSBConfigurationParser *) malloc (sizeof (TUSBConfigurationParser));
	assert (pThis->m_pConfigParser != 0);
	assert (pFunction->m_pConfigParser != 0);
	USBConfigurationParserCopy (pThis->m_pConfigParser, pFunction->m_pConfigParser);

	pThis->m_pInterfaceDesc = (TUSBInterfaceDescriptor *) USBConfigurationParserGetCurrentDescriptor (pThis->m_pConfigParser);
	assert (pThis->m_pInterfaceDesc != 0);
}

void _USBFunction (TUSBFunction *pThis)
{
	assert (pThis != 0);

	pThis->m_pInterfaceDesc = 0;

	_USBConfigurationParser (pThis->m_pConfigParser);
	free (pThis->m_pConfigParser);
	pThis->m_pConfigParser = 0;

	pThis->m_pDevice = 0;

	pThis->Configure = 0;
}

boolean USBFunctionConfigure (TUSBFunction *pThis)
{
	assert (pThis != 0);

	assert (pThis->m_pInterfaceDesc != 0);
	if (pThis->m_pInterfaceDesc->bAlternateSetting != 0)
	{
		if (DWHCIDeviceControlMessage (USBFunctionGetHost (pThis), USBFunctionGetEndpoint0 (pThis),
						REQUEST_OUT | REQUEST_TO_INTERFACE, SET_INTERFACE,
						pThis->m_pInterfaceDesc->bAlternateSetting,
						pThis->m_pInterfaceDesc->bInterfaceNumber, 0, 0) < 0)
		{
			LogWrite (FromUSBFunction, LOG_ERROR, "Cannot set interface");

			return FALSE;
		}
	}

	return TRUE;
}

TString *USBFunctionGetInterfaceName (TUSBFunction *pThis)
{
	assert (pThis != 0);

	TString *pString = (TString *) malloc (sizeof (TString));
	assert (pString != 0);
	String2 (pString, "unknown");

	if (   pThis->m_pInterfaceDesc != 0
	    && pThis->m_pInterfaceDesc->bInterfaceClass != 0x00
	    && pThis->m_pInterfaceDesc->bInterfaceClass != 0xFF)
	{
		StringFormat (pString, "int%x-%x-%x",
			      (unsigned) pThis->m_pInterfaceDesc->bInterfaceClass,
			      (unsigned) pThis->m_pInterfaceDesc->bInterfaceSubClass,
			      (unsigned) pThis->m_pInterfaceDesc->bInterfaceProtocol);
	}

	return pString;
}

u8 USBFunctionGetNumEndpoints (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	return pThis->m_pInterfaceDesc->bNumEndpoints;
}

TUSBDevice *USBFunctionGetDevice (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pDevice != 0);
	return pThis->m_pDevice;
}

TUSBEndpoint *USBFunctionGetEndpoint0 (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pDevice != 0);
	return USBDeviceGetEndpoint0 (pThis->m_pDevice);
}

TDWHCIDevice *USBFunctionGetHost (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pDevice != 0);
	return USBDeviceGetHost (pThis->m_pDevice);
}

const TUSBDescriptor *USBFunctionGetDescriptor (TUSBFunction *pThis, u8 ucType)
{
	assert (pThis != 0);
	assert (pThis->m_pConfigParser != 0);
	return USBConfigurationParserGetDescriptor (pThis->m_pConfigParser, ucType);
}

void USBFunctionConfigurationError (TUSBFunction *pThis, const char *pSource)
{
	assert (pThis != 0);
	assert (pThis->m_pConfigParser != 0);
	assert (pSource != 0);
	USBConfigurationParserError (pThis->m_pConfigParser, pSource);
}

boolean USBFunctionSelectInterfaceByClass (TUSBFunction *pThis, u8 uchClass, u8 uchSubClass, u8 uchProtocol)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	assert (pThis->m_pConfigParser != 0);
	assert (pThis->m_pDevice != 0);

	do
	{
		if (   pThis->m_pInterfaceDesc->bInterfaceClass    == uchClass
		    && pThis->m_pInterfaceDesc->bInterfaceSubClass == uchSubClass
		    && pThis->m_pInterfaceDesc->bInterfaceProtocol == uchProtocol)
		{
			return TRUE;
		}

		// skip to next interface in interface enumeration in class CDevice
		USBDeviceGetDescriptor (pThis->m_pDevice, DESCRIPTOR_INTERFACE);
	}
	while ((pThis->m_pInterfaceDesc = (TUSBInterfaceDescriptor *)
				USBConfigurationParserGetDescriptor (pThis->m_pConfigParser, DESCRIPTOR_INTERFACE)) != 0);

	return FALSE;
}

u8 USBFunctionGetInterfaceNumber (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	return pThis->m_pInterfaceDesc->bInterfaceNumber;
}

u8 USBFunctionGetInterfaceClass (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	return pThis->m_pInterfaceDesc->bInterfaceClass;
}

u8 USBFunctionGetInterfaceSubClass (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	return pThis->m_pInterfaceDesc->bInterfaceSubClass;
}

u8 USBFunctionGetInterfaceProtocol (TUSBFunction *pThis)
{
	assert (pThis != 0);
	assert (pThis->m_pInterfaceDesc != 0);
	return pThis->m_pInterfaceDesc->bInterfaceProtocol;
}
