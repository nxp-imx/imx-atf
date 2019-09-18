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
#include <smccc.h>
#include <smccc_helpers.h>
#include <imx_sip.h>
#include <interrupt_mgmt.h>
#include <std_svc.h>

static struct dram_info dram_info;

/* lock used for DDR DVFS */
spinlock_t dfs_lock;
/* IRQ used for DDR DVFS */
#if defined(PLAT_IMX8M)
/* ocram used to dram timing */
static uint8_t dram_timing_saved[13 * 1024] __aligned(8);
#endif
static volatile uint32_t wfe_done;
static volatile bool wait_ddrc_hwffc_done = true;

static unsigned int dev_fsp = 0x1;
bool bypass_mode_supported = true;

#if defined (PLAT_IMX8M)
/* copy the dram timing info from DRAM to OCRAM */
void imx8mq_dram_timing_copy(struct dram_timing_info *from,
	 struct dram_timing_info *to)
{
	struct dram_cfg_param *cfg1, *cfg2;
	unsigned int num;

	/* copy the dram_timing info header */
	cfg1 = (struct dram_cfg_param *) ((unsigned long) to + sizeof(struct dram_timing_info));
	cfg2 = from->ddrc_cfg;

	/* if no valid dram timing info, return */
	if (((unsigned long)from + sizeof(struct dram_timing_info)) != (unsigned long)cfg2)
			return;

	/* copy the ddrc init config */
	to->ddrc_cfg_num = from->ddrc_cfg_num;
	to->ddrphy_cfg_num = from->ddrphy_cfg_num;
	to->ddrphy_trained_csr_num = from->ddrphy_trained_csr_num;
	to->ddrphy_pie_num = from->ddrphy_pie_num;

	/* copy the fsp table */
	for (int i = 0; i < 4; i++)
		to->fsp_table[i] = from->fsp_table[i];

	/* copy the ddrc config */
	to->ddrc_cfg = cfg1;
	num = from->ddrc_cfg_num;
	for (int i = 0; i < num; i++) {
		cfg1->reg = cfg2->reg;
		cfg1->val = cfg2->val;
		cfg1++;
		cfg2++;
	}

	/* copy the ddrphy init config */
	to->ddrphy_cfg = cfg1;
	num = from->ddrphy_cfg_num;
	for (int i = 0; i < num; i++) {
		cfg1->reg = cfg2->reg;
		cfg1->val = cfg2->val;
		cfg1++;
		cfg2++;
	}

	/* copy the ddrphy csr */
	to->ddrphy_trained_csr = cfg1;
	num = from->ddrphy_trained_csr_num;
	for (int i = 0; i < num; i++) {
		cfg1->reg = cfg2->reg;
		cfg1->val = cfg2->val;
		cfg1++;
		cfg2++;
	}
	/* copy the PIE image */
	to->ddrphy_pie = cfg1;
	num = from->ddrphy_pie_num;
	for (int i = 0; i < num; i++) {
		cfg1->reg = cfg2->reg;
		cfg1->val = cfg2->val;
		cfg1++;
		cfg2++;
	}
}
#endif

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

#define BYPASS_MODE_DRATE		666
static bool is_bypass_mode_enabled(struct dram_timing_info *info)
{
	/*
	 * if there is a fsp drate is lower than 666, we assume
	 * that the bypass mode is enanbled.
	 */
	if(info->fsp_table[1] > BYPASS_MODE_DRATE ||
		 info->fsp_table[2] > BYPASS_MODE_DRATE)
		return false;
	else
		return true;
}

/* EL3 SGI-8 handler */
static uint64_t waiting_dvfs(uint32_t id, uint32_t flags,
				void *handle, void *cookie)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
	uint32_t irq;

	irq = plat_ic_acknowledge_interrupt();
	if (irq < 1022U) {
		plat_ic_end_of_interrupt(irq);
	}

	/* set the WFE done status */
	spin_lock(&dfs_lock);
	wfe_done |= (1 << cpu_id * 8);
	dsb();
	spin_unlock(&dfs_lock);

	while (1) {
		/* ddr frequency change done */
		wfe();
		if (!wait_ddrc_hwffc_done)
			break;
	}

	return 0;
}

void dram_info_init(unsigned long dram_timing_base)
{
	uint32_t current_fsp, ddr_type, ddrc_mstr;

	/* get the dram type */
	ddrc_mstr = mmio_read_32(DDRC_MSTR(0));
	ddr_type = ddrc_mstr & DDR_TYPE_MASK;

	dram_info.dram_type = ddr_type;
	dram_info.num_rank = (ddrc_mstr >> 24) & DDRC_ACTIVE_RANK_MASK;

	/* init the boot_fsp & current_fsp */
	current_fsp = mmio_read_32(DDRC_DFIMISC(0));
	current_fsp = (current_fsp >> 8) & 0xf;
	dram_info.boot_fsp = current_fsp;
	dram_info.current_fsp = current_fsp;

#if defined(PLAT_IMX8M)
	imx8mq_dram_timing_copy((struct dram_timing_info *)dram_timing_base,
		(struct dram_timing_info *)dram_timing_saved);

	dram_timing_base = (unsigned long) dram_timing_saved;
#endif
	bypass_mode_supported = is_bypass_mode_enabled((struct dram_timing_info *)dram_timing_base);
	/*
	 * No need to do save for ddrc and phy config register,
	 * we have done it in SPL stage and save in memory
	 */
	dram_info.timing_info = (struct dram_timing_info *)dram_timing_base;

	/* switch to the highest frequency point */
	if(ddr_type == DDRC_LPDDR4 && current_fsp != 0x0) {
		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);
		lpddr4_swffc(&dram_info, dev_fsp, 0x0, bypass_mode_supported);
		dev_fsp = (~dev_fsp) & 0x1;
	} else if (ddr_type == DDRC_DDR4 && current_fsp != 0x0) {
		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);
		ddr4_swffc(&dram_info, 0x0);
	}

	/* register the SGI handler for DVFS */
	uint64_t flags = 0;
	uint64_t rc;

	set_interrupt_rm_flag(flags, NON_SECURE);
	rc = register_interrupt_type_handler(INTR_TYPE_EL3, waiting_dvfs, flags);

	if(rc)
		panic();
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

/*
 * For each freq return the following info:
 *
 * r1: data rate
 * r2: 1 + dram_core parent
 * r3: 1 + dram_alt parent index
 * r4: 1 + dram_apb parent index
 *
 * The parent indices can be used by an OS who manages source clocks to enabled
 * them ahead of the switch.
 *
 * A parent value of "0" means "don't care".
 *
 * Current implementation of freq switch is hardcoded in
 * plat/imx/common/imx8m/clock.c but in theory this can be enhanced to support
 * a wide variety of rates.
 */
int dram_dvfs_get_freq_info(void *handle, u_register_t index)
{
	switch (index) {
	case 0: SMC_RET4(handle, dram_info.timing_info->fsp_table[0],
				1, 0, 5);
	case 1: SMC_RET4(handle, dram_info.timing_info->fsp_table[1],
				2, 2, 4);
	case 2: SMC_RET4(handle, dram_info.timing_info->fsp_table[2],
				2, 3, 3);
	case 3: SMC_RET4(handle, dram_info.timing_info->fsp_table[3],
				1, 0, 0);
	default:
		SMC_RET1(handle, -3);
	}
}

int dram_dvfs_handler(uint32_t smc_fid,
			void *handle,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
	unsigned int target_freq = x1;
	uint32_t online_cores = x2;

	if (x1 == IMX_SIP_DDR_DVFS_WAIT_CHANGE) {
		/* set the WFE done status */
		spin_lock(&dfs_lock);
		wfe_done |= (1 << cpu_id * 8);
		dsb();
		spin_unlock(&dfs_lock);

		while (1) {
			/* ddr frequency change done */
			wfe();
			if (!wait_ddrc_hwffc_done)
				break;
		}
	} else if (x1 == IMX_SIP_DDR_DVFS_GET_FREQ_COUNT) {
		int i;
		for (i = 0; i < 4; ++i)
			if (!dram_info.timing_info->fsp_table[i])
				break;
		SMC_RET1(handle, i);
	} else if (x1 == IMX_SIP_DDR_DVFS_GET_FREQ_INFO) {
		return dram_dvfs_get_freq_info(handle, x2);
	} else if (x1 < 4) {
		wait_ddrc_hwffc_done = true;
		dsb();

		/* trigger the SGI to info other cores */
		for (int i = 0; i < PLATFORM_CORE_COUNT; i++)
			if (cpu_id != i && (online_cores & (0x1 << (i * 8))))
				plat_ic_raise_el3_sgi(0x8, i);

		/* make sure all the core in WFE */
		online_cores &= ~(0x1 << (cpu_id * 8));
#if defined(PLAT_IMX8M)
		for (int i = 0; i < 4; i++) {
			if (i != cpu_id && online_cores & (1 << (i * 8)))
				imx_gpc_core_wake(1 << i);
		}
#endif
		while (1) {
			if (online_cores == wfe_done)
				break;
		}

		/* flush the L1/L2 cache */
		dcsw_op_all(DCCSW);

		if (dram_info.dram_type == DDRC_LPDDR4) {
			lpddr4_swffc(&dram_info, dev_fsp, target_freq, bypass_mode_supported);
			dev_fsp = (~dev_fsp) & 0x1;
		} else if (dram_info.dram_type == DDRC_DDR4) {
			ddr4_swffc(&dram_info, target_freq);
		}

		wait_ddrc_hwffc_done = false;
		wfe_done = 0;
		dsb();
		sev();
		isb();

		SMC_RET1(handle, 0);
	}

	SMC_RET1(handle, SMC_UNK);
}
