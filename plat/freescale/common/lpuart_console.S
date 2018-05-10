/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

#define UARTBAUD	0x10
#define UARTSTAT	0x14
#define UARTCTRL	0x18
#define UARTDATA	0x1C
#define UARTMATCH	0x20
#define UARTMODIR	0x24
#define UARTFIFO	0x28
#define UARTWATER	0x2c

#define US1_TDRE	(1 << 23)
#define US1_RDRF	(1 << 21)

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc
	.globl	console_core_flush

/* console_core_init */
func console_core_init
	mov	w0, wzr
	ret
endfunc console_core_init

/* console_core_putc */
func console_core_putc
	/* Check the input parameter */
	cbz	x1, putc_error
	/* Insert implementation here */
	/* Prepare '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UARTSTAT]
	tbz	w2, #23, 1b
	mov	w2, #0xD
	str	w2, [x1, #UARTDATA]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UARTSTAT]
	tbz	w2, #23, 2b
	str	w0, [x1, #UARTDATA]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_core_putc

/* console_core_getc */
func console_core_getc
	cbz	x0, getc_error
	/* Insert implementation here */
	/* Check if the receive FIFO state */
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc

/* console_core_flush */
func console_core_flush
	mov	w0, wzr
	ret
endfunc console_core_flush
