//
// usbmidi.c
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2016-2018  R. Stange <rsta2@o2online.de>
// Copyright (C) 2016  J. Otto <joshua.t.otto@gmail.com>
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

// Refer to "Universal Serial Bus Device Class Specification for MIDI Devices"

#include "uspi/usbmidi.h"
#include "uspi/usbhostcontroller.h"
#include "uspi/devicenameservice.h"
#include "uspi/assert.h"
#include "uspios.h"

static const char FromMIDI[] = "umidi";

static unsigned s_nDeviceNumber = 1;

static boolean USBMIDIDeviceStartRequest (TUSBMIDIDevice *pThis);
static void USBMIDIDeviceCompletionRoutine (TUSBRequest *pURB, void *pParam, void *pContext);

// This handy table from the Linux driver encodes the mapping between MIDI
// packet Code Index Number values and encapsulated packet lengths.
static const unsigned cin_to_length[] = {
	0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
};

#define EVENT_PACKET_SIZE 4

void USBMIDIDevice (TUSBMIDIDevice *pThis, TUSBFunction *pDevice)
{
	assert (pThis != 0);

	USBFunctionCopy (&pThis->m_USBFunction, pDevice);
	pThis->m_USBFunction.Configure = USBMIDIDeviceConfigure;

	pThis->m_pEndpointIn = 0;
	pThis->m_pPacketHandler = 0;
	pThis->m_pPacketBuffer = 0;
}

void _CUSBMIDIDevice (TUSBMIDIDevice *pThis)
{
	assert (pThis != 0);

	if (pThis->m_pPacketBuffer != 0)
	{
		free (pThis->m_pPacketBuffer);
		pThis->m_pPacketBuffer = 0;
	}

	if (pThis->m_pEndpointIn != 0)
	{
		_USBEndpoint (pThis->m_pEndpointIn);
		free (pThis->m_pEndpointIn);
		pThis->m_pEndpointIn = 0;
	}

	_USBFunction (&pThis->m_USBFunction);
}

boolean USBMIDIDeviceConfigure (TUSBFunction *pUSBFunction)
{
	TUSBMIDIDevice *pThis = (TUSBMIDIDevice *)pUSBFunction;
	assert (pThis != 0);

	if (USBFunctionGetNumEndpoints (&pThis->m_USBFunction) <  1)
	{
		USBFunctionConfigurationError (&pThis->m_USBFunction, FromMIDI);

		return FALSE;
	}

	// Our strategy for now is simple: we'll take the first MIDI streaming
	// bulk-in endpoint on this interface we can find.  To distinguish
	// between the MIDI streaming bulk-in endpoints we want (which carry
	// actual MIDI data streams) and 'transfer bulk data' endpoints (which
	// are used to implement features like Downloadable Samples that we
	// don't care about), we'll look for an immediately-accompanying
	// class-specific endpoint descriptor.
	TUSBAudioEndpointDescriptor *pEndpointDesc;
	while ((pEndpointDesc = (TUSBAudioEndpointDescriptor *) USBFunctionGetDescriptor (&pThis->m_USBFunction, DESCRIPTOR_ENDPOINT)) != 0)
	{
		if (   (pEndpointDesc->bEndpointAddress & 0x80) != 0x80		// Input EP
		    || (pEndpointDesc->bmAttributes     & 0x3F) != 0x02)	// Bulk EP
		{
			continue;
		}

		TUSBMIDIStreamingEndpointDescriptor *pMIDIDesc =
			(TUSBMIDIStreamingEndpointDescriptor *) USBFunctionGetDescriptor (&pThis->m_USBFunction, DESCRIPTOR_CS_ENDPOINT);
		if (   pMIDIDesc == 0
		    || (u8 *)pEndpointDesc + pEndpointDesc->bLength != (u8 *)pMIDIDesc)
		{
			continue;
		}

		assert (pThis->m_pEndpointIn == 0);
		pThis->m_pEndpointIn = malloc (sizeof (TUSBEndpoint));
		assert (pThis->m_pEndpointIn != 0);

		pThis->m_usBufferSize = pEndpointDesc->wMaxPacketSize;
		pThis->m_usBufferSize -= pEndpointDesc->wMaxPacketSize % EVENT_PACKET_SIZE;
		assert (pThis->m_pPacketBuffer == 0);
		pThis->m_pPacketBuffer = malloc (pThis->m_usBufferSize);
		assert (pThis->m_pPacketBuffer != 0);

		USBEndpoint2 (pThis->m_pEndpointIn, USBFunctionGetDevice (&pThis->m_USBFunction), (TUSBEndpointDescriptor *) pEndpointDesc);
	}

	if (pThis->m_pEndpointIn == 0)
	{
		USBFunctionConfigurationError (&pThis->m_USBFunction, FromMIDI);

		return FALSE;
	}

	if (!USBFunctionConfigure (&pThis->m_USBFunction))
	{
		LogWrite (FromMIDI, LOG_ERROR, "Cannot set interface");

		return FALSE;
	}

	TString DeviceName;
	String (&DeviceName);
	StringFormat (&DeviceName, "umidi%u", s_nDeviceNumber++);
	DeviceNameServiceAddDevice (DeviceNameServiceGet (), StringGet (&DeviceName), pThis, FALSE);

	_String(&DeviceName);

	return USBMIDIDeviceStartRequest (pThis);
}

void USBMIDIDeviceRegisterPacketHandler (TUSBMIDIDevice *pThis, TMIDIPacketHandler *pPacketHandler)
{
	assert (pThis != 0);
	assert (pPacketHandler != 0);
	pThis->m_pPacketHandler = pPacketHandler;
}

boolean USBMIDIDeviceStartRequest (TUSBMIDIDevice *pThis)
{
	assert (pThis != 0);

	assert (pThis->m_pEndpointIn != 0);
	assert (pThis->m_pPacketBuffer != 0);

	USBRequest (&pThis->m_URB, pThis->m_pEndpointIn, pThis->m_pPacketBuffer, pThis->m_usBufferSize, 0);
	USBRequestSetCompletionRoutine (&pThis->m_URB, USBMIDIDeviceCompletionRoutine, 0, pThis);

	return DWHCIDeviceSubmitAsyncRequest (USBFunctionGetHost (&pThis->m_USBFunction), &pThis->m_URB);
}

void USBMIDIDeviceCompletionRoutine (TUSBRequest *pURB, void *pParam, void *pContext)
{
	TUSBMIDIDevice *pThis = (TUSBMIDIDevice *) pContext;
	assert (pThis != 0);

	assert (pURB != 0);
	assert (&pThis->m_URB == pURB);

	if (   USBRequestGetStatus (pURB) != 0
		&& USBRequestGetResultLength (pURB) % EVENT_PACKET_SIZE == 0)
	{
		assert (pThis->m_pPacketBuffer != 0);

		u8 *pEnd = pThis->m_pPacketBuffer + USBRequestGetResultLength(pURB);
		for (u8 *pPacket = pThis->m_pPacketBuffer; pPacket < pEnd; pPacket += EVENT_PACKET_SIZE)
		{
			// Follow the Linux driver's example and ignore packets with Cable
			// Number == Code Index Number == 0, which some devices seem to
			// generate as padding in spite of their status as reserved.
			if (pPacket[0] != 0)
			{
				if (pThis->m_pPacketHandler != 0)
				{
					unsigned nCable = pPacket[0] >> 4;
					unsigned nLength = cin_to_length[pPacket[0] & 0xf];
					pThis->m_pPacketHandler(nCable, nLength, &pPacket[1]);
				}
			}
		}
	}

	_USBRequest (&pThis->m_URB);

	USBMIDIDeviceStartRequest (pThis);
}
