/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <ddrc.h>
#include <mmio.h>
#include <platform_def.h>
#include <spinlock.h>
#include <soc.h>

#include "lpddr4_dvfs.h"

#define DDRC_LPDDR4	(1 << 5)
#define DDR_TYPE_MASK	0x3f

/* lock used for DDR DVFS */
spinlock_t dfs_lock;
/* IRQ used for DDR DVFS */
static uint32_t irqs_used [] = {102, 109, 110, 111};
static volatile uint32_t wfe_done;
static volatile bool wait_ddrc_hwffc_done = true;

static unsigned int init_fsp = 0x1;

static inline int get_ddr_type(void)
{
	return mmio_read_32(IMX_DDRC_BASE + DDRC_MSTR(0)) & DDR_TYPE_MASK;
}

void lpddr4_switch_to_3200(void)
{
	if (get_ddr_type() == DDRC_LPDDR4)
		lpddr4_dvfs_swffc(init_fsp, 0x0);
}

/*
 * x1: for DDR frequency target info
 * x2: for online cpu info.
 */
int lpddr4_dvfs_handler(uint32_t smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
	unsigned int target_freq = x1;
	uint32_t online_cores = x2;

	if (target_freq == 0xf) {
		/* set the WFE done status */
		spin_lock(&dfs_lock);
		wfe_done |= (1 << cpu_id * 8);
		spin_unlock(&dfs_lock);

		while (1) {
			/* ddr frequency change done */
			wfe();
			if (!wait_ddrc_hwffc_done) {
				break;
			}
		}
	} else {
		wait_ddrc_hwffc_done = true;
		/* trigger the IRQ */
		for (int i = 0; i < 4; i++) {
			int irq = irqs_used[i] % 32;
			if (cpu_id != i && (online_cores & (0x1 << (i * 8)))) {
				mmio_write_32(0x38800204 + (irqs_used[i] / 32) * 4, (1 << irq));
			}
		}

		/* make sure all the core in WFE */
		online_cores &= ~(0x1 << (cpu_id * 8));
		while (1) {
			mmio_write_32(0x30340004, mmio_read_32(0x30340004) | (1 << 12));
			if (online_cores == wfe_done)
				break;
		}

		mmio_write_32(0x30340004, mmio_read_32(0x30340004) & ~(1 << 12));

		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);

		/*
		 * for DDR dvfs, we have two method: hwffc or swffc,
		 * the default method we used on i.MX8MQ is swffc.
		 */
#if LPDDR4_DVFS_HWFFC
		lpddr4_dvfs_hwffc(init_vrcg, init_fsp, target_freq, discamdrain);
		init_fsp = (~init_fsp) & 0x1;
#else
		lpddr4_dvfs_swffc(init_fsp, target_freq);
#endif
		wait_ddrc_hwffc_done = false;
		wfe_done = 0;
		dsb();
		sev();
		isb();
	}

	return 0;
}



