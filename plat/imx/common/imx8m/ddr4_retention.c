/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <stdbool.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>
#include <platform_def.h>
#include <soc.h>

#define SRC_IPS_BASE_ADDR       IMX_SRC_BASE
#define SRC_DDRC_RCR_ADDR	(SRC_IPS_BASE_ADDR + 0x1000)

#define GPC_PU_PWRHSK		(IMX_GPC_BASE + 0x01FC)
#define CCM_SRC_CTRL_OFFSET     (IMX_CCM_BASE + 0x800)
#define CCM_CCGR_OFFSET         (IMX_CCM_BASE + 0x4000)
#define CCM_SRC_CTRL(n)		(CCM_SRC_CTRL_OFFSET + 0x10 * n)
#define CCM_CCGR(n)		(CCM_CCGR_OFFSET + 0x10 * n)

void ddr4_enter_retention(void)
{
	volatile unsigned int tmp;

	/* wait DBGCAM to be empty */
	do {
		tmp = mmio_read_32(DDRC_DBGCAM(0));
	} while (tmp != 0x36000000);

	/* Blocks AXI ports from taking anymore transactions */
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000000);

	/* Waits unit all AXI ports are idle */
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
	} while (tmp & 0x10001);

	/* enters self refresh */
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);

	tmp=0;
	/* self-refresh state */
	while (tmp!=0x23) {
		tmp  = 0x3f & (mmio_read_32((DDRC_STAT(0))));
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	/* set SWCTL.sw_done to disable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);
	mmio_write_32(DDRC_DFIMISC(0), 0x00001f20);

	/* wait DFISTAT.dfi_init_complete to 0 */
	while (1 == (0x1 & mmio_read_32(DDRC_DFISTAT(0))))
		;

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);

	/* wait DFISTAT.dfi_init_complete to 1 */
	while (0 == (0x1 & mmio_read_32(DDRC_DFISTAT(0))))
		;

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000,0x0);
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0x00090028);
	tmp = tmp & 0x1;
	if(tmp)
		printf("C: PhyInLP3 = 1\n");
	else
		printf("vt_fail\n");
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 2));
	do {
		tmp = mmio_read_32(GPC_PU_PWRHSK);
	} while (tmp & (0x1 << 20)); /* wait untill pwrdnackn_async=0 */
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 2));

	/* remove PowerOk: assert [3]ddr1_phy_pwrokin_n */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000008);

	mmio_write_32(CCM_CCGR(5), 0);
	mmio_write_32(CCM_SRC_CTRL(15), 2);

	mmio_setbits_32(0x303A0D40, 1);
	mmio_setbits_32(0x303A0104, (1 << 5));
}

void ddr4_exit_retention(void)
{
	volatile unsigned int tmp, tmp_t, i;

	for (i = 0; i < 20; i++){
	}

	/* assert all reset */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00003F);

	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F); /* release [4]src_system_rst_b! */

	mmio_write_32(CCM_CCGR(5), 2);
	mmio_write_32(CCM_SRC_CTRL(15), 2);
	printf("C: enable all DRAM clocks \n");

	mmio_write_32(0x303A00EC, 0x0000ffff); /* PGC_CPU_MAPPING */
	mmio_setbits_32(0x303A00F8, (1 << 5));

	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000006); /* release [0]ddr1_preset_n, [3]ddr1_phy_pwrokin_n */
	/* RESET: <core_ddrc_rstn> ASSERTED (ACTIVE LOW) */
	/* RESET: <presetn> ASSERTED (ACTIVE LOW) */
	/* RESET: <aresetn> for Port 0  ASSERTED (ACTIVE LOW) */
	/* RESET: <presetn> DEASSERTED */

	/* wait dram pll locked */
	while(!(mmio_read_32(DRAM_PLL_CTRL) & (1 << 31)))
		;

	mmio_write_32(DDRC_DBG1(0), 0x00000001);
	mmio_write_32(DDRC_PWRCTL(0), 0x00000001);
	while (0 != (0x7 & mmio_read_32(DDRC_STAT(0))))
		;

	/* ddrc init */
	dram_umctl2_init();

	/* Skips the DRAM init routine and starts up in selfrefresh mode */
	/* Program INIT0.skip_dram_init = 2'b11 */
	mmio_write_32(DDRC_INIT0(0), 0xc0000000 | (mmio_read_32(DDRC_INIT0(0))));
	mmio_write_32(DDRC_MSTR2(0), 0x0);

	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0x000001a8);
	mmio_clrbits_32(DDRC_DFIMISC(0), 1);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000004);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000000); /* release all reset */
	mmio_write_32(DDRC_DBG1(0), 0x00000000);

	/* restore the ddr phy config */
	dram_phy_init();

	do {
		tmp_t = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0x00020097);
	} while (tmp_t != 0);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020);

	/* wait DFISTAT.dfi_init_complete to 1 */
	while(0 == (0x1 & mmio_read_32(DDRC_DFISTAT(0))))
		;

	/* clear DFIMISC.dfi_init_complete_en */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);
	/* set DFIMISC.dfi_init_complete_en again */
	mmio_setbits_32(DDRC_DFIMISC(0), 1);
	mmio_clrbits_32(DDRC_PWRCTL(0), (1 << 5));

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);
	/* wait SWSTAT.sw_done_ack to 1 */
	while(0 == (0x1 & mmio_read_32(DDRC_SWSTAT(0))));

	/* wait STAT to normal state */
	while(0x1 != (0x7 & mmio_read_32(DDRC_STAT(0))));

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001);
	/* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x00000000);

	/* should check PhyInLP3 pub reg */
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0xd0000, 0x0);
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0x00090028);
	tmp = tmp & 0x1;
	if (!tmp)
		printf("C: PhyInLP3 = 0\n");
	else
		printf("vt_fail\n");
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0xd0000, 0x1);
}
