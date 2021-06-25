/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <lib/mmio.h>

#include <upower_soc_defs.h>
#include <upower_api.h>

#define PHY_FREQ_SEL_INDEX(x) 		((x) << 16)
#define PHY_FREQ_MULTICAST_EN(x)	((x) << 8)
#define DENALI_PHY_1537			U(0x5804)

#define IMX_DDRC_BASE			0x2E060000
#define DENALI_CTL_144			0x240
#define LPI_WAKEUP_EN_SHIFT		U(8)
#define IMX_LPAV_SIM_BASE		0x2DA50000
#define LPDDR_CTRL			0x14
#define LPDDR_AUTO_LP_MODE_DISABLE	BIT(24)
#define SOC_LP_CMD_SHIFT		U(15)
#define LPDDR_CTRL2			0x18

#define DENALI_CTL_23				U(0x5c)
#define DFIBUS_FREQ_INIT_SHIFT			U(24)
#define TSREF2PHYMSTR_SHIFT			U(8)
#define TSREF2PHYMSTR_MASK			GENMASK(13, 8)

#define DENALI_CTL_24				U(0x60)

#define DENALI_CTL_93				U(0x174)
#define PWRUP_SREFRESH_EXIT			BIT(0)

#define DENALI_CTL_127				U(0x1fc)
#define PHYMSTR_TRAIN_AFTER_INIT_COMPLETE	BIT(16)

#define DENALI_CTL_147				U(0x24c)
#define DENALI_CTL_153				U(0x264)
#define PCPCS_PD_EN				BIT(8)

#define DENALI_PHY_1547				U(0x582c)
#define PHY_LP4_BOOT_DISABLE			BIT(8)

#define DENALI_PHY_1559				U(0x585c)
#define DENALI_PI_134				U(0x2218)

extern void upower_wait_resp();

struct dram_cfg_param {
	uint32_t reg;
	uint32_t val;
};

struct dram_timing_info {
        /* ddr controller config */
        struct dram_cfg_param *ctl_cfg;
        unsigned int ctl_cfg_num;
        /* pi config */
        struct dram_cfg_param *pi_cfg;
        unsigned int pi_cfg_num;
        /* phy freq1 config */
        struct dram_cfg_param *phy_f1_cfg;
        unsigned int phy_f1_cfg_num;
        /* phy freq2 config */
        struct dram_cfg_param *phy_f2_cfg;
        unsigned int phy_f2_cfg_num;
        /* initialized drate table */
        unsigned int fsp_table[3];
};

struct dram_timing_info *info;

/* mark if dram cfg is already saved */
static bool dram_cfg_saved = false;

static void ddr_init(void)
{
	int i;

	/* restore the ddr ctl config */
	for (i = 0; i < info->ctl_cfg_num; i++)
		mmio_write_32(info->ctl_cfg[i].reg, info->ctl_cfg[i].val);

	 /* set PHY_FREQ_SEL_MULTICAST_EN=1 & PHY_FREQ_SEL_INDEX=0. restore all PHY registers. */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_SEL_INDEX(0) | PHY_FREQ_MULTICAST_EN(1));
	/* restore phy f1 config */
	for (i = 0; i < info->phy_f1_cfg_num; i++)
		mmio_write_32(info->phy_f1_cfg[i].reg, info->phy_f1_cfg[i].val);

	 /* set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=0. restore only the diff. */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_SEL_INDEX(0) | PHY_FREQ_MULTICAST_EN(0));
	for (i = 0; i < info->phy_f2_cfg_num; i++)
		mmio_write_32(info->phy_f2_cfg[i].reg, info->phy_f2_cfg[i].val);

	/* Re-enable MULTICAST mode */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_SEL_INDEX(0) | PHY_FREQ_MULTICAST_EN(1));
}

void dram_enter_retention(void)
{
	int i;

	/* 1. config the PCC_LPDDR4[SSADO] to 2b'11 for ACK domain 0/1's STOP */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, 0x2 << 22);

	/*
	 * 2. Make sure the DENALI_CTL_144[LPI_WAKEUP_EN[5:0]] has the bit LPI_WAKEUP_EN[3] = 1b'1.
	 * This enables the option 'self-refresh long with mem and ctlr clk gating or self-refresh
	 * power-down long with mem and ctlr clk gating'
	 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_144, BIT(3) << LPI_WAKEUP_EN_SHIFT);

	/*
	 * 3a. Config SIM_LPAV LPDDR_CTRL[LPDDR_AUTO_LP_MODE_DISABLE] to 1b'0(enable the logic to
	 * to automatic handles low power entry/exit. This is the recommended option over handling
	 * through software.
	 * 3b. Config the SIM_LPAV LPDDR_CTRL[SOC_LP_CMD] to 6b'101001(encoding for self_refresh with
	 * both DDR controller and DRAM clock gate. THis is mandatory since LPPDR logic will be power
	 * gated).
	 */
	mmio_clrbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL, LPDDR_AUTO_LP_MODE_DISABLE);
	mmio_clrsetbits_32(IMX_LPAV_SIM_BASE + LPDDR_CTRL, 0x3f << SOC_LP_CMD_SHIFT, 0x29 << SOC_LP_CMD_SHIFT);

	/* Save DDR Controller & PHY config.
	 * Set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=1. Read and store all the PHY registers
	 * for F2 into phy_f1_cfg, then read/store the diff between F1 & F2 into phy_f2_cfg.
	 */
	if (!dram_cfg_saved) {
		info = (struct dram_timing_info *)0x2006c000;
		/* original dram controller config should be used, so skip saving it */

		/* set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=1. Read and store all PHY registers. */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_SEL_INDEX(1) | PHY_FREQ_MULTICAST_EN(0));
		/* save phy f1 config */
		for (i = 0; i < info->phy_f1_cfg_num; i++)
			info->phy_f1_cfg[i].val = mmio_read_32(info->phy_f1_cfg[i].reg);

		/* set PHY_FREQ_SEL_MULTICAST_EN=0 & PHY_FREQ_SEL_INDEX=0. Read and store only the diff. */
		/* save phy f2 config */
		mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1537, PHY_FREQ_SEL_INDEX(0) | PHY_FREQ_MULTICAST_EN(0));
		for (i = 0; i < info->phy_f2_cfg_num; i++)
			info->phy_f2_cfg[i].val = mmio_read_32(info->phy_f2_cfg[i].reg);
		dram_cfg_saved = true;
	}
}

void dram_exit_retention(void)
{
	/* 1. Config the LPAV PLL4 and DDR clock for the desired LPDDR operating frequency. */
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, BIT(30));
	
	/* 2. Write PCC5.PCC_LPDDR4[SWRST] to 1b'1 to release LPDDR from reset. */ 
	mmio_setbits_32(IMX_PCC5_BASE + 0x108, BIT(28));

	/* 3. Reload the LPDDR CTL/PI/PHY register */
	ddr_init();

	/* 4a. Set PHY_SET_DFI_INPUT_N parameters to 4'h1. */
	mmio_write_32(IMX_DDRC_BASE + DENALI_PHY_1559, 0x01010101);

	/* 4b. Set PWRUP_SREFRESH_EXIT to 1b'1. This skips DRAM initialization since it was
	 * already in a known SR state.
	 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_93, PWRUP_SREFRESH_EXIT);  

	/* 4c. Set PHYMSTR_TRAIN_AFTER_INIT_COMPLETE to 1b'1 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_CTL_127, PHYMSTR_TRAIN_AFTER_INIT_COMPLETE);

	/* 4d. Set TSREF2PHYMSTR to 6h'01 */
	mmio_clrsetbits_32(IMX_DDRC_BASE + DENALI_CTL_23, 0x3f << TSREF2PHYMSTR_SHIFT, 0x1 << TSREF2PHYMSTR_SHIFT);

	/*
	 * 4e. Set PHY_LP4_BOOT_DISABLE to 1b'1; config DFIBUS_FREQ_INIT parameter with the
	 * same frequency number system was using before the power down.
	 */
	mmio_setbits_32(IMX_DDRC_BASE + DENALI_PHY_1547, PHY_LP4_BOOT_DISABLE);

	/*
	 * FIXME: normally, we use the frequency point config in dram ctl config(the frequency after calibration)
	 * so skip this step for now.
	 * mmio_clrsetbits_32(IMX_DDRC_BASE + DENALI_CTL_23, 0x3 << DFIBUS_FREQ_INIT_SHIFT, 0x2 << DFIBUS_FREQ_INIT_SHIFT);
	 */

	mmio_write_32(IMX_DDRC_BASE + DENALI_CTL_24, 0x1000200);

	/* 5. Disable automatic LP entry and PCPCS modes LP_AUTO_ENTRY_EN to 1b'0, PCPCS_PD_EN to 1b'0 */
	mmio_clrbits_32(IMX_DDRC_BASE + DENALI_CTL_147, 0xf);
	mmio_clrbits_32(IMX_DDRC_BASE + DENALI_CTL_153, PCPCS_PD_EN);

	/* 6. Write START parameter to 1b'1 */
	mmio_setbits_32(IMX_DDRC_BASE, BIT(0));

	/*
	 * 7. Ask for powersys_clear_ddrdr service from uPower FW, which will remove the high-Z
	 * from LPDDR I/Os and remove data retention from DDR_CKE and DDR_DRAM_RST_b I/Os.
	 */
	upwr_xcp_set_ddr_retention(APD_DOMAIN, 0, NULL);
	upower_wait_resp();
}
