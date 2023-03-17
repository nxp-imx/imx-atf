/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRAM_H
#define DRAM_H

#include <lib/utils_def.h>
#include <arch_helpers.h>
#include <assert.h>

#include <ddrc.h>
#include <platform_def.h>

#define MAX_FSP_NUM		U(3)

#define DDR_PHY_BASE	        	DDRPHY_BASE_ADDR
#define IP2APB_DDRPHY_IPS_BASE_ADDR(X)	(DDR_PHY_BASE + (X * 0x2000000))

#define dwc_ddrphy_apb_wr(addr, data) \
        mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr), data)
#define dwc_ddrphy_apb_rd(addr) \
        mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr))

/* reg & config param */
struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_fsp_cfg{
        struct dram_cfg_param ddrc_cfg[20];
        struct dram_cfg_param mr_cfg[10];
	unsigned int bypass;
};

struct dram_timing_info {
	/* umctl2 config */
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	/* fsp config */
	struct dram_fsp_cfg *fsp_cfg;
	unsigned int fsp_cfg_num;
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

enum reg_type{
    REG_TYPE_DDRC = 0,
    REG_TYPE_MR,
};

extern struct dram_timing_info *timing_info;

void dram_info_init(unsigned long dram_timing_base);

/* dram frequency change */
int ddr_swffc(struct dram_timing_info *dram_info, unsigned int pstate);
void ddr_hwffc(uint32_t pstate);

/* dram retention */
void dram_enter_retention(void);
void dram_exit_retention(void);

int check_ddrc_idle(int waitus, uint32_t flag);
uint32_t ddrc_mrr(uint32_t mr_rank, uint32_t mr_addr);
void ddrc_mrs(uint32_t cs_sel, uint32_t opcode, uint32_t mr);
int ddrc_apply_reg_config(enum reg_type type, struct dram_cfg_param *reg_config);
unsigned long ddrphy_addr_remap(uint32_t paddr_apb_from_ctlr);

#endif /* DRAM_H */
