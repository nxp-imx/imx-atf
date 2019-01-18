/*
 * Copyright 2018 NXP
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

void lpddr4_enter_retention(void)
{
	unsigned int tmp, tmp_t, i;
	INFO("enter lpddr4 retention\n");

	/* wait DBGCAM to be empty */
	do {
		tmp = mmio_read_32(DDRC_DBGCAM(0));
	} while(tmp != 0x36000000);

	/* Blocks AXI ports from taking anymore transactions */
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000000);

	/* Waits unit all AXI ports are idle */
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
	} while(tmp & 0x10001);

	/* enters self refresh */
	//reg32setbit(DDRC_PWRCTL(0), 5);
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);

	tmp=0;
	/* self-refresh state */
	while (tmp!=0x223) {
		tmp  = 0x33f & (mmio_read_32((DDRC_STAT(0))));
		INFO("C: waiting for STAT selfref_type= Self Refresh\n");
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
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0x00090028);
	tmp = tmp & 0x1;
	if (tmp)
		INFO("C: PhyInLP3 = 1\n");
	else
		INFO("vt_fail5\n");
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	for (i=0; i<20; i++){
 	}

#if defined(PLAT_IMX8M)
	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 1));
	do {
		tmp = mmio_read_32(GPC_PU_PWRHSK);
	//        printf("C: wait pwrdnackn_async clr\n");
	} while(tmp&(0x1<<18));//wait untill pwrdnackn_async=0
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 1));
#else
	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 2));
	do {
		tmp = mmio_read_32(GPC_PU_PWRHSK);
		INFO("C: wait pwrdnackn_async clr\n");
	} while(tmp&(0x1<<20));//wait untill pwrdnackn_async=0
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 2));
#endif
	/* remove PowerOk */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000008); 
	INFO("vt_event1\n");

	for (i=0; i<20; i++) {
	}

	mmio_write_32(CCM_CCGR(5),0);
	mmio_write_32(CCM_SRC_CTRL(15),2);// when test on chip, this should be 0x2
	INFO("C: gated all DRAM clocks \n");

	for (i=0; i<20; i++) {
	}
	/* enable the phy iso */
	mmio_setbits_32(0x303a0d40, 1);
	mmio_setbits_32(0x303a0104, (1 << 5));
}

void lpddr4_exit_retention(void)
{
	unsigned int tmp, tmp_t, i;

	INFO("exit lpddr4 retention\n");

	for (i=0; i<50; i++) {
	}

	for (i=0; i<50; i++) {
	}

	/*assert all reset */
#if defined(PLAT_IMX8M)
	mmio_write_32(SRC_DDRC_RCR_ADDR+0x4, 0x8F000003); // assert [0]src_system_rst_b!
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n,
	mmio_write_32(SRC_DDRC_RCR_ADDR+0x4, 0x8F000000); // deassert  [4]src_system_rst_b!
#else
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00001F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n
#endif

	mmio_write_32(CCM_CCGR(5),2);
	mmio_write_32(CCM_SRC_CTRL(15),2);
	INFO("C: enable all DRAM clocks \n");

	/* change the clock source of dram_apb_clk_root */
	mmio_write_32(0x3038a088, (0x7<<24)|(0x7<<16));
	mmio_write_32(0x3038a084, (0x4<<24)|(0x3<<16));

	/* disable iso */
	mmio_write_32(0x303A00EC,0x0000ffff); //PGC_CPU_MAPPING
	mmio_setbits_32(0x303a00f8, (1 << 5));

	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000006); // release [0]ddr1_preset_n,  [3]ddr1_phy_pwrokin_n

	/* wait dram pll locked */
	while(!(mmio_read_32(DRAM_PLL_CTRL) & (1 << 31)))
		;

	/* ddrc re-init */
	dram_umctl2_init();

	/*
	 * Skips the DRAM init routine and starts up in selfrefresh mode
	 * Program INIT0.skip_dram_init = 2'b11
	 */
	mmio_write_32(DDRC_INIT0(0), 0xc0000000 | (mmio_read_32(DDRC_INIT0(0))));
	mmio_write_32(DDRC_MSTR2(0), 0x0);
	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000004); /* release all reset */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000000); /* release all reset */
	mmio_write_32(DDRC_DBG1(0), 0x00000000);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DDR_SS_GPR0, 0x01); /*LPDDR4 mode */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	/* dram phy re-init */
	dram_phy_init();

	INFO("ddrphy config to 3000mts\n");

	/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	dwc_ddrphy_apb_wr(0xd0000,0x0); 
	do {
		tmp_t = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0x00020097); 
		INFO("C: Waiting for CalBusy = 0x%x\n",tmp_t);
	} while (tmp_t != 0);
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020); 

	/* wait DFISTAT.dfi_init_complete to 1 */
	tmp_t = 0;
	while (tmp_t==0) {
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
	while(tmp_t!=0x1){
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
	while(tmp_t!=0x1){
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x1;
		INFO("2wait SWSTAT.sw_done_ack to 1\n");
	}

	mmio_write_32(DDRC_PWRCTL(0), 0x00000088); 

	/* wait STAT to normal state */
	tmp_t = 0;
	while(tmp_t!=0x1) {
		tmp  = mmio_read_32(DDRC_STAT(0));
		tmp_t = tmp & 0x7;
		INFO("wait STAT to normal state\n");
	}
 
	mmio_write_32(DDRC_DERATEEN(0), 0x00000302);

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001); 
	 /* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x00000000);

	/* should check PhyInLP3 pub reg */
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0xd0000,0x0);
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0x00090028);
	tmp = tmp & 0x1;
	if(!tmp)
		INFO("C: PhyInLP3 = 0\n");
	else
		INFO("vt_fail6\n");
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0xd0000,0x1);

}
