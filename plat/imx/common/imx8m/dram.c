/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>

static struct dram_info dram_info;

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

	if (ddr_type == DDRC_LPDDR4) {
		dram_info.dram_type = ddr_type;
	} else {
		/* TODO DDR4 support will be added later */
		return;
	}

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
}

void dram_enter_retention(void)
{
	/* TODO add the ddr4 support in the furture */
	if (dram_info.dram_type == DDRC_LPDDR4)
		lpddr4_enter_retention();
}

void dram_exit_retention(void)
{
	/* TODO add the ddr4 support in the furture */
	if (dram_info.dram_type == DDRC_LPDDR4)
		lpddr4_exit_retention();
}
