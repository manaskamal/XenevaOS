/**
* @file board.c
*
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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
#include <Board/RPI3bp/rpi3bp.h>
#include <Drivers/uart.h>

#include <aucon.h>

/**
 * @brief AuAA64BoardInitialize -- initialize board specific data
 */
void AuAA64BoardInitialize() {
	AuTextOut("[aurora]: initializing board data \r\n");
#ifdef __TARGET_BOARD_RPI3__
	AuRPI3Initialize();
#endif
}

/**
 * @brief AuAA64BoardSleepUS -- calls board specific sleep function
 */
void AuAA64BoardSleepUS(uint32_t us) {
#ifdef __TARGET_BOARD_RPI3__
	AuRPI3DelayUS(us);
#endif
}

/**
 * @brief AuAA64BoardSleepMS -- calls board specific sleep function
 * in ms
 */
void AuAA64BoardSleepMS(uint32_t ms) {
#ifdef __TARGET_BOARD_RPI3__
	AuRPIDelayMS(ms);
#endif
}