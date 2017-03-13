/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
 * Neither the name of ARM nor the names of its contributors may be used
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

#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl	plat_reset_handler
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	platform_mem_init
	.globl  imx_mailbox_init

	/* ----------------------------------------------
	 * The mailbox_base is used to distinguish warm/cold
	 * reset. The mailbox_base is in the data section, not
	 * in .bss, this allows function to start using this
	 * variable before the runtime memory is initialized.
	 * ----------------------------------------------
	 */
.section .data.mailbox_base ; .align 3
	mailbox_base: .quad 0x0

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This Function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

	/*
	 * unsigned int plat_calc_core_pos(uint64_t mpidr)
	 * helper function to calculate the core position.
	 * With this function.
	 */
func plat_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and 	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_calc_core_pos


	/* ----------------------------------------------
	 * function to handle platform specific reset.
	 * ----------------------------------------------
	 */
func plat_reset_handler
	/* enable EL2 cpuectlr RW access */
	mov	x0, #0x73
	msr	actlr_el3, x0
	msr	actlr_el2, x0
	isb
	ret
endfunc plat_reset_handler

	/* ---------------------------------------------
	 * function to get the entrypoint.
	 * ---------------------------------------------
	 */
func plat_get_my_entrypoint
	adrp	x1, mailbox_base
	ldr	x0, [x1, :lo12:mailbox_base]
	ret
endfunc	plat_get_my_entrypoint

func plat_secondary_cold_boot_setup
	b	.
endfunc plat_secondary_cold_boot_setup

func plat_crash_console_init
	/* TODO */
	ret
endfunc plat_crash_console_init

func plat_crash_console_putc
	/* TODO */
	ret
endfunc plat_crash_console_putc

func platform_mem_init
	ret
endfunc platform_mem_init

/* Init the mailbox base address */
func imx_mailbox_init
	adrp	x1, mailbox_base
	str	x0, [x1, :lo12:mailbox_base]
	ret
endfunc imx_mailbox_init
