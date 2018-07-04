/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>

static struct dram_info *dram_info;

void dram_cfg_save(struct dram_cfg_param *params, int params_num)
{
	for (int idx = 0;  idx < params_num; idx++) {
		params->cfg = mmio_read_32(params->reg);
		params++;
	}
}

void dram_cfg_restore(struct dram_cfg_param *params, int params_num)
{
	for (int idx = 0; idx < params_num; idx++) {
		mmio_write_32(params->reg, params->cfg);
		params++;
	}
}

/* restore the ddrc config */
void dram_umctl2_init(void)
{
	struct dram_cfg_param *params = dram_info->ddrc_cfg;
	int params_num = dram_info->ddrc_cfg_num;

	dram_cfg_restore(params, params_num);
	mmio_write_32(DDRC_DBG1(0), 0x1);
	mmio_write_32(DDRC_DFIMISC(0), 0x11);
	mmio_write_32(DDRC_DFIUPD0(0),0xE0400018); 
}

/* resotre the dram phy config */
void dram_phy_init(void)
{
	struct dram_cfg_param *ddrphy_params = dram_info->ddrphy_cfg;
	int params_num = dram_info->ddrphy_cfg_num;

	/* retore the phy config */
	mmio_write_32(DDRPHY_REG(0xd0000), 0x0);
	dram_cfg_restore(ddrphy_params, params_num);
	mmio_write_32(DDRPHY_REG(0xd0000), 0x1);

	/* load the PIE image */
	ddrphy_load_pie_image();
}

void dram_info_init(void)
{
	uint32_t current_fsp, ddr_type;

	/* get the dram type */
	ddr_type = mmio_read_32(DDRC_MSTR(0)) & DDR_TYPE_MASK;

	if (ddr_type == DDRC_LPDDR4) {
		dram_info = &imx8m_lpddr4_dram_info;
		dram_info->dram_type = ddr_type;
	} else {
		/* TODO DDR4 support will be added later */
		return;
	}

	/* init the boot_fsp & current_fsp */
	current_fsp = mmio_read_32(DDRC_DFIMISC(0));
	current_fsp = (current_fsp >> 8) & 0xf;
	dram_info->boot_fsp = current_fsp;
	dram_info->current_fsp = current_fsp;

	/* save the ddrc config */
	dram_cfg_save(dram_info->ddrc_cfg, dram_info->ddrc_cfg_num);

	/* save dram phy config */
	mmio_write_32(DDRPHY_REG(0xd0000), 0x0);
	mmio_write_32(DDRPHY_REG(0xc0080), 0x3);
	dram_cfg_save(dram_info->ddrphy_cfg, dram_info->ddrphy_cfg_num);
	mmio_write_32(DDRPHY_REG(0xc0080), 0x0);
	mmio_write_32(DDRPHY_REG(0xd0000), 0x1);
}

void dram_enter_retention(void)
{
	/* TODO add the ddr4 support in the furture */
	if (dram_info->dram_type == DDRC_LPDDR4)
		lpddr4_enter_retention();
}

void dram_exit_retention(void)
{
	/* TODO add the ddr4 support in the furture */
	if (dram_info->dram_type == DDRC_LPDDR4)
		lpddr4_exit_retention();
}
