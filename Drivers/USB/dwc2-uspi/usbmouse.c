//
// usbmouse.c
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
#include "uspi/usbmouse.h"
#include "uspi/usbhid.h"
#include "uspi/usbhostcontroller.h"
#include "uspi/devicenameservice.h"
#include "uspi/util.h"
#include "uspi/assert.h"
#include "uspios.h"

static unsigned s_nDeviceNumber = 1;

static const char FromUSBMouse[] = "umouse";

static boolean USBMouseDeviceStartRequest (TUSBMouseDevice *pThis);
static void USBMouseDeviceCompletionRoutine (TUSBRequest *pURB, void *pParam, void *pContext);

void USBMouseDevice (TUSBMouseDevice *pThis, TUSBFunction *pDevice)
{
	assert (pThis != 0);

	USBFunctionCopy (&pThis->m_USBFunction, pDevice);
	pThis->m_USBFunction.Configure = USBMouseDeviceConfigure;

	pThis->m_pReportEndpoint = 0;
	pThis->m_pStatusHandler = 0;
	pThis->m_pReportBuffer = 0;

	pThis->m_pReportBuffer = malloc (MOUSE_BOOT_REPORT_SIZE);
	assert (pThis->m_pReportBuffer != 0);

	pThis->m_pHIDReportDescriptor = 0;
}

void _CUSBMouseDevice (TUSBMouseDevice *pThis)
{
	assert (pThis != 0);

	if (pThis->m_pHIDReportDescriptor != 0)
	{
		free (pThis->m_pHIDReportDescriptor);
		pThis->m_pHIDReportDescriptor = 0;
	}

	if (pThis->m_pReportBuffer != 0)
	{
		free (pThis->m_pReportBuffer);
		pThis->m_pReportBuffer = 0;
	}

	if (pThis->m_pReportEndpoint != 0)
	{
		_USBEndpoint (pThis->m_pReportEndpoint);
		free (pThis->m_pReportEndpoint);
		pThis->m_pReportEndpoint = 0;
	}

	_USBFunction (&pThis->m_USBFunction);
}

boolean USBMouseDeviceConfigure (TUSBFunction *pUSBFunction)
{
	TUSBMouseDevice *pThis = (TUSBMouseDevice *) pUSBFunction;
	assert (pThis != 0);

	if (USBFunctionGetNumEndpoints (&pThis->m_USBFunction) <  1)
	{
		USBFunctionConfigurationError (&pThis->m_USBFunction, FromUSBMouse);

		return FALSE;
	}

	TUSBHIDDescriptor *pHIDDesc = (TUSBHIDDescriptor *)
		USBFunctionGetDescriptor (&pThis->m_USBFunction, DESCRIPTOR_HID);
	if (   pHIDDesc == 0
	    || pHIDDesc->wReportDescriptorLength == 0)
	{
		USBFunctionConfigurationError (&pThis->m_USBFunction, FromUSBMouse);

		return FALSE;
	}

	TUSBEndpointDescriptor *pEndpointDesc;
	while ((pEndpointDesc =	(TUSBEndpointDescriptor *)
		USBFunctionGetDescriptor (&pThis->m_USBFunction, DESCRIPTOR_ENDPOINT)) != 0)
	{
		if (   (pEndpointDesc->bEndpointAddress & 0x80) != 0x80		// Input EP
		    || (pEndpointDesc->bmAttributes     & 0x3F)	!= 0x03)	// Interrupt EP
		{
			continue;
		}

		assert (pThis->m_pReportEndpoint == 0);
		pThis->m_pReportEndpoint = malloc (sizeof (TUSBEndpoint));
		assert (pThis->m_pReportEndpoint != 0);
		USBEndpoint2 (pThis->m_pReportEndpoint, USBFunctionGetDevice (&pThis->m_USBFunction), pEndpointDesc);

		break;
	}

	if (pThis->m_pReportEndpoint == 0)
	{
		USBFunctionConfigurationError (&pThis->m_USBFunction, FromUSBMouse);

		return FALSE;
	}
	
	pThis->m_usReportDescriptorLength = pHIDDesc->wReportDescriptorLength;
	pThis->m_pHIDReportDescriptor = (u8 *) malloc (pThis->m_usReportDescriptorLength);
	assert (pThis->m_pHIDReportDescriptor != 0);

	if (DWHCIDeviceControlMessage (USBFunctionGetHost (&pThis->m_USBFunction),
				       USBFunctionGetEndpoint0 (&pThis->m_USBFunction),
				       REQUEST_IN | REQUEST_TO_INTERFACE, GET_DESCRIPTOR,
				       (pHIDDesc->bReportDescriptorType << 8) | DESCRIPTOR_INDEX_DEFAULT,
				       USBFunctionGetInterfaceNumber (&pThis->m_USBFunction),
				       pThis->m_pHIDReportDescriptor, pThis->m_usReportDescriptorLength)
	    != pThis->m_usReportDescriptorLength)
	{
		LogWrite (FromUSBMouse, LOG_ERROR, "Cannot get HID report descriptor");

		return FALSE;
	}
	//DebugHexdump (pThis->m_pHIDReportDescriptor, pThis->m_usReportDescriptorLength, FromUSBMouse);

	if (!USBFunctionConfigure (&pThis->m_USBFunction))
	{
		LogWrite (FromUSBMouse, LOG_ERROR, "Cannot set interface");

		return FALSE;
	}

	if (DWHCIDeviceControlMessage (USBFunctionGetHost (&pThis->m_USBFunction),
				       USBFunctionGetEndpoint0 (&pThis->m_USBFunction),
				       REQUEST_OUT | REQUEST_CLASS | REQUEST_TO_INTERFACE,
				       SET_PROTOCOL, BOOT_PROTOCOL,
				       USBFunctionGetInterfaceNumber (&pThis->m_USBFunction), 0, 0) < 0)
	{
		LogWrite (FromUSBMouse, LOG_ERROR, "Cannot set boot protocol");

		return FALSE;
	}

	TString DeviceName;
	String (&DeviceName);
	StringFormat (&DeviceName, "umouse%u", s_nDeviceNumber++);
	DeviceNameServiceAddDevice (DeviceNameServiceGet (), StringGet (&DeviceName), pThis, FALSE);

	_String (&DeviceName);

	return USBMouseDeviceStartRequest (pThis);
}

void USBMouseDeviceRegisterStatusHandler (TUSBMouseDevice *pThis, TMouseStatusHandler *pStatusHandler)
{
	assert (pThis != 0);
	assert (pStatusHandler != 0);
	pThis->m_pStatusHandler = pStatusHandler;
}

boolean USBMouseDeviceStartRequest (TUSBMouseDevice *pThis)
{
	assert (pThis != 0);

	assert (pThis->m_pReportEndpoint != 0);
	assert (pThis->m_pReportBuffer != 0);
	
	USBRequest (&pThis->m_URB, pThis->m_pReportEndpoint, pThis->m_pReportBuffer, MOUSE_BOOT_REPORT_SIZE, 0);
	USBRequestSetCompletionRoutine (&pThis->m_URB, USBMouseDeviceCompletionRoutine, 0, pThis);
	
	return DWHCIDeviceSubmitAsyncRequest (USBFunctionGetHost (&pThis->m_USBFunction), &pThis->m_URB);
}

void USBMouseDeviceCompletionRoutine (TUSBRequest *pURB, void *pParam, void *pContext)
{
	TUSBMouseDevice *pThis = (TUSBMouseDevice *) pContext;
	assert (pThis != 0);
	
	assert (pURB != 0);
	assert (&pThis->m_URB == pURB);

	if (   USBRequestGetStatus (pURB) != 0
	    && USBRequestGetResultLength (pURB) == MOUSE_BOOT_REPORT_SIZE
	    && pThis->m_pStatusHandler != 0)
	{
		assert (pThis->m_pReportBuffer != 0);
		(*pThis->m_pStatusHandler) (pThis->m_pReportBuffer[0],
					    uspi_char2int ((char) pThis->m_pReportBuffer[1]),
					    uspi_char2int ((char) pThis->m_pReportBuffer[2]));
	}

	_USBRequest (&pThis->m_URB);
	
	USBMouseDeviceStartRequest (pThis);
}
