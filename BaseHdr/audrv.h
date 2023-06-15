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

#ifndef __AU_DRV_H__
#define __AU_DRV_H__

#include <stdint.h>
#include <Hal\x86_64_hal.h>
#include <Hal\hal.h>
#include <Fs\vfs.h>

#define DRIVER_CLASS_AUDIO 1
#define DRIVER_CLASS_VIDEO 2
#define DRIVER_CLASS_NETWORK 3
#define DRIVER_CLASS_CONNECTIVITY_BLUETOOTH 4
#define DRIVER_CLASS_CONNECTIVITY_WIFI 5
#define DRIVER_CLASS_STORAGE  6
#define DRIVER_CLASS_USB 7
#define DRIVER_CLASS_HID 8
#define DRIVER_CLASS_UNKNOWN 9

#define DEVICE_CLASS_ETHERNET 1
#define DEVICE_CLASS_HD_AUDIO 2
#define DEVICE_CLASS_USB3     3

#define AURORA_MAX_DRIVERS  256

typedef int(*au_drv_entry)();
typedef int(*au_drv_unload)();

typedef struct _aurora_driver_ {
	uint8_t id;
	uint8_t class_type;
	char name[32];
	bool present;
	uint64_t base;
	uint64_t end;
	au_drv_entry entry;
	au_drv_unload unload;
}AuDriver;

typedef struct _aurora_device_ {
	uint16_t classCode;
	uint16_t subClassCode;
	uint8_t progIf;
	bool initialized;
	uint8_t aurora_dev_class;
	uint8_t aurora_driver_class;
}AuDevice;


/*
* AuDrvMngrInitialize -- Initialize the driver manager
* @param info -- kernel boot info
*/
extern void AuDrvMngrInitialize(KERNEL_BOOT_INFO *info);





#endif