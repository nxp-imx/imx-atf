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
	unsigned int val;
};

struct dram_timing_info {
	/* umctl2 config */
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	/* ddrphy config */
	struct dram_cfg_param *ddrphy_cfg;
	unsigned int ddrphy_cfg_num;
	/* ddr fsp train info */
	struct dram_fsp_msg *fsp_msg;
	unsigned int fsp_msg_num;
	/* ddr phy trained CSR */
	struct dram_cfg_param *ddrphy_trained_csr;
	unsigned int ddrphy_trained_csr_num;
	/* ddr phy PIE */
	struct dram_cfg_param *ddrphy_pie;
	unsigned int ddrphy_pie_num;
};

struct dram_info {
	int dram_type;
	int current_fsp;
	int boot_fsp;
	struct dram_timing_info *timing_info;
};

void ddrphy_load_pie_image(void);
void dram_info_init(unsigned long dram_timing_base);
void lpddr4_enter_retention(void);
void lpddr4_exit_retention(void);
void dram_umctl2_init(void);
void dram_phy_init(void);
void dram_enter_retention(void);
void dram_exit_retention(void);

#endif /* __DRAM_H__ */
