//
// usbfunction.h
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
#ifndef _uspi_usbfunction_h
#define _uspi_usbfunction_h

#include "uspi/usbconfigparser.h"
#include "uspi/usb.h"
#include "uspi/string.h"
#include "uspi/types.h"

struct TUSBDevice;
struct TDWHCIDevice;
struct TUSBEndpoint;

typedef struct TUSBFunction
{
	boolean (*Configure) (struct TUSBFunction *pThis);

	struct TUSBDevice *m_pDevice;

	TUSBConfigurationParser *m_pConfigParser;

	TUSBInterfaceDescriptor *m_pInterfaceDesc;
}
TUSBFunction;

void USBFunction (TUSBFunction *pThis, struct TUSBDevice *pDevice, TUSBConfigurationParser *pConfigParser);
void USBFunctionCopy (TUSBFunction *pThis, TUSBFunction *pFunction);	// copy constructor
void _USBFunction (TUSBFunction *pThis);

boolean USBFunctionConfigure (TUSBFunction *pThis);

TString *USBFunctionGetInterfaceName (TUSBFunction *pThis);		// string deleted by caller
u8 USBFunctionGetNumEndpoints (TUSBFunction *pThis);

struct TUSBDevice *USBFunctionGetDevice (TUSBFunction *pThis);
struct TUSBEndpoint *USBFunctionGetEndpoint0 (TUSBFunction *pThis);
struct TDWHCIDevice *USBFunctionGetHost (TUSBFunction *pThis);

// get next sub descriptor of ucType from interface descriptor
const TUSBDescriptor *USBFunctionGetDescriptor (TUSBFunction *pThis, u8 ucType); // returns 0 if not found
void USBFunctionConfigurationError (TUSBFunction *pThis, const char *pSource);

// select a specific USB interface, called in constructor of derived class,
// if device has been detected by vendor/product ID
boolean USBFunctionSelectInterfaceByClass (TUSBFunction *pThis, u8 uchClass, u8 uchSubClass, u8 uchProtocol);

u8 USBFunctionGetInterfaceNumber (TUSBFunction *pThis);
u8 USBFunctionGetInterfaceClass (TUSBFunction *pThis);
u8 USBFunctionGetInterfaceSubClass (TUSBFunction *pThis);
u8 USBFunctionGetInterfaceProtocol (TUSBFunction *pThis);

#endif
