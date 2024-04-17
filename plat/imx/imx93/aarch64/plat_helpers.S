/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_a55.h>

#include <platform_def.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl	platform_mem_init
	.globl  plat_reset_handler

	/* ------------------------------------------------------
	 * Helper macro that reads the part number of the current
	 * CPU and jumps to the given label if it matches the CPU
	 * MIDR provided.
	 *
	 * Clobbers x0.
	 * ------------------------------------------------------
	 */
	.macro  jump_if_cpu_midr _cpu_midr, _label

	mrs	x0, midr_el1
	ubfx	x0, x0, MIDR_PN_SHIFT, #12
	cmp     w0, #((\_cpu_midr >> MIDR_PN_SHIFT) & MIDR_PN_MASK)
	b.eq	\_label

	.endm

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	mov_imm x1, MPIDR_AFFINITY_MASK
	and	x0, x0, x1
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ----------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 * This function uses the plat_calc_core_pos()
	 * to get the index of the calling CPU.
	 * ----------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_my_core_pos

	/*
	 * unsigned int plat_calc_core_pos(uint64_t mpidr)
	 * helper function to calculate the core position.
	 * With this function.
	 */
func plat_calc_core_pos
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_calc_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_reset_handler
#if defined(PLAT_imx91)
	/* set L3PCTL 4 lines to improve 91 performance */
	mrs x0, CORTEX_A55_CPUECTLR_EL1
	bic x0, x0, #0x1C00
	orr x0, x0, #0x1800
	msr CORTEX_A55_CPUECTLR_EL1, x0
	isb
#endif
	/* check if IRQ22 is pending in GPC */
	mov x0, #0x44470000
	ldr w1, [x0, #0x968]
	tbnz w1, #28, suspend
	ret
suspend:
	/* clear the s401 halt_ack req */
	mov x0, #0x444f0000
	mov w1, #0x100
	str w1, [x0, #0x134]
	nop
	nop
	str wzr, [x0, #0x134]

	/* enable core power down */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb

	/* enable cluster power down */
	mrs	x0, CLUSTERPWRDN_EL1
	orr	x0, x0, #DSU_CLUSTER_PWR_OFF | BIT(1)
	msr	CLUSTERPWRDN_EL1, x0
	isb

	wfi
endfunc plat_reset_handler
