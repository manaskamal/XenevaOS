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

#include <Hal/AA64/rpi3.h>
#include <Board/RPI3bp/rpi3bp.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <aucon.h>


static uint64_t* gpioBase;

#define GPFSEL0  0x00
#define GPFSEL1  0x04
#define GPFSEL2  0x08
#define GPSET0   0x1C
#define GPCLR0   0x28

/*
 * AuRPI3BPGpioMap -- map the GPIO base to kernel
 * highrr half address
 */
void AuRPI3BPGpioMap() {
	gpioBase = (uint64_t*)AuMapMMIO(GPIO, 1);
	AuTextOut("[aurora]: RPI3 GPIO mapped to %x address \r\n", gpioBase);
}

/*
 * AuRPIGPIOSetFunction -- assign function to 
 * given pin
 * @param pin -- pin number 
 * @param function -- function type
 */
void AuRPIGPIOSetFunction(uint8_t pin, uint8_t function) {
	uint32_t reg = pin / 10;
	uint32_t shift = (pin % 10) * 3;
	uint64_t addr = (uint64_t)gpioBase + GPFSEL0 + (reg * 4);
	uint32_t val = *(volatile uint32_t*)addr; 
	val &= ~(7 << shift);
	val |= (function << shift);
	*(volatile uint32_t*)addr = val;
	dsb_ish();
	isb_flush();
}

/*
 * AuRPIGPIOSet -- set a specific pin
 * @param pin -- Pin number
 */
void AuRPIGPIOSet(uint8_t pin) {
	*(volatile uint32_t*)((uint64_t)gpioBase + GPSET0) = (1 << pin);
	dsb_ish();
	isb_flush();
}

/*
 * AuRPIGPIOClear -- clear a pin
 * @param pin -- Pin number to clear
 */
void AuRPIGPIOClear(uint8_t pin) {
	*(volatile uint32_t*)((uint64_t)gpioBase + GPCLR0) = (1 << pin);
	dsb_ish();
	isb_flush();
}

/*
 * AuRPIGPIOWrite -- write boolean value
 * to specific pin
 * @param pin -- Pin number
 * @param value -- value to write
 */
void AuRPIGPIOWrite(uint8_t pin, bool value) {
	if (value)
		AuRPIGPIOSet(pin);
	else
		AuRPIGPIOClear(pin);
}



