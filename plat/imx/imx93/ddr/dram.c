/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/interrupt_mgmt.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <drivers/delay_timer.h>

#include <dram.h>

#define IMX_SIP_DDR_DVFS_GET_FREQ_COUNT		0x10
#define IMX_SIP_DDR_DVFS_GET_FREQ_INFO		0x11

/* lock used for DDR DVFS */
spinlock_t dfs_lock;
struct dram_timing_info *timing_info;
static volatile uint32_t core_count;
static unsigned int num_fsp;
static unsigned int cur_fsp;
static volatile bool in_progress = false;
static bool in_swffc = false;

unsigned long ddrphy_addr_remap(uint32_t paddr_apb_from_ctlr)
{
	uint32_t paddr_apb_qual;
	uint32_t paddr_apb_unqual_dec_22_13;
	uint32_t paddr_apb_unqual_dec_19_13;
	uint32_t paddr_apb_unqual_dec_12_1;
	uint32_t paddr_apb_unqual;
	uint32_t paddr_apb_phy;

	paddr_apb_qual = (paddr_apb_from_ctlr << 1);
	paddr_apb_unqual_dec_22_13 = ((paddr_apb_qual & 0x7fe000) >> 13);
	paddr_apb_unqual_dec_12_1 = ((paddr_apb_qual & 0x1ffe) >> 1);

	switch (paddr_apb_unqual_dec_22_13) {
	case 0x000: paddr_apb_unqual_dec_19_13 = 0x00; break;
	case 0x001: paddr_apb_unqual_dec_19_13 = 0x01; break;
	case 0x002: paddr_apb_unqual_dec_19_13 = 0x02; break;
	case 0x003: paddr_apb_unqual_dec_19_13 = 0x03; break;
	case 0x004: paddr_apb_unqual_dec_19_13 = 0x04; break;
	case 0x005: paddr_apb_unqual_dec_19_13 = 0x05; break;
	case 0x006: paddr_apb_unqual_dec_19_13 = 0x06; break;
	case 0x007: paddr_apb_unqual_dec_19_13 = 0x07; break;
	case 0x008: paddr_apb_unqual_dec_19_13 = 0x08; break;
	case 0x009: paddr_apb_unqual_dec_19_13 = 0x09; break;
	case 0x00a: paddr_apb_unqual_dec_19_13 = 0x0a; break;
	case 0x00b: paddr_apb_unqual_dec_19_13 = 0x0b; break;
	case 0x100: paddr_apb_unqual_dec_19_13 = 0x0c; break;
	case 0x101: paddr_apb_unqual_dec_19_13 = 0x0d; break;
	case 0x102: paddr_apb_unqual_dec_19_13 = 0x0e; break;
	case 0x103: paddr_apb_unqual_dec_19_13 = 0x0f; break;
	case 0x104: paddr_apb_unqual_dec_19_13 = 0x10; break;
	case 0x105: paddr_apb_unqual_dec_19_13 = 0x11; break;
	case 0x106: paddr_apb_unqual_dec_19_13 = 0x12; break;
	case 0x107: paddr_apb_unqual_dec_19_13 = 0x13; break;
	case 0x108: paddr_apb_unqual_dec_19_13 = 0x14; break;
	case 0x109: paddr_apb_unqual_dec_19_13 = 0x15; break;
	case 0x10a: paddr_apb_unqual_dec_19_13 = 0x16; break;
	case 0x10b: paddr_apb_unqual_dec_19_13 = 0x17; break;
	case 0x200: paddr_apb_unqual_dec_19_13 = 0x18; break;
	case 0x201: paddr_apb_unqual_dec_19_13 = 0x19; break;
	case 0x202: paddr_apb_unqual_dec_19_13 = 0x1a; break;
	case 0x203: paddr_apb_unqual_dec_19_13 = 0x1b; break;
	case 0x204: paddr_apb_unqual_dec_19_13 = 0x1c; break;
	case 0x205: paddr_apb_unqual_dec_19_13 = 0x1d; break;
	case 0x206: paddr_apb_unqual_dec_19_13 = 0x1e; break;
	case 0x207: paddr_apb_unqual_dec_19_13 = 0x1f; break;
	case 0x208: paddr_apb_unqual_dec_19_13 = 0x20; break;
	case 0x209: paddr_apb_unqual_dec_19_13 = 0x21; break;
	case 0x20a: paddr_apb_unqual_dec_19_13 = 0x22; break;
	case 0x20b: paddr_apb_unqual_dec_19_13 = 0x23; break;
	case 0x300: paddr_apb_unqual_dec_19_13 = 0x24; break;
	case 0x301: paddr_apb_unqual_dec_19_13 = 0x25; break;
	case 0x302: paddr_apb_unqual_dec_19_13 = 0x26; break;
	case 0x303: paddr_apb_unqual_dec_19_13 = 0x27; break;
	case 0x304: paddr_apb_unqual_dec_19_13 = 0x28; break;
	case 0x305: paddr_apb_unqual_dec_19_13 = 0x29; break;
	case 0x306: paddr_apb_unqual_dec_19_13 = 0x2a; break;
	case 0x307: paddr_apb_unqual_dec_19_13 = 0x2b; break;
	case 0x308: paddr_apb_unqual_dec_19_13 = 0x2c; break;
	case 0x309: paddr_apb_unqual_dec_19_13 = 0x2d; break;
	case 0x30a: paddr_apb_unqual_dec_19_13 = 0x2e; break;
	case 0x30b: paddr_apb_unqual_dec_19_13 = 0x2f; break;
	case 0x010: paddr_apb_unqual_dec_19_13 = 0x30; break;
	case 0x011: paddr_apb_unqual_dec_19_13 = 0x31; break;
	case 0x012: paddr_apb_unqual_dec_19_13 = 0x32; break;
	case 0x013: paddr_apb_unqual_dec_19_13 = 0x33; break;
	case 0x014: paddr_apb_unqual_dec_19_13 = 0x34; break;
	case 0x015: paddr_apb_unqual_dec_19_13 = 0x35; break;
	case 0x016: paddr_apb_unqual_dec_19_13 = 0x36; break;
	case 0x017: paddr_apb_unqual_dec_19_13 = 0x37; break;
	case 0x018: paddr_apb_unqual_dec_19_13 = 0x38; break;
	case 0x019: paddr_apb_unqual_dec_19_13 = 0x39; break;
	case 0x110: paddr_apb_unqual_dec_19_13 = 0x3a; break;
	case 0x111: paddr_apb_unqual_dec_19_13 = 0x3b; break;
	case 0x112: paddr_apb_unqual_dec_19_13 = 0x3c; break;
	case 0x113: paddr_apb_unqual_dec_19_13 = 0x3d; break;
	case 0x114: paddr_apb_unqual_dec_19_13 = 0x3e; break;
	case 0x115: paddr_apb_unqual_dec_19_13 = 0x3f; break;
	case 0x116: paddr_apb_unqual_dec_19_13 = 0x40; break;
	case 0x117: paddr_apb_unqual_dec_19_13 = 0x41; break;
	case 0x118: paddr_apb_unqual_dec_19_13 = 0x42; break;
	case 0x119: paddr_apb_unqual_dec_19_13 = 0x43; break;
	case 0x210: paddr_apb_unqual_dec_19_13 = 0x44; break;
	case 0x211: paddr_apb_unqual_dec_19_13 = 0x45; break;
	case 0x212: paddr_apb_unqual_dec_19_13 = 0x46; break;
	case 0x213: paddr_apb_unqual_dec_19_13 = 0x47; break;
	case 0x214: paddr_apb_unqual_dec_19_13 = 0x48; break;
	case 0x215: paddr_apb_unqual_dec_19_13 = 0x49; break;
	case 0x216: paddr_apb_unqual_dec_19_13 = 0x4a; break;
	case 0x217: paddr_apb_unqual_dec_19_13 = 0x4b; break;
	case 0x218: paddr_apb_unqual_dec_19_13 = 0x4c; break;
	case 0x219: paddr_apb_unqual_dec_19_13 = 0x4d; break;
	case 0x310: paddr_apb_unqual_dec_19_13 = 0x4e; break;
	case 0x311: paddr_apb_unqual_dec_19_13 = 0x4f; break;
	case 0x312: paddr_apb_unqual_dec_19_13 = 0x50; break;
	case 0x313: paddr_apb_unqual_dec_19_13 = 0x51; break;
	case 0x314: paddr_apb_unqual_dec_19_13 = 0x52; break;
	case 0x315: paddr_apb_unqual_dec_19_13 = 0x53; break;
	case 0x316: paddr_apb_unqual_dec_19_13 = 0x54; break;
	case 0x317: paddr_apb_unqual_dec_19_13 = 0x55; break;
	case 0x318: paddr_apb_unqual_dec_19_13 = 0x56; break;
	case 0x319: paddr_apb_unqual_dec_19_13 = 0x57; break;
	case 0x020: paddr_apb_unqual_dec_19_13 = 0x58; break;
	case 0x120: paddr_apb_unqual_dec_19_13 = 0x59; break;
	case 0x220: paddr_apb_unqual_dec_19_13 = 0x5a; break;
	case 0x320: paddr_apb_unqual_dec_19_13 = 0x5b; break;
	case 0x040: paddr_apb_unqual_dec_19_13 = 0x5c; break;
	case 0x140: paddr_apb_unqual_dec_19_13 = 0x5d; break;
	case 0x240: paddr_apb_unqual_dec_19_13 = 0x5e; break;
	case 0x340: paddr_apb_unqual_dec_19_13 = 0x5f; break;
	case 0x050: paddr_apb_unqual_dec_19_13 = 0x60; break;
	case 0x051: paddr_apb_unqual_dec_19_13 = 0x61; break;
	case 0x052: paddr_apb_unqual_dec_19_13 = 0x62; break;
	case 0x053: paddr_apb_unqual_dec_19_13 = 0x63; break;
	case 0x054: paddr_apb_unqual_dec_19_13 = 0x64; break;
	case 0x055: paddr_apb_unqual_dec_19_13 = 0x65; break;
	case 0x056: paddr_apb_unqual_dec_19_13 = 0x66; break;
	case 0x057: paddr_apb_unqual_dec_19_13 = 0x67; break;
	case 0x070: paddr_apb_unqual_dec_19_13 = 0x68; break;
	case 0x090: paddr_apb_unqual_dec_19_13 = 0x69; break;
	case 0x190: paddr_apb_unqual_dec_19_13 = 0x6a; break;
	case 0x290: paddr_apb_unqual_dec_19_13 = 0x6b; break;
	case 0x390: paddr_apb_unqual_dec_19_13 = 0x6c; break;
	case 0x0c0: paddr_apb_unqual_dec_19_13 = 0x6d; break;
	case 0x0d0: paddr_apb_unqual_dec_19_13 = 0x6e; break;
	default: paddr_apb_unqual_dec_19_13 = 0x00;
	}
	paddr_apb_unqual = ((paddr_apb_unqual_dec_19_13 << 13) | (paddr_apb_unqual_dec_12_1 << 1));
	paddr_apb_phy = (paddr_apb_unqual << 1);
	return paddr_apb_phy;
}

int check_ddrc_idle(int waitus, uint32_t flag){
	uint32_t regval;
	int waitforever = 0;
	int waitedus = 0;
	if(waitus == 0)
		waitforever = 1;
	do{
		regval = mmio_read_32(REG_DDRDSR_2);
		if((regval & flag) == flag)
			break;
		udelay(1);
		waitus--;
		waitedus++;
	}while(waitus | waitforever);

	if((waitus == 0) & !waitforever){
		return -1;
	}

	return waitedus;
}

void ddrc_mrs(uint32_t cs_sel, uint32_t opcode, uint32_t mr)
{
	uint32_t regval;

	regval = (cs_sel << 28) | (opcode << 6) | (mr);
	mmio_write_32(REG_DDR_SDRAM_MD_CNTL, regval);
	mmio_setbits_32(REG_DDR_SDRAM_MD_CNTL, BIT(31));
	/* Wait until REG_DDR_SDRAM_MD_CNTL[31] is cleared by HW (MD_EN=0) */
	while((mmio_read_32(REG_DDR_SDRAM_MD_CNTL) & 0x80000000) == 0x80000000) {
	}
	check_ddrc_idle(3, 0x80000000);
}

int ddrc_apply_reg_config(enum reg_type type, struct dram_cfg_param *reg_config){

	int ret = 0;

	if (reg_config == NULL)
		return -1;

	do {
		if(reg_config->reg  == 0)
			break;

		switch (type) {
		case REG_TYPE_DDRC:
			mmio_write_32(reg_config->reg, reg_config->val);
			break;
		case REG_TYPE_MR:
			ddrc_mrs(0x4, reg_config->val, reg_config->reg);
			break;
		default:
			break;
		}

		reg_config++;

	} while(1);

	return ret;
}

/* EL3 SGI-8 IPI handler for DDR Dynamic frequency scaling */
/* For non-primary core, waiting for frequency scaling down */
static uint64_t waiting_dvfs(uint32_t id, uint32_t flags,
		void *handle, void *cookie)
{
	uint32_t irq;

	irq = plat_ic_acknowledge_interrupt();
	if (irq < 1022U)
		plat_ic_end_of_interrupt(irq);

	/* set the WFE done status */
	spin_lock(&dfs_lock);
	core_count++;
	dsb();
	spin_unlock(&dfs_lock);

	while (1) {
		/* frequency change done */
		if (!in_progress)
			break;
		wfe();
	}

	return 0;
}

void dram_info_init(unsigned long dram_timing_base)
{
	uint32_t flags = 0;
	uint32_t rc;
	int i;

	timing_info = (struct dram_timing_info *)dram_timing_base;

	/* get the num of supported fsp */
	for (i = 0; i < 4; ++i){
		if (!timing_info->fsp_table[i])
			break;
	}

	/* only support maximum 3 setpoints */
	num_fsp = (i > MAX_FSP_NUM) ? MAX_FSP_NUM : i;

	/* no valid fsp table, return directly */
	if (i == 0)
		return;

	/* set SR_FAST_WK_EN to 1 by default */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(1));

	/* Register the EL3 handler for DDR DVFS */
	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3, waiting_dvfs, flags);
	if (rc)
		panic();
}

int dram_dvfs_handler(uint32_t smc_fid, void *handle,
		u_register_t x1, u_register_t x2, u_register_t x3)
{
	unsigned int fsp_index = x1;
	uint32_t online_cpus = x2 - 1; 
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL1_VAL(mpidr);
	int ret = 0;

	/* get the fsp num, return the number of supported fsp */
	if (IMX_SIP_DDR_DVFS_GET_FREQ_COUNT == x1) {
		SMC_RET1(handle, num_fsp);
	} else if (IMX_SIP_DDR_DVFS_GET_FREQ_INFO == x1) {
		SMC_RET1(handle, timing_info->fsp_table[x2]);
	} else if (fsp_index > num_fsp) {
		/* fsp out of range */
		SMC_RET1(handle, SMC_UNK);
	} else if (fsp_index == cur_fsp) {
		SMC_RET1(handle, SMC_OK);
	}

	in_progress = true;
	dsb();

	/* notify other core wait for scaling done */
	for (int i = 0; i < PLATFORM_CORE_COUNT; i++)
		/* Skip raise SGI for current CPU */
		if (i != cpu_id)
			plat_ic_raise_el3_sgi(0x8, i << 8);
	
	/* Make sure all the cpu in WFE */
	while (1) {
		if (online_cpus == core_count)
			break;
	}
	
	/* Flush the L1/L2 cache */
	dcsw_op_all(DCCSW);

	/* if current pstate 0, next state to 1: hwffc */
	if (!in_swffc && (fsp_index == 1 || fsp_index == 0)) {
		ddr_hwffc(fsp_index);
		in_swffc = false;
	} else if(fsp_index != 1) {
		ret = ddr_swffc(timing_info, fsp_index);
		if(ret == 0)
			in_swffc = (fsp_index == 2);
	}

	if(ret == 0)
		cur_fsp = fsp_index;

	in_progress = false;
	core_count = 0;
	dsb();
	sev();
	isb();

	SMC_RET1(handle, ret);
}
