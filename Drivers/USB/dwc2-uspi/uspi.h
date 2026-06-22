//
// uspi.h
//
// Services provided by the USPi library
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
#ifndef _uspi_h
#define _uspi_h

#include "uspi/types.h"
#include <Mm/pmmngr.h>
#ifdef __cplusplus
extern "C" {
#endif

//
// Version information
//

#define USPI_NAME			"USPi library"

#define USPI_MAJOR_VERSION		2
#define USPI_MINOR_VERSION		0
#define USPI_VERSION_STRING		"2.00"

//
// USPi initialization
//

// returns 0 on failure
int USPiInitialize (void);

//
// Keyboard device
//

// returns != 0 if available
int USPiKeyboardAvailable (void);

// "cooked mode"
typedef void TUSPiKeyPressedHandler (const char *pString);
void USPiKeyboardRegisterKeyPressedHandler (TUSPiKeyPressedHandler *pKeyPressedHandler);

// This handler is called when Ctrl-Alt-Del is pressed.
typedef void TUSPiShutdownHandler (void);
void USPiKeyboardRegisterShutdownHandler (TUSPiShutdownHandler *pShutdownHandler);

// Call this frequently from your application main loop to allow updating the keyboard LEDs.
void USPiKeyboardUpdateLEDs (void);

// "raw mode" (if this handler is registered the others are ignored)
// The raw handler is called when the keyboard sends a status report (on status change and/or continously).
typedef void TUSPiKeyStatusHandlerRaw (unsigned char	     ucModifiers,
				       const unsigned char   RawKeys[6]);  // key code or 0 in each byte
void USPiKeyboardRegisterKeyStatusHandlerRaw (TUSPiKeyStatusHandlerRaw *pKeyStatusHandlerRaw);

// ucModifiers (bit is set if modifier key is pressed)
#define LCTRL		(1 << 0)
#define LSHIFT		(1 << 1)
#define ALT		(1 << 2)
#define LWIN		(1 << 3)
#define RCTRL		(1 << 4)
#define RSHIFT		(1 << 5)
#define ALTGR		(1 << 6)
#define RWIN		(1 << 7)

// Set the keyboard LEDs in "raw mode"
void USPiKeyboardSetLEDs (unsigned char ucLEDMask);
#define LED_NUM_LOCK	(1 << 0)
#define LED_CAPS_LOCK	(1 << 1)
#define LED_SCROLL_LOCK	(1 << 2)

//
// Mouse device
//

// returns != 0 if available
int USPiMouseAvailable (void);

// The status handler is called when the mouse sends a status report.
typedef void TUSPiMouseStatusHandler (unsigned nButtons,
				      int nDisplacementX,		// -127..127
				      int nDisplacementY);		// -127..127
void USPiMouseRegisterStatusHandler (TUSPiMouseStatusHandler *pStatusHandler);

// nButtons (bit is set if button is pressed)
#define MOUSE_BUTTON1	(1 << 0)
#define MOUSE_BUTTON2	(1 << 1)
#define MOUSE_BUTTON3	(1 << 2)

//
// Mass storage device
//

// returns number of available devices
int USPiMassStorageDeviceAvailable (void);

#define USPI_BLOCK_SIZE		512			// other block sizes are not supported

// ullOffset and nCount must be multiple of USPI_BLOCK_SIZE
// returns number of read bytes or < 0 on failure
// nDeviceIndex is 0-based
int USPiMassStorageDeviceRead (unsigned long long ullOffset, void *pBuffer, unsigned nCount, unsigned nDeviceIndex);

// ullOffset and nCount must be multiple of USPI_BLOCK_SIZE
// returns number of written bytes or < 0 on failure
// nDeviceIndex is 0-based
int USPiMassStorageDeviceWrite (unsigned long long ullOffset, const void *pBuffer, unsigned nCount, unsigned nDeviceIndex);

// returns the number of available blocks of USPI_BLOCK_SIZE or 0 on failure
unsigned USPiMassStorageDeviceGetCapacity (unsigned nDeviceIndex);

//
// Ethernet services
//
// (You should delay 2 seconds after USPiInitialize before accessing the Ethernet.)
//

// checks the controller only, not if Ethernet link is up
// returns != 0 if available
int USPiEthernetAvailable (void);

void USPiGetMACAddress (unsigned char Buffer[6]);

// returns != 0 if link is up
int USPiEthernetIsLinkUp (void);

// returns 0 on failure
int USPiSendFrame (const void *pBuffer, unsigned nLength);

// pBuffer must have size USPI_FRAME_BUFFER_SIZE
// returns 0 if no frame is available or on failure
#define USPI_FRAME_BUFFER_SIZE	1600
int USPiReceiveFrame (void *pBuffer, unsigned *pResultLength);

//
// GamePad device
//

// returns number of available devices
int USPiGamePadAvailable (void);

#define MAX_AXIS    6
#define MAX_HATS    6

typedef struct USPiGamePadState
{
    int naxes;
    struct
    {
        int value;
        int minimum;
        int maximum;
    } axes[MAX_AXIS];

    int nhats;
    int hats[MAX_HATS];

    int nbuttons;
    unsigned int buttons;
}
USPiGamePadState;

// returns 0 on failure
const USPiGamePadState *USPiGamePadGetStatus (unsigned nDeviceIndex);		// nDeviceIndex is 0-based

typedef void TGamePadStatusHandler (unsigned nDeviceIndex, const USPiGamePadState *pGamePadState);
void USPiGamePadRegisterStatusHandler (TGamePadStatusHandler *pStatusHandler);

//
// MIDI device
//

// returns != 0 if a MIDI device is available
int USPiMIDIAvailable (void);

// The packet handler is called once for each MIDI event packet received from the device.
typedef void TUSPiMIDIPacketHandler (unsigned nCable, unsigned nLength, u8 *pPacket);
void USPiMIDIRegisterPacketHandler (TUSPiMIDIPacketHandler *pPacketHandler);

//
// USB device information
//

#define KEYBOARD_CLASS	1
#define MOUSE_CLASS	2
#define STORAGE_CLASS	3
#define ETHERNET_CLASS	4
#define GAMEPAD_CLASS	5
#define MIDI_CLASS	6

typedef struct TUSPiDeviceInformation
{
	// from USB device descriptor
	unsigned short	idVendor;
	unsigned short	idProduct;
	unsigned short	bcdDevice;

	// points to a buffer in the USPi library, empty string if not available
	const char	*pManufacturer;
	const char	*pProduct;
}
TUSPiDeviceInformation;

// returns 0 on failure
int USPiDeviceGetInformation (unsigned nClass,			// see above
			      unsigned nDeviceIndex,		// 0-based index
			      TUSPiDeviceInformation *pInfo);	// provided buffer is filled

#ifdef __cplusplus
}
#endif

#endif
