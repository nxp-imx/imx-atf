/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>

#include <lib/mmio.h>
#include <drivers/delay_timer.h>

#include <platform_def.h>
#include "dram.h"

#define DDRC_BASE		U(0x4E300000)
#define DDR_SDRAM_CFG		(DDRC_BASE + 0x110)
#define DDR_SDRAM_CFG_2		(DDRC_BASE + 0x114)
#define DDR_SDRAM_MD_CNTL	(DDRC_BASE + 0x120)
#define DDR_SDRAM_INTERVAL	(DDRC_BASE + 0x124)
#define DDR_ZQ_CNTL		(DDRC_BASE + 0x170)
#define DDR_SR_CNTR		(DDRC_BASE + 0x17c)
#define DDR_SDRAM_CFG_3		(DDRC_BASE + 0x260)
#define DDR_SDRAM_CFG_4		(DDRC_BASE + 0x264)
#define DDRDSR_2		(DDRC_BASE + 0xB24)
#define DEBUG_26		(DDRC_BASE + 0xF64)

/* DDRMIX_BLK_CTRL */
#define DDRC_STOP_CTRL		(0x4E01000C)
#define AUTO_CG_CTRL		(0x4E010010)

#define DRAM_PLL_BASE		U(0x44481300)

#define PLL_CTRL	U(0x0)
#define PLL_CTRL_SET	U(0x4)
#define PLL_CTRL_CLR	U(0x8)
#define PLL_CTRL_TOG	U(0xC)

#define PLL_SPREAD_SPECTRUM		U(0x30)
#define PLL_SPREAD_SPECTRUM_SET		U(0x34)
#define PLL_SPREAD_SPECTRUM_CLR		U(0x38)
#define PLL_SPREAD_SPECTRUM_TOG		U(0x3C)

#define PLL_NUMERATOR			U(0x40)
#define PLL_DENOMINATOR			U(0x50)

#define PLL_DIV				U(0x60)
#define PLL_STATUS			U(0xF0)

#define SRC_DDRC_SW_CTRL		U(0x44461020)
#define SRC_DDRPHY_SW_CTRL		U(0x44461420)
#define SRC_DDRPHY_SINGLE_RESET_SW_CTRL	U(0x44461424)

extern void ddr_disable_bypass(void);

void dram_pll_init(void)
{
	/* powerup and enable */
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x1);

	/* wait lock */
	while (!(mmio_read_32(DRAM_PLL_BASE + PLL_STATUS) & 0x1)) {
		;
	}

	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_CLR, 0x4);
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x2);
}


void self_refresh_enter()
{
	mmio_setbits_32(DDR_SDRAM_CFG_2, BIT(31));
}

void self_refresh_exit()
{
	mmio_clrbits_32(DDR_SDRAM_CFG_2,BIT(31));
}

static void ipg_stop_ack_wait(uint32_t expect_value)
{
	uint32_t read_data;
	read_data = mmio_read_32(DDRC_STOP_CTRL);
	if (expect_value == 0x1) {
		while ((read_data & BIT(1)) == 0x0){
			read_data= mmio_read_32(DDRC_STOP_CTRL);
		}
	} else if (expect_value == 0x0){
		while ((read_data & BIT(1)) != 0x0){
			read_data = mmio_read_32(DDRC_STOP_CTRL);
		}
	}
}

void check_dfi_init_complete(void)
{
	uint32_t regval;

	do {
		regval = mmio_read_32(DDRDSR_2);
 		if (regval & 0x04) {
			break;
		}
	} while (1);

	mmio_setbits_32(DDRDSR_2, BIT(2));
}

void dram_enter_retention(void)
{
	uint32_t val;
	int eccen = 0;
	uint32_t waitflag = 0;

	eccen = !!(mmio_read_32(REG_ERR_EN) & 0x40000000);

	if(eccen && ((mmio_read_32(REG_DDR_TX_CFG_1) & 0xF) != 0)){
		mmio_clrbits_32(REG_DDR_TX_CFG_1, 0xF);
	}

	if(eccen){
		waitflag = 0xC0000000;
	}
	else{
		waitflag = 0x80000000;
	}

	/* 2. Polling for DDRDSR_2[IDLE] to be set */
	check_ddrc_idle(0, waitflag);

	/* HALT the ddrc axi port */
	mmio_setbits_32(DDR_SDRAM_CFG, BIT(1));

	/* 3. Set DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_setbits_32(DDR_SDRAM_CFG_3, BIT(1));
	if ((mmio_read_32(DDR_SDRAM_CFG_3) & 0x2) != 0) {
		NOTICE("SR_FAST_WK_EN bit set success\n");
	} else {
		NOTICE("SR_FAST_WK_EN bit set failed\n");
	}

	/* 4. Program DDR_SDRAM_CFG_4[DFI_FREQ] to 2b'1111 */
	val = mmio_read_32(DDR_SDRAM_CFG_4);
	mmio_write_32(DDR_SDRAM_CFG_4, val | 0x1f000);
	if ((mmio_read_32(DDR_SDRAM_CFG_4) & 0x1f000) != 0x1f000) {
		NOTICE("DFI_FREQ bits set success\n");
	} else {
		NOTICE("DFI_FREQ bit set failed\n");
	}

	/* 5. Clear DDR_ZQ_CNTL register */
	mmio_write_32(DDR_ZQ_CNTL, 0x0);

	/* 7. Force the DDRC to enter self refresh */
	self_refresh_enter();
	mmio_setbits_32(AUTO_CG_CTRL, BIT(17)); /* Enable ipg_stop_reg while auto cg */
	mmio_setbits_32(DDR_SDRAM_CFG, BIT(30)); /* enable SR during sleep */
	mmio_setbits_32(DDR_SDRAM_CFG_3, BIT(11)); /* drain for self refresh */

	/* 8. Set stop and poll stop ack */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(DDRC_STOP_CTRL, BIT(0));
	ipg_stop_ack_wait(0x1);

	/* 9. Clear DDR_INTERVAL(this disables refreshes */
	mmio_write_32(DDR_SDRAM_INTERVAL, 0x0);

	/* 10. Set DDR_SDRAM_MD_CNTL to 0x00100000(this forces CKE to remain low */
	mmio_write_32(DDR_SDRAM_MD_CNTL, 0x00100000);

	/* 11. Remove Stop request via DDRMIX register */
	mmio_clrbits_32(DDRC_STOP_CTRL, BIT(0));
	check_dfi_init_complete();

	/* 12. Clear DDR_SDRAM_CFG[SREN] */
	mmio_clrbits_32(DDR_SDRAM_CFG, BIT(30));

	/* 13. Clear DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_clrbits_32(DDR_SDRAM_CFG_3, BIT(1));

	/* 14. Set stop request again via DDRMIX register */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(DDRC_STOP_CTRL, BIT(0));

	/* 15. Poll for STOP_ACK to be asserted in DDRMIX register */
	ipg_stop_ack_wait(0x1);

	/* 16. Set STOP request again via DDRMIX register */
	mmio_clrbits_32(DDRC_STOP_CTRL, BIT(0));

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if (!(dwc_ddrphy_apb_rd(0x90028) & 0x1))
		INFO("PhyInLP3 = 1\n");
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* 17. Clear PwrOkIn via DDRMIX regsiter */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));

	/* 18. Power off the DDRMIX */
	NOTICE("Enable ddr power off\r\n");
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(31));
	NOTICE("enter retention done\n");
}

/* Restore the dram PHY config */
void dram_phy_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *cfg = timing->ddrphy_cfg;
	unsigned int i;

	/* Restore the PHY init config */
	cfg = timing->ddrphy_cfg;
	for (i = 0U; i < timing->ddrphy_cfg_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_wr(0xc0080, 0x3);
	/* Restore the DDR PHY CSRs */
	cfg = timing->ddrphy_trained_csr;
	for (i = 0U; i < timing->ddrphy_trained_csr_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xc0080, 0x2);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* Load the PIE image */
	cfg = timing->ddrphy_pie;
	for (i = 0U; i < timing->ddrphy_pie_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}
}

void ddrc_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *ddrc_cfg = timing->ddrc_cfg;
	unsigned int i;
	uint32_t val;

	for (i = 0;  i < timing->ddrc_cfg_num; i++) {
		/* skip the dram init as we resume from retention */
		if (ddrc_cfg->reg == 0x4e300114) {
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val & ~BIT(4));
		} else {
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
		}
		ddrc_cfg++;
	}

	if(timing->fsp_cfg != NULL){
		ddrc_cfg = timing->fsp_cfg[0].ddrc_cfg;
		while(ddrc_cfg->reg != 0){
			mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
			ddrc_cfg++;
		}
	}

	check_dfi_init_complete();

	val = mmio_read_32(DDR_SDRAM_CFG);
	mmio_write_32(DDR_SDRAM_CFG, val | BIT(31));

	check_ddrc_idle(0, 0x80000000);
}

void ddrphy_coldreset(void) {
	/*
	 * dramphy_apb_n default 1 , assert -> 0, de_assert -> 1
	 * dramphy_reset_n default 0 , assert -> 0, de_assert -> 1
	 * dramphy_PwrOKIn default 0 , assert -> 1, de_assert -> 0
	 */
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	/* src_gen_dphy_PwrOKIn_sw_rst_de_assert() */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);

	/* src_gen_dphy_apb_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	udelay(10);

	/* src_gen_dphy_PwrOKIn_sw_rst_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert() */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
}

void dram_exit_retention(void)
{
	/* 1. Power up the DDRMIX */
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(31));

	/* additional step to make sure DDR exit retenton works */
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10);
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10);

	/* 2. Cold reset the DDRPHY */
	ddrphy_coldreset();

	/* 3. Config the DRAM PLL to FSP0 */
	dram_pll_init();
	ddr_disable_bypass();

	/* 4. Reload the DDRPHY config */
	dram_phy_init(timing_info);
	printf("phy reload done\n");

	/* 5. Reload the ddrc config */
	ddrc_init(timing_info);

	/* set SR_FAST_WK_EN to 1 by default */
	mmio_setbits_32(REG_DDR_SDRAM_CFG_3, BIT(1));

	NOTICE("exit retention done\n");
}
