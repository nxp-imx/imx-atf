/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <gicv3.h>
#include <plat_imx8.h>
#include <platform.h>
#include <platform_def.h>

/* the GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* array of Group1 secure interrupts to be configured by the gic driver */
const unsigned int g1s_interrupt_array[] = {
	6
};

/* array of Group0 interrupts to be configured by the gic driver */
const unsigned int g0_interrupt_array[] = {
	7
};

const gicv3_driver_data_t arm_gic_data = {
	.gicd_base = PLAT_GICD_BASE,
	.gicr_base = PLAT_GICR_BASE,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_calc_core_pos,
};

void plat_gic_driver_init(void)
{
	/*
	 * the GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#if IMAGE_BL31
	gicv3_driver_init(&arm_gic_data);
#endif
}

void plat_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void plat_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void plat_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

void plat_gic_save(unsigned int proc_num, struct plat_gic_ctx *ctx)
{
	/* save the gic rdist/dist context */
	gicv3_rdistif_save(proc_num, &ctx->rdist_ctx);
	gicv3_distif_save(&ctx->dist_ctx);
}

void plat_gic_restore(unsigned int proc_num, struct plat_gic_ctx *ctx)
{
	/* restore the gic rdist/dist context */
	gicv3_rdistif_init_restore(proc_num, &ctx->rdist_ctx);
	gicv3_distif_init_restore(&ctx->dist_ctx);
}
