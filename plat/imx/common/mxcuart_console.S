/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <assert_macros.S>
#include "uart.h"

	.globl	console_uart_register
	.globl	console_uart_init
	.globl	console_uart_putc
	.globl	console_uart_getc

func console_uart_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_DRVDATA]

	bl	console_uart_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register uart

register_fail:
	ret	x7
endfunc console_uart_register

func console_uart_init
	mov	w0, #1
	ret
endfunc console_uart_init

func console_uart_putc
	ldr	x1, [x1, #CONSOLE_T_DRVDATA]
	cbz	x1, putc_error

	/* Prepare '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UTS]
	tbz	w2, #6, 1b
	mov	w2, #0xD
	str	w2, [x1, #UTXD]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UTS]
	tbz	w2, #6, 2b
	str	w0, [x1, #UTXD]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_uart_putc

func console_uart_getc
	ldr	x0, [x0, #CONSOLE_T_DRVDATA]
	cbz	x0, getc_error
1:
	ldr	w1, [x0, #UTS]
	tbnz	w1, #5, 1b

	ldr	w1, [x0, #URXD]
	and	w0, w1, #URXD_RX_DATA

	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_uart_getc
