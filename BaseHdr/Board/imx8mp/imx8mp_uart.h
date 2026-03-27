/**
* @file imx8mp_uart.h
*
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

#ifndef __IMX8MP_UART_H__
#define __IMX8MP_UART_H__

/**
 * UART Register Map for NXP.IMX8MP SoC family, can be found in
 * iMX8MPRM -- Processor Reference Manual
 */

#define IMX8MP_UART1_BASE_ADDRESS 0x30860000

#define IMX8MP_UART1_URXD(base)   base  /* UART Receiver Register (URDX) -- 32 bit width*/
#define IMX8MP_UART1_UTXD(base)   (base + 0x40) /** UART Transmitter Register (UTXD) -- 32 bit width */
#define IMX8MP_UART1_UCR1(base)   (base + 0x80) /** UART1 Control register */
#define IMX8MP_UART1_UCR2(base)   (base + 0x84)
#define IMX8MP_UART1_UCR3(base)   (base + 0x88)
#define IMX8MP_UART1_UCR4(base)   (base + 0x8C)
#define IMX8MP_UART1_UFCR(base)   (base + 0x90) /** UART1 FIFO Control Register **/
#define IMX8MP_UART1_USR1(base)   (base + 0x94) /** UART1 Status Regster 1 **/
#define IMX8MP_UART1_USR2(base)   (base + 0x98) /** UART1  Status Register 2 **/
#define IMX8MP_UART1_UESC(base)   (base + 0x9C) /** UART1 Escape Character Register **/
#define IMX8MP_UART1_UTIM(base)   (base + 0xA0) /** UART1 Escape Timer Register **/
#define IMX8MP_UART1_UBIR(base)   (base + 0xA4) /** UART1 BRM Incremental Register **/
#define IMX8MP_UART1_UBMR(base)   (base + 0xA8) /** UART1 BRM Modulator Register **/
#define IMX8MP_UART1_UBRC(base)   (base + 0xAC) /** UART1 Baude Rate Count Register **/
#define IMX8MP_UART1_ONEMS(base)  (base + 0xB0) /** UART1 One Millisecond Register **/
#define IMX8MP_UART1_UTS(base)    (base + 0xB4) /** UART1 Test Register **/
#define IMX8MP_UART1_UMCR(base)   (base + 0xB8) /** UART1 RS-485 Mode Control Register **/

#define IMX8MP_UART3_BASE_ADDRESS 0x30880000

#define IMX8MP_UART3_URXD(base)  (base) /** UART3 Receiver Register (URDX) -- 32 bit width **/
#define IMX8MP_UART3_UTXD(base)  (base + 0x40) /** UART3 Transmitter Register (UTXD) -- 32 bit width **/
#define IMX8MP_UART3_UCR1(base)  (base + 0x80) /** UART3 Control Register **/
#define IMX8MP_UART3_UCR2(base)  (base + 0x84) 
#define IMX8MP_UART3_UCR3(base)   (base + 0x88)
#define IMX8MP_UART3_UCR4(base)   (base + 0x8C)
#define IMX8MP_UART3_UFCR(base)   (base + 0x90) /** UART3 FIFO Control Register **/
#define IMX8MP_UART3_USR1(base)   (base + 0x94) /** UART3 Status Regster 1 **/
#define IMX8MP_UART3_USR2(base)   (base + 0x98) /** UART3  Status Register 2 **/
#define IMX8MP_UART3_UESC(base)   (base + 0x9C) /** UART3 Escape Character Register **/
#define IMX8MP_UART3_UTIM(base)   (base + 0xA0) /** UART3 Escape Timer Register **/
#define IMX8MP_UART3_UBIR(base)   (base + 0xA4) /** UART3 BRM Incremental Register **/
#define IMX8MP_UART3_UBMR(base)   (base + 0xA8) /** UART3 BRM Modulator Register **/
#define IMX8MP_UART3_UBRC(base)   (base + 0xAC) /** UART3 Baude Rate Count Register **/
#define IMX8MP_UART3_ONEMS(base)  (base + 0xB0) /** UART3 One Millisecond Register **/
#define IMX8MP_UART3_UTS(base)    (base + 0xB4) /** UART3 Test Register **/
#define IMX8MP_UART3_UMCR(base)   (base + 0xB8) /** UART3 RS-485 Mode Control Register **/

#define IMX8MP_UART2_BASE_ADDRESS 0x30890000

#define IMX8MP_UART2_URXD(base)  (base) /** UART2 Receiver Register (URDX) -- 32 bit width **/
#define IMX8MP_UART2_UTXD(base)  (base + 0x40) /** UART2 Transmitter Register (UTXD) -- 32 bit width **/
#define IMX8MP_UART2_UCR1(base)  (base + 0x80) /** UART2 Control Register **/
#define IMX8MP_UART2_UCR2(base)  (base + 0x84) 
#define IMX8MP_UART2_UCR3(base)   (base + 0x88)
#define IMX8MP_UART2_UCR4(base)   (base + 0x8C)
#define IMX8MP_UART2_UFCR(base)   (base + 0x90) /** UART2 FIFO Control Register **/
#define IMX8MP_UART2_USR1(base)   (base + 0x94) /** UART2 Status Regster 1 **/
#define IMX8MP_UART2_USR2(base)   (base + 0x98) /** UART2  Status Register 2 **/
#define IMX8MP_UART2_UESC(base)   (base + 0x9C) /** UART2 Escape Character Register **/
#define IMX8MP_UART2_UTIM(base)   (base + 0xA0) /** UART2 Escape Timer Register **/
#define IMX8MP_UART2_UBIR(base)   (base + 0xA4) /** UART2 BRM Incremental Register **/
#define IMX8MP_UART2_UBMR(base)   (base + 0xA8) /** UART2 BRM Modulator Register **/
#define IMX8MP_UART2_UBRC(base)   (base + 0xAC) /** UART2 Baude Rate Count Register **/
#define IMX8MP_UART2_ONEMS(base)  (base + 0xB0) /** UART2 One Millisecond Register **/
#define IMX8MP_UART2_UTS(base)    (base + 0xB4) /** UART2 Test Register **/
#define IMX8MP_UART2_UMCR(base)   (base + 0xB8) /** UART2 RS-485 Mode Control Register **/


#endif
