/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <stdbool.h>
#include <ddrc.h>
#include <mmio.h>
#include <platform_def.h>
#include <spinlock.h>
#include <soc.h>

#include "lpddr4_dvfs.h"

#define SRC_IPS_BASE_ADDR       0x30390000
#define SRC_DDRC_RCR_ADDR	(SRC_IPS_BASE_ADDR + 0x1000)

#define GPC_PU_PWRHSK		(IMX_GPC_BASE + 0x01FC)  //added by baoxye
#define CCM_SRC_CTRL_OFFSET     (IMX_CCM_BASE + 0x800)
#define CCM_CCGR_OFFSET         (IMX_CCM_BASE + 0x4000)
#define CCM_SRC_CTRL(n)		(CCM_SRC_CTRL_OFFSET + 0x10 * n)
#define CCM_CCGR(n)		(CCM_CCGR_OFFSET + 0x10 * n)

static bool trained_csr_saved = false;

void ddrc_enter_retention(void)
{
	unsigned int tmp, tmp_t, i;

	/* only need be save once */
	if (!trained_csr_saved) {
		lpddr4_phy_save_trained_csr();
		trained_csr_saved = true;
	}

	INFO("enter lpddr4_retention\n");
	/* wait DBGCAM to be empty */
	do{
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
	while (tmp!=0x223) {
		tmp  = 0x33f & (mmio_read_32((DDRC_STAT(0))));
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	/* set SWCTL.sw_done to disable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);
	mmio_write_32(DDRC_DFIMISC(0), 0x00001f20);

	/* wait DFISTAT.dfi_init_complete to 0 */
	tmp_t = 0;
	while (tmp_t==1) {
		tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);

	/* wait DFISTAT.dfi_init_complete to 1 */
	tmp_t = 0;
	while (tmp_t==0) {
		 tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
	}

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000,0x0);
	tmp = dwc_ddrphy_apb_rd(0x90028);
	tmp = tmp & 0x1;
	if (tmp)
		INFO("C: PhyInLP3 = 1\n");
	else
		INFO("FAILED\n");

	dwc_ddrphy_apb_wr(0xd0000,0x1);

	for (i=0; i<20; i++);

	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 1));
	do {
	        tmp = mmio_read_32(GPC_PU_PWRHSK);
		 INFO("C: wait pwrdnackn_async clr\n");
	} while (tmp & (0x1<<18));/* wait untill pwrdnackn_async=0 */
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 1));

	/* remove PowerOk */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000008);

	for (i=0; i<20; i++);

	mmio_write_32(CCM_CCGR(5),0);
	mmio_write_32(CCM_SRC_CTRL(15),2);

	for (i=0; i<20; i++);

	/* enable the phy iso */
	mmio_setbits_32(0x303a0d40, 0x1);
	mmio_setbits_32(0x303a0104, (1 << 5));
}

void ddrc_exit_retention(void)
{
	volatile unsigned int tmp, tmp_t, i;
	INFO("exit lpddr4_retention\n");

	for (i=0; i<50; i++);

	for (i=0; i<50; i++);

	/* assert all reset */
	/* assert [0]src_system_rst_b! */
	mmio_write_32(SRC_DDRC_RCR_ADDR + 0x04, 0x8F000001);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F);
	/* release [0]src_system_rst_b! */
	mmio_write_32(SRC_DDRC_RCR_ADDR + 0x04, 0x8F000000);

	mmio_write_32(CCM_CCGR(5),2);
	mmio_write_32(CCM_SRC_CTRL(15),2);
	INFO("C: enable all DRAM clocks \n");

	/* change the clock source of dram_apb_clk_root */
	mmio_write_32(0x3038a088, (0x7<<24)|(0x7<<16));
	mmio_write_32(0x3038a084, (0x4<<24)|(0x3<<16));

	/* disable iso */
	/* mmio_write_32(0x303A00EC,0x0000ffff); //PGC_CPU_MAPPING */
	/* reg32setbit(0x303A00F8,5);//PU_PGC_SW_PUP_REQ */
	mmio_setbits_32(0x303a00f8, (1 << 5));

	/* release [0]ddr1_preset_n,  [3]ddr1_phy_pwrokin_n */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000006);

	lpddr4_cfg_umctl2();
	/* Skips the DRAM init routine and starts up in selfrefresh mode */
	/* Program INIT0.skip_dram_init = 2'b11 */
	mmio_write_32(DDRC_INIT0(0), 0xc0000000 | (mmio_read_32(DDRC_INIT0(0))));
	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000000); /* release all reset */
	mmio_write_32(DDRC_DBG1(0), 0x00000000);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DDR_SS_GPR0, 0x01);//LPDDR4 mode
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	lpddr4_phy_cfg();

	dwc_ddrphy_apb_wr(0xd0000,0x0);
	do {
		tmp_t = dwc_ddrphy_apb_rd(0x20097);
		INFO("C: Waiting for CalBusy = 0\n");
	} while(tmp_t != 0);
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020); 

	/* wait DFISTAT.dfi_init_complete to 1 */
	tmp_t = 0;
	while(tmp_t==0){
		tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
		INFO("wait DFISTAT.dfi_init_complete to 1\n");
	}

	/* clear DFIMISC.dfi_init_start */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);
	/* set DFIMISC.dfi_init_complete_en */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000001);
	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* wait SWSTAT.sw_done_ack to 1 */
	tmp_t = 0;
	while (tmp_t!=0x1) {
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x1;
		INFO("1wait SWSTAT.sw_done_ack to 1\n");
	}

	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000001);
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* wait SWSTAT.sw_done_ack to 1 */
	tmp_t = 0;
	while (tmp_t!=0x1) {
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x1;
		INFO("2wait SWSTAT.sw_done_ack to 1\n");
	}

	/* mmio_write_32(DDRC_PWRCTL(0), 0x0000008a); */
	mmio_write_32(DDRC_PWRCTL(0), 0x00000088);

	/* wait STAT to normal state */
	tmp_t = 0;
	while(tmp_t!=0x1){
		tmp  = mmio_read_32(DDRC_STAT(0));
		tmp_t = tmp & 0x7;
		INFO("wait STAT to normal state\n");
	}

	mmio_write_32(DDRC_DERATEEN(0), 0x00000302);

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001);
	/* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x00000000);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_rd(0x90028);
	tmp = tmp & 0x1;
	if(!tmp)
		 INFO("C: PhyInLP3 = 0\n");
	else
		INFO("FAILED\n");
	dwc_ddrphy_apb_wr(0xd0000, 0x1);
}
