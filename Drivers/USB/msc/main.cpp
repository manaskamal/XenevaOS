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

#include <aurora.h>
#include <Drivers/usb.h>
#include <Mm/kmalloc.h>
#include <Mm/pmmngr.h>
#include <aucon.h>
#include <Hal/serial.h>


#define SCSI_CMD_BLK_SIGNATURE 0x43425355 //('USBC')
#define SCSI_CMD_FLAG_INPUT 0x80
#define SCSI_CMD_FLAG_OUTPUT 0x00
#define SCSI_CMD_STATUS_SIGNATURE 0x53425355
#define SCSI_STATUS_FAILED 1
#define SCSI_STATUS_PHASE_ERR 2

#pragma pack(push,1)
typedef struct _scsi_cmd_ {
	uint32_t signature;
	uint32_t tag;
	uint32_t dataBytes;
	uint8_t flags;
	uint8_t lun;
	uint8_t cmdBytes;
	uint8_t cmd[16];
}SCSICommand;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _scsi_status_ {
	uint32_t signature;
	uint32_t tag;
	uint32_t residue;
	uint8_t status;
}SCSIStatus;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _scsi_inquiry_resp_ {
	uint8_t preipheralDeviceType;
	uint8_t removalMedia;
	uint8_t version;
	uint8_t responseDataFormat;
	uint8_t additionalLength;
	uint8_t reserved[3];
	char vendorID[8];
	char productID[16];
	char productRev[4];
}SCSIInquiryResponse;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _scsi_capacity_resp_ {
	uint32_t lbaCount;
	uint32_t blockSize;
}SCSICapacityResponse;
#pragma pack(pop)

/*
* AuUSBDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload(AuUSBDeviceStruc* dev) {

	return 0;
}

/*
 * AuUSBMSCSendCommand -- send a command to MSC device
 * @param dev -- Pointer to USB device struc
 * @param bulkIn -- Bulk IN Endpoint
 * @param bulkOut -- Bulk OUT Endpoint
 * @param cbw -- SCSI Command to submit
 * @param resp -- Response buffer
 * @param respSize -- Response size
 */
void AuUSBMSCSendCommand(AuUSBDeviceStruc *dev, void* bulkIn, void* bulkOut, SCSICommand* cbw, void* resp, uint32_t respSize) {
	dev->AuBulkTranfer(dev, (uint64_t)cbw, sizeof(SCSICommand), bulkOut);
	dev->AuUSBWait(dev, USB_WAIT_EVENT_TRANSFER);

	if (respSize > 0) {
		dev->AuBulkTranfer(dev, (uint64_t)resp, respSize, bulkIn);
		dev->AuUSBWait(dev, USB_WAIT_EVENT_TRANSFER);
	}
	SCSIStatus* csw = (SCSIStatus*)P2V((uint64_t)AuPmmngrAlloc());
	memset(csw, 0, sizeof(SCSIStatus));
	dev->AuBulkTranfer(dev, V2P((uint64_t)csw), sizeof(SCSIStatus), bulkIn);
	dev->AuUSBWait(dev, USB_WAIT_EVENT_TRANSFER);

	if (csw->signature == SCSI_CMD_STATUS_SIGNATURE) {
		SeTextOut("USB MSC -- Command submitted successfully \r\n");
	}
	AuPmmngrFree((void*)V2P((uint64_t)csw));
}
/*
* AuUSBDriverMain -- Main entry for USB driver 
*/
AU_EXTERN AU_EXPORT int AuUSBDriverMain(AuUSBDeviceStruc* dev) {
	if (dev->classCode == 0x08 && dev->subClassCode == 0x6 && dev->protocol == 0x50) {
		AuTextOut("USB-MSC class attached \n");
	}

	uint8_t mscInterfaceNumber = 0;
	AuUSBConfigDesc* config = (AuUSBConfigDesc*)dev->descriptor;
	AuUSBInterfaceDesc* interface_desc = raw_offset<AuUSBInterfaceDesc*>(config, config->bLength);

	/* 
	 * Extract the interface number for Mass Storage class 
	 * it will reside in Interface Class 0x08
	 */
	AuUSBInterfaceDesc* iface = interface_desc;
	for (int i = 0; i < config->bNumInterfaces; i++) {
		if (iface->bInterfaceClass == 0x08) {
			mscInterfaceNumber = iface->bInterfaceNumber;
		}
		iface = raw_offset<AuUSBInterfaceDesc*>(iface, iface->bLength);
	}

	AuUSBEndpointDesc* endp = raw_offset<AuUSBEndpointDesc*>(interface_desc, interface_desc->bLength);
	while (raw_diff(endp, config) < config->wTotalLength) {
		uint8_t endpointAddr = endp->bEndpointAddress;
		uint8_t attrib = endp->bmAttributes;
		if ((attrib & 0x03) == 0x02) { //Bulk transfer
			if (endpointAddr & 0x80) {
				AuTextOut("Bulk IN/ endpoint found \n");
				AuTextOut("Bulk IN max pack -> %d \n", (endp->wMaxPacketSize & 0x7FFF));
			}
			else {
				AuTextOut("Bulk OUT/ endpoint found \n");
				AuTextOut("BULK OUT MaxPACk -> %d \n", (endp->wMaxPacketSize & 0x7FFF));

			}
		}

		endp = raw_offset<AuUSBEndpointDesc*>(endp, endp->bLength);
	}

	void* ep_bulk_in = dev->AuGetBulkEndpoint(dev, 1);
	if (ep_bulk_in) {
		AuTextOut("USB MSC Bulk-in ep found \n");
	}

	void* ep_bulk_out = dev->AuGetBulkEndpoint(dev, 0);
	if (ep_bulk_out)
		AuTextOut("USB MSC Bulk-out ep found \n");

	/* Reset the USB MSC Device */
	AuUSBRequestPacket pack;
	pack.request_type = 0x21;
	pack.request = 0xFF;
	pack.value = 0x0000;
	pack.index = mscInterfaceNumber;
	pack.length = 0x0000;
	dev->AuControlTransfer(dev, &pack, 0, 0);
	int t_idx = -1;
	t_idx = dev->AuUSBWait(dev, USB_WAIT_EVENT_TRANSFER);

	SCSICommand* cbw = (SCSICommand*)P2V((uint64_t)AuPmmngrAlloc());
	SCSIInquiryResponse* resp = (SCSIInquiryResponse*)P2V((uint64_t)AuPmmngrAlloc());
	memset(resp, 0, sizeof(SCSIInquiryResponse));
	memset(cbw, 0, sizeof(SCSICommand));

	cbw->signature = SCSI_CMD_BLK_SIGNATURE;
	cbw->tag = 0x12345678;
	cbw->dataBytes = sizeof(SCSIInquiryResponse);
	cbw->flags = SCSI_CMD_FLAG_INPUT;
	cbw->lun = 0;
	cbw->cmdBytes = 6;
	cbw->cmd[0] = 0x12;
	cbw->cmd[4] = sizeof(SCSIInquiryResponse);

	AuUSBMSCSendCommand(dev, ep_bulk_in, ep_bulk_out, (SCSICommand*)V2P((uint64_t)cbw), 
		(void*)V2P((uint64_t)resp), sizeof(SCSIInquiryResponse));
	AuTextOut("\nUSB Mass Storage device : %s \n", resp->vendorID);
	AuPmmngrFree((void*)V2P((uint64_t)resp));
	AuPmmngrFree((void*)V2P((uint64_t)cbw));
	AuTextOut("USB MSC driver initialised %d \n", t_idx);
	
	return 0;
}