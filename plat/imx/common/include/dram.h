/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRAM_H__
#define __DRAM_H__

#include <utils_def.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <stdbool.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_imx8.h>
#include <xlat_tables.h>
#include <soc.h>
#include <tzc380.h>
#include <imx_csu.h>
#include <imx_rdc.h>
#include <uart.h>

#define DDRC_LPDDR4	BIT(5)
#define DDRC_DDR4	BIT(4)
#define DDR_TYPE_MASK	0x3f
#define DDRC_ACTIVE_RANK_MASK	0x3

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
	/* initialized fsp table */
	unsigned int fsp_table[4];
};

struct dram_info {
	int dram_type;
	unsigned int num_rank;
	int current_fsp;
	int boot_fsp;
	struct dram_timing_info *timing_info;
};

void lpddr4_mr_write(uint32_t, uint32_t, uint32_t);
uint32_t lpddr4_mr_read(uint32_t, uint32_t);

void ddrphy_load_pie_image(void);
void dram_info_init(unsigned long dram_timing_base);
void dram_umctl2_init(void);
void dram_phy_init(void);

void lpddr4_enter_retention(void);
void lpddr4_exit_retention(void);
void ddr4_enter_retention(void);
void ddr4_exit_retention(void);
/* lpddr4 swffc for dvfs */
void lpddr4_swffc(struct dram_info *dram_info, unsigned int dev_fsp,
	 unsigned int tgt_freq, bool bypass_mode_supported);
/* ddr4 swffw for dvfs */
void ddr4_swffc(struct dram_info *dram_info, unsigned int target_fsp);

/* dram retention */
void dram_enter_retention(void);
void dram_exit_retention(void);

void dram_clock_switch(unsigned int target_freq);
void dram_pll_init(unsigned int drate);

#endif /* __DRAM_H__ */
