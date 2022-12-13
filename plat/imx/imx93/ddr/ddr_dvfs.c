/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <dram.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>

#define DRAM_PLL_BASE		U(0x44481300)
#define MFI_MASK		GENMASK_32(24, 16)
#define RDIV_MASK		GENMASK_32(15, 13)
#define ODIV_MASK		GENMASK_32(7, 0)

/* GPR2 used for DDR alt clock source select 0: PLL, 1: CCM ALT */
#define CCM_GPR2		U(0x44454840)
#define DRAM_ALT_CLK		U(0x44452600)
#define DRAM_APB_CLK		U(0x44452680)

#define FRAC_PLL_RATE(_rate, _r, _m, _o, _n, _d)		\
        {							\
		.rate   =       (_rate),			\
		.rdiv   =       (_r),				\
		.mfi    =       (_m),				\
		.odiv   =       (_o),				\
		.mfn    =       (_n),				\
		.mfd    =       (_d),				\
        }

struct imx_fracpll_rate_table {
        uint32_t rate; /*khz*/
        int rdiv;
        int mfi;
        int odiv;
        int mfn;
        int mfd;
};

static struct imx_fracpll_rate_table imx9_fracpll_tbl[] = {
        FRAC_PLL_RATE(933000000U, 1, 155, 4, 1, 2), /* 933Mhz */
        FRAC_PLL_RATE(800000000U, 1, 200, 6, 0, 1), /* 800Mhz */
        FRAC_PLL_RATE(600000000U, 1, 200, 8, 0, 1), /* 600Mhz */
        FRAC_PLL_RATE(400000000U, 1, 200, 12, 0, 1), /* 400Mhz */
        FRAC_PLL_RATE(233000000U, 1, 155, 16, 1, 3), /* 233Mhz */
        FRAC_PLL_RATE(200000000U, 1, 200, 24, 0, 1), /* 200Mhz */
        FRAC_PLL_RATE(166000000U, 1, 166, 24, 0, 1), /* 166Mhz */
        FRAC_PLL_RATE(167000000U, 1, 167, 24, 0, 1), /* 167Mhz */
};

int ddr_pll_init(unsigned int drate)
{
	uint32_t pll_div, pll_mfn, pll_mfd, new_div, new_mfn, new_mfd;
	struct imx_fracpll_rate_table *rate;
	unsigned int freq = (drate / 4) * 1000000;
	unsigned int maxtimeout = 10;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(imx9_fracpll_tbl); i++) {
		if (freq == imx9_fracpll_tbl[i].rate)
			break;
	}

	if (i == ARRAY_SIZE(imx9_fracpll_tbl)) {
		WARN("no valid freq table: %u\n", freq);
		return -1;
	}

	rate = &imx9_fracpll_tbl[i];

	new_div = ((rate->mfi << 16) & MFI_MASK) | ((rate->rdiv << 13) & RDIV_MASK) |
		  (rate->odiv & ODIV_MASK);
	new_mfn = rate->mfn << 2;
	new_mfd = rate->mfd & GENMASK_32(29, 0);

	pll_div = mmio_read_32(DRAM_PLL_BASE + 0x60);
	pll_div &= (MFI_MASK | RDIV_MASK | ODIV_MASK);
	pll_mfn = mmio_read_32(DRAM_PLL_BASE + 0x40) & GENMASK_32(31, 2);
	pll_mfd = mmio_read_32(DRAM_PLL_BASE + 0x50) & GENMASK_32(29, 0);
	/* skip reconfig if the pll is running at the the target freq */
	if (pll_div == new_div && pll_mfn == new_mfn && pll_mfd == new_mfd) {
		return 0;
	}

	/* disable the PLL output & power down */
	mmio_write_32(DRAM_PLL_BASE + 0x8, BIT(0) | BIT(1));

	/* set ODIV, RDIV, MFI */
	mmio_write_32(DRAM_PLL_BASE + 0x60, new_div);
	/* set the NUM & DENOM */
	mmio_write_32(DRAM_PLL_BASE + 0x40, new_mfn);
	mmio_write_32(DRAM_PLL_BASE + 0x50, new_mfd);

	/* power up the PLL */
	mmio_write_32(DRAM_PLL_BASE + 0x4, BIT(0));
	/* wait for the PLL lock */
	do {
		if (mmio_read_32(DRAM_PLL_BASE + 0xf0) & BIT(0)) {
			/* enable the clock output */
			mmio_write_32(DRAM_PLL_BASE + 0x4, BIT(1));
			break;
		}

		udelay(10);
	} while(--maxtimeout);

	return 0;
}

void ddr_pll_bypass(unsigned int drate)
{
	switch (drate) {
	case 400:
		mmio_clrsetbits_32(DRAM_ALT_CLK, 0x3ff, (0x2 << 8) | 0x1);
		break;
	case 200:
		mmio_clrsetbits_32(DRAM_ALT_CLK, 0x3ff, (0x2 << 8) | 0x3);
		break;
	case 100:
		mmio_clrsetbits_32(DRAM_ALT_CLK, 0x3ff, (0x2 << 8) | 0x7);
		break;
	default:
		break;
	}

	/* wait for ROOT reconfig done */
	while (mmio_read_32(DRAM_ALT_CLK + 0x20) & BIT(28)) {
	}

	/* FIXME: set DRAM APB clock to 133MHz */
	mmio_clrsetbits_32(DRAM_APB_CLK, 0x3FF, (0x2 << 8) | 0x2);

	/* wait for the new setting update done */
	while (mmio_read_32(DRAM_APB_CLK + 0x20) & BIT(28)) {
	}

	/* Switch DRAM clock source to CCM DRAM_ALT */
	mmio_setbits_32(CCM_GPR2, BIT(0));
}

void ddr_disable_bypass(void) {
        /* Set DRAM APB to 133Mhz, */
	mmio_clrsetbits_32(DRAM_APB_CLK, 0x3FF, (0x2 << 8) | 0x2);
	/* wait for the new setting update done */
	while (mmio_read_32(DRAM_APB_CLK + 0x20) & BIT(28)) {
	}

        /* Switch from DRAM  clock root from CCM to PLL */
	mmio_clrbits_32(CCM_GPR2, BIT(0));
}

/* change the dram clock frequency */
void ddr_clock_switch(struct dram_fsp_cfg *cfg, unsigned int target_drate){

	if (!cfg->bypass){
		ddr_pll_init(target_drate);
		ddr_disable_bypass();
	} else {
		ddr_pll_bypass(target_drate);
	}

}

/* DDR SWFFC flow is normally used for non half speed DDR frequency scaling */
void ddr_swffc(struct dram_timing_info *info, unsigned int pstat)
{
	static uint32_t cur_state = 0;
	uint32_t regval, tmp;
	uint32_t PPTTrainSetup_Save = 0;
	uint8_t mr13 = 0;
	if(pstat == cur_state)
		return;

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	PPTTrainSetup_Save = dwc_ddrphy_apb_rd(0x20010);
	dwc_ddrphy_apb_wr(0x20010, 0x0);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* [SR_FAST_WK_EN] should be cleared */
	mmio_clrbits_32(REG_DDR_SDRAM_CFG_3,BIT(1));

	/* DDR_SDRAM_CFG[MEM_HALT] = 1 */
	mmio_setbits_32(REG_DDR_SDRAM_CFG,BIT(1));
	do {
		regval = mmio_read_32(REG_DDR_SDRAM_CFG);
		if (regval & 0x02)
			break;
	} while (1);

	check_ddrc_idle();

	mr13 = 0xC0;   /* freqset: 1, mrset: 1 */
	ddrc_mrs(0x4, mr13, 13);

	ddrc_apply_reg_config(REG_TYPE_MR, info->fsp_cfg[pstat].mr_cfg);

	/* [DFI_FREQ] for PState */
	tmp= mmio_read_32(REG_DDR_SDRAM_CFG_4);
	tmp = ((tmp & 0xFFFE0FFF) | (pstat << 12));
	mmio_write_32(REG_DDR_SDRAM_CFG_4, tmp);
	while (1) {
		tmp =mmio_read_32(REG_DDR_SDRAM_CFG_4);
		if(((tmp & 0x0001F000) >> 12) == pstat)
			break;
	}

	if (info->fsp_cfg[cur_state].bypass) {
		/* bypass mode reset DDRC state machine */
		udelay(2);
		mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(31));
		while((mmio_read_32(REG_DDR_SDRAM_CFG_3) & BIT(31)) == BIT(31)) {}
		udelay(5);
	}

	/* FRC_SR] to force DDRC to place DRAM in self refresh */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_2, BIT(31));

	/* Must wait some time to ensure in self refresh */
	udelay(70);
	ddr_clock_switch(&info->fsp_cfg[pstat], info->fsp_table[pstat]);
	ddrc_apply_reg_config(REG_TYPE_DDRC, info->fsp_cfg[pstat].ddrc_cfg);

	/* Clear FRC_SR] exit self refresh */
	mmio_clrbits_32(REG_DDR_SDRAM_CFG_2, BIT(31));
	do {
		regval = mmio_read_32(REG_DDR_SDRAM_CFG_2);
		if ((regval & 0x80000000) == 0x0)
			break;
	} while (1);

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_wr(0x20010, PPTTrainSetup_Save);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* Clear MEM_HALT */
	mmio_clrbits_32(REG_DDR_SDRAM_CFG,BIT(1));
	do {
		regval = mmio_read_32(REG_DDR_SDRAM_CFG);
		if ((regval & 0x02)== 0x0)
			break;
	} while (1);

	cur_state = pstat;
}

void ipg_stop_ack_wait(uint32_t expect)
{
	uint32_t regval;

	do {
		regval = mmio_read_32(REG_DDRC_STOP_CTRL);
		regval = (regval >> 1) & 0x1;
		if(regval == expect)
			break;
	} while (1);
}

void dram_hwffc_half_speed()
{
	mmio_write_32(REG_HWFFC_CTRL, 0x3); /* enable HWFFC and select half speed */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(REG_DDRC_STOP_CTRL, BIT(0));
	ipg_stop_ack_wait(0x1);
	mmio_clrbits_32(REG_DDRC_STOP_CTRL, BIT(0));
}

void dram_hwffc_full_speed()
{
	mmio_write_32(REG_HWFFC_CTRL, 0x1); /* enable HWFFC and select full speed */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(REG_DDRC_STOP_CTRL, BIT(0));
	ipg_stop_ack_wait(0x1);
	mmio_clrbits_32(REG_DDRC_STOP_CTRL, BIT(0));
}

/* DDR HWFFC can only be used for full speed & half speed fast frequency change */
void ddr_hwffc(uint32_t pstat)
{
	static uint32_t cur_state = 0;

	if(pstat == cur_state)
		return;
	mmio_setbits_32(REG_AUTO_CG_CTRL, BIT(17));

	if (pstat)
		dram_hwffc_half_speed();
	else
		dram_hwffc_full_speed();

	cur_state = pstat;
}
