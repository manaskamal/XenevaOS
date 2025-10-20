/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#include <pcie.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/gic.h>
#include <Fs/Dev/devinput.h>
#include <Drivers/virtio.h>
#include <Drivers/uart.h>
#include <Mm/pmmngr.h>
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/sched.h>

/*
 * AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
void AuVirtioTabletInitialize() {
	int bus = 0;
	int func = 0;
	int dev = 0;
	uint64_t device = AuPCIEScanVendorDevice(0x1AF4, 0x1052, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	AuTextOut("[aurora]: virtio-tablet device found \n");
}