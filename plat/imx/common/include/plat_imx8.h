/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_IMX8_H__
#define __PLAT_IMX8_H__

#include <gicv3.h>

/* gicv3 context save */
struct plat_gic_ctx {
	gicv3_redist_ctx_t rdist_ctx;
	gicv3_dist_ctx_t dist_ctx;
};

unsigned int plat_calc_core_pos(u_register_t mpidr);
void imx_mailbox_init(uintptr_t base_addr);
void plat_gic_driver_init(void);
void plat_gic_init(void);
void plat_gic_cpuif_enable(void);
void plat_gic_cpuif_disable(void);
void plat_gic_pcpu_init(void);
void plat_gic_save(unsigned int proc_num, struct plat_gic_ctx *ctx);
void plat_gic_restore(unsigned int proc_num, struct plat_gic_ctx *ctx);

#endif /*__PLAT_IMX8_H__ */
