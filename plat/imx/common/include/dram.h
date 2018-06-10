/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRAM_H__
#define __DRAM_H__

#include <utils_def.h>

#define DDRC_LPDDR4	BIT(5)
#define DDR_TYPE_MASK	0x3f

#define DDRPHY_REG(x)	(0x3c000000 + 4*x)

/* reg & config param */
struct dram_cfg_param {
	unsigned int reg;
	unsigned int cfg;
};

struct dram_info {
	int dram_type;
	int current_fsp;
	int boot_fsp;
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	struct dram_cfg_param *ddrphy_cfg;
	unsigned int ddrphy_cfg_num;
};

/* lpddr4 ddrc and phy config info */
extern struct dram_info imx8m_lpddr4_dram_info;

void ddrphy_load_pie_image(void);
void dram_info_init(void);
void lpddr4_enter_retention(void);
void lpddr4_exit_retention(void);
void dram_umctl2_init(void);
void dram_phy_init(void);
void dram_enter_retention(void);
void dram_exit_retention(void);

#endif /* __DRAM_H__ */
