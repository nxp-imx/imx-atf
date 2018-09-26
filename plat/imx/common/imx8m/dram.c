/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>
#include <spinlock.h>

static struct dram_info dram_info;

/* lock used for DDR DVFS */
spinlock_t dfs_lock;
/* IRQ used for DDR DVFS */
static uint32_t irqs_used[] = {74, 75, 76, 77};
static volatile uint32_t wfe_done;
static volatile bool wait_ddrc_hwffc_done = true;

static unsigned int dev_fsp = 0x1;

/* restore the ddrc config */
void dram_umctl2_init(void)
{
	struct dram_timing_info *timing = dram_info.timing_info;
	struct dram_cfg_param *ddrc_cfg = timing->ddrc_cfg;
	int num = timing->ddrc_cfg_num;

	for (int i = 0; i < num; i++) {
		mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}

	/* set the default fsp to P0 */
	mmio_write_32(DDRC_MSTR2(0), 0x0);
}

/* resotre the dram phy config */
void dram_phy_init(void)
{
	struct dram_timing_info *timing = dram_info.timing_info;
	struct dram_cfg_param *ddrphy_cfg = timing->ddrphy_cfg;
	int num = timing->ddrphy_cfg_num;

	/* restore the phy init config */
	for (int i = 0; i < num; i++) {
		dwc_ddrphy_apb_wr(ddrphy_cfg->reg, ddrphy_cfg->val);
		ddrphy_cfg++;
	}

	/* restore the ddr phy csr */
	num = timing->ddrphy_trained_csr_num;
	ddrphy_cfg = timing->ddrphy_trained_csr;
	for (int i = 0; i < num; i++) {
		dwc_ddrphy_apb_wr(ddrphy_cfg->reg, ddrphy_cfg->val);
		ddrphy_cfg++;
	}

	/* load the PIE image */
	num = timing->ddrphy_pie_num;
	ddrphy_cfg = timing->ddrphy_pie;
	for (int i = 0; i < num; i++) {
		dwc_ddrphy_apb_wr(ddrphy_cfg->reg, ddrphy_cfg->val);
		ddrphy_cfg++;
	}
}

void dram_info_init(unsigned long dram_timing_base)
{
	uint32_t current_fsp, ddr_type;

	/* get the dram type */
	ddr_type = mmio_read_32(DDRC_MSTR(0)) & DDR_TYPE_MASK;
	dram_info.dram_type = ddr_type;

	/* init the boot_fsp & current_fsp */
	current_fsp = mmio_read_32(DDRC_DFIMISC(0));
	current_fsp = (current_fsp >> 8) & 0xf;
	dram_info.boot_fsp = current_fsp;
	dram_info.current_fsp = current_fsp;

	/*
	 * No need to do save for ddrc and phy config register,
	 * we have done it in SPL stage and save in memory
	 */
	dram_info.timing_info = (struct dram_timing_info *)dram_timing_base;

	/* switch to the highest frequency point */
	if(ddr_type == DDRC_LPDDR4 && current_fsp != 0x0) {
		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);
		lpddr4_swffc(dev_fsp, 0x0);
		dev_fsp = (~dev_fsp) & 0x1;
	}
}

void dram_enter_retention(void)
{
	if (dram_info.dram_type == DDRC_LPDDR4)
		lpddr4_enter_retention();
	else if (dram_info.dram_type == DDRC_DDR4)
		ddr4_enter_retention();
}

void dram_exit_retention(void)
{
	if (dram_info.dram_type == DDRC_LPDDR4)
		lpddr4_exit_retention();
	else if (dram_info.dram_type == DDRC_DDR4)
		ddr4_exit_retention();
}

int dram_dvfs_handler(uint32_t smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
	unsigned int target_freq = x1;
	uint32_t online_cores = x2;

	/* TODO add ddr4 dvfs support later */
	if (dram_info.dram_type != DDRC_LPDDR4)
		return 0;

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
			if (online_cores == wfe_done)
				break;
		}

		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);

		lpddr4_swffc(dev_fsp, target_freq);
		dev_fsp = (~dev_fsp) & 0x1;

		wait_ddrc_hwffc_done = false;
		wfe_done = 0;
		dsb();
		sev();
		isb();
	}

	return 0;
}
