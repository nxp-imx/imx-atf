/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ddrc.h>
#include <debug.h>
#include <mmio.h>

#include "lpddr4_dvfs.h"

#define P0_INIT3 0x00D4002D
#define P0_INIT4 0x00310008
#define P0_INIT6 0x0066004a
#define P0_INIT7 0x0006004a

#define P1_INIT3 0x0140009
#define P1_INIT4 0x00310008
#define P1_INIT6 0x0066004a
#define P1_INIT7 0x0006004a

#define HW_DRAM_PLL_CFG0_ADDR     (0x30360000 + 0x60)
#define HW_DRAM_PLL_CFG1_ADDR     (0x30360000 + 0x64)
#define HW_DRAM_PLL_CFG2_ADDR     (0x30360000 + 0x68)

#define DFILP_SPT

void sscgpll_bypass_enable(unsigned int reg_addr)
{
	unsigned int read_data;
	read_data = mmio_read_32(reg_addr);
	mmio_write_32(reg_addr, read_data | 0x00000010);
	read_data = mmio_read_32(reg_addr);
	mmio_write_32(reg_addr, read_data | 0x00000020);
}

void sscgpll_bypass_disable(unsigned int reg_addr)
{
	unsigned int read_data;
	read_data = mmio_read_32(reg_addr);
	mmio_write_32(reg_addr, read_data & 0xffffffdf);
	read_data = mmio_read_32(reg_addr);
	mmio_write_32(reg_addr, read_data & 0xffffffef);
}

unsigned int wait_pll_lock(unsigned int reg_addr)
{
	unsigned int pll_lock;
	pll_lock = mmio_read_32(reg_addr) >> 31;
	return pll_lock;
}

void DDR_PLL_CONFIG_FREQ(unsigned int freq) 
{
	unsigned int ddr_pll_lock = 0x0;
	sscgpll_bypass_enable(HW_DRAM_PLL_CFG0_ADDR);
	switch(freq)
	{
		case 800:
			mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x00ece580);
			break;
		case 700:
			mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x00ec4580);
			break;
		case 667:
			mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x00ece480);
			break;
	        case 400:
	                mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x00ec6984);
			break;
	        case 167:
			mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x00f5a406);
			break;
	        case 100:
	                mmio_write_32(HW_DRAM_PLL_CFG2_ADDR, 0x015dea96);
			break;

		default:
			printf("Input freq=%d error.\n",freq);
	}

	sscgpll_bypass_disable(HW_DRAM_PLL_CFG0_ADDR);
	while (ddr_pll_lock != 0x1)
	{
		ddr_pll_lock = wait_pll_lock(HW_DRAM_PLL_CFG0_ADDR);
	}
}

void lpddr4_mr_write(unsigned int  mr_rank, unsigned int mr_addr, unsigned int mr_data)
{
	unsigned int tmp;
	/*1. Poll MRSTAT.mr_wr_busy until it is 0. This checks
	 * that there is no outstanding MR transaction. No writes
	 * should be performed to MRCTRL0 and MRCTRL1 if MRSTAT.mr_wr_busy = 1.
	 */
	do{
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while (tmp & 0x1);
	/* 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank
	 * and (for MRWs) MRCTRL1.mr_data to define the MR transaction.
 	 */
	mmio_write_32(DDRC_MRCTRL0(0), (mr_rank << 4));
	mmio_write_32(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	mmio_setbits_32(DDRC_MRCTRL0(0), (1 << 31));
}

void lpddr4_dvfs_swffc(unsigned int init_fsp, unsigned int target_freq)
{
	unsigned int mr, emr, emr2, emr3;//, mr4;
	unsigned int mr11, mr12, mr22, mr14;
	unsigned int tmp, tmp_t, tmp2, i;

	/* MR13.FSP-WR=1, MRW to update the MR registers */

	/* setting for 3200 mts */
	if(target_freq==0) {
		mr   = P0_INIT3 >> 16;
		emr  = P0_INIT3 & 0xFFFF;
		emr2 = P0_INIT4 >> 16;
		emr3 = P0_INIT4 & 0xFFFF;
		mr11 = P0_INIT6 >> 16;
		mr12 = P0_INIT6 & 0xFFFF;
		mr22 = P0_INIT7 >> 16;
		mr14 = P0_INIT7 & 0xFFFF;
	} else {
	/* setting for 667 mts */
		mr   = P1_INIT3 >> 16;
		emr  = P1_INIT3 & 0xFFFF;
		emr2 = P1_INIT4 >> 16;
		emr3 = P1_INIT4 & 0xFFFF;
		mr11 = P1_INIT6 >> 16;
		mr12 = P1_INIT6 & 0xFFFF;
		mr22 = P1_INIT7 >> 16;
		mr14 = P1_INIT7 & 0xFFFF;
	}

	tmp = (init_fsp== 1) ? 0x2 <<6 : 0x1 <<6;
	emr3 = (emr3 & 0x003f) | tmp | 0x0d00;

	/* set PWRCTL.selfref_en=0 */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~0xf; 
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	lpddr4_mr_write(3, 13, emr3);
	lpddr4_mr_write(3, 1, mr);
	lpddr4_mr_write(3, 2, emr);
	lpddr4_mr_write(3, 3, emr2);
	lpddr4_mr_write(3, 11, mr11);
	lpddr4_mr_write(3, 12, mr12);
	lpddr4_mr_write(3, 14, mr14);
	lpddr4_mr_write(3, 22, mr22);

	/* polling mr done */
	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while(tmp & 0x1);

	/* disable AXI ports */
	mmio_write_32(DDRC_PCTRL_0(0), 0x0);

	/* Poll PSTAT.rd_port_busy_n=0 and PSTAT.wr_port_busy_n=0. */
	i=0;
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
		i++;
	} while ((tmp != 0) && (i < 100));

	/* Disable phy master */
	mmio_write_32(DDRC_DFIPHYMSTR(0),0x00000000);


#ifdef DFILP_SPT 
	/* 8. disable DFI LP */
	/* DFILPCFG0.dfi_lp_en_sr */
	tmp = mmio_read_32(DDRC_DFILPCFG0(0));
	if (tmp & 0x100) { 
		tmp &= 0x0;
		mmio_write_32(DDRC_DFILPCFG0(0),tmp);
		do {
			tmp = mmio_read_32(DDRC_DFISTAT(0)); /* dfi_lp_ack */
			tmp2= mmio_read_32(DDRC_STAT(0)); /* operating_mode */
		} while (((tmp & 0x2) == 0x2) && ((tmp2 & 0x7) == 3));
	}
#endif

	/* 9. wait until in normal or power down states */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); /* operating_mode */
	} while (((tmp & 0x7) != 1) && ((tmp & 0x7) != 2));

	/* 10. Disable automatic derating: derate_enable */
	tmp= mmio_read_32(DDRC_DERATEEN(0));
	tmp &= ~0x1;
	mmio_write_32(DDRC_DERATEEN(0), tmp); /* ddrc_derate_enable, bit 0 */

	tmp= mmio_read_32(DDRC_FREQ1_DERATEEN(0));
	tmp &= ~0x1;
	mmio_write_32(DDRC_FREQ1_DERATEEN(0), tmp); /* ddrc_derate_enable, bit 0 */
	tmp= mmio_read_32(DDRC_ZQCTL0(0));
	tmp |= 0x80000000;
	mmio_write_32(DDRC_ZQCTL0(0), tmp); /* ddrc_derate_enable, bit 0 */
	tmp= mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
	tmp |= 0x80000000;
	mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp); /* ddrc_derate_enable, bit 0 */

	/* 12. set PWRCTL.selfref_en=0 */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~0x1; 
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	/* 13.Poll STAT.operating_mode is in "Normal" (001) or "Power-down" (010) */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); /* operating_mode */
	} while (((tmp & 0x7) != 1) && ((tmp & 0x7) != 2));

	/* 14/15. trigger SW SR */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp |= 0x60;
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	/* 16. Poll STAT.selfref_state in "Self Refresh 1" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); // operating_mode
	} while ((tmp & 0x300) != 0x100);

	/* 17. disable dq */
	tmp= mmio_read_32(DDRC_DBG1(0));
	tmp |= 0x1;
	mmio_write_32(DDRC_DBG1(0), tmp);

	/* 18. Poll DBGCAM.wr_data_pipeline_empty and DBGCAM.rd_data_pipeline_empty */
	do {
		tmp  = mmio_read_32(DDRC_DBGCAM(0));
		tmp_t = tmp & 0x30000000;
	} while(tmp_t!=0x30000000);

	/* 19. change MR13.FSP-OP to new FSP and MR13.VRCG to high current */
	emr3 = (((~init_fsp) & 0x1) << 7) | (0x1 << 3) | (emr3 & 0x0077) | 0x0d00;
	lpddr4_mr_write(3, 13, emr3);

	/* 20. enter SR Power Down */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~0x60;
	tmp |= 0x20;
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	/* 21. Poll STAT.selfref_state is in "SR Power down" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); 
	} while ((tmp & 0x300) != 0x200);

	/* 22. set dfi_init_complete_en = 0 */
	/* 23. switch clock */
	/* set SWCTL.dw_done to 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0000);

	/* 24. program frequency mode=1(bit 29), target_frequency=target_freq (bit 29) */

	mmio_write_32(DDRC_MSTR2(0),target_freq);

	/* 25. DBICTL for FSP-OP[1], skip it if never enable it */

	/* 26.trigger initialization in the PHY */

	/* Q3: if refresh level is updated, then should program */
	tmp= mmio_read_32(DDRC_RFSHCTL3(0));
	tmp = tmp ^ 0x2;
	mmio_write_32(DDRC_RFSHCTL3(0),tmp);

	/* Q4: only for legacy PHY, so here can skipped */
	tmp= mmio_read_32(DDRC_DFIMISC(0));
	tmp &= 0xFF; 
	tmp |= (target_freq << 8);  /* should be 0 */
	mmio_write_32(DDRC_DFIMISC(0),tmp);
	tmp |= 0x20;
	mmio_write_32(DDRC_DFIMISC(0),tmp);

	/* polling dfi_init_complete de-assert */
	do {
		tmp = mmio_read_32(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x1);

	/* switch_clocks for target frequency */
	if(target_freq==0x1) {
	/*	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1),(0x7<<24)|(0x7<<16)); */
		mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
	/*	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1),(0x4<<24)|(0x4<<16)); */
		mmio_write_32(0x3038a084, (0x4 << 24) | (0x4 << 16)); //to source 4 --800MHz/5
		DDR_PLL_CONFIG_FREQ(167);
	} else {
		DDR_PLL_CONFIG_FREQ(800);
	/*	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1),(0x7<<24)|(0x7<<16)); */
		mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
	/*	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1),(0x4<<24)|(0x3<<16)); */
		mmio_write_32(0x3038a084, (0x4 << 24) | (0x3 << 16)); //to source 4 --800MHz/4*/
	}

	/* dfi_init_start de-assert */
	tmp= mmio_read_32(DDRC_DFIMISC(0));
	tmp &= ~0x20;
	mmio_write_32(DDRC_DFIMISC(0),tmp);

	/* polling dfi_init_complete re-assert */
	do {
		tmp = mmio_read_32(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x0);
  
	/* 27. set ZQCTL0.dis_srx_zqcl = 1 */
	if (target_freq ==0) {
		tmp = mmio_read_32(DDRC_ZQCTL0(0));
		tmp |= 0x40000000;
		mmio_write_32(DDRC_ZQCTL0(0), tmp);
	} else {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp |= 0x40000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	}

	/* 28,29. exit "self refresh power down" to stay "self refresh 2" */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~(0x60);
	tmp |= 0x40; 
	mmio_write_32(DDRC_PWRCTL(0),tmp);
	/* 30. Poll STAT.selfref_state in "Self refresh 2" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); // operating_mode
	} while ((tmp & 0x300)!=0x300);

	/* 31. change MR13.VRCG to normal */
	emr3 = (emr3 & 0x00f7) | 0x0d00;
	lpddr4_mr_write(3, 13, emr3);

	/* enable PHY master */
	mmio_write_32(DDRC_DFIPHYMSTR(0), 0x1);

	/* 32. issue ZQ if required: zq_calib_short, bit 4 */
	/* polling zq_calib_short_busy */
	tmp= mmio_read_32(DDRC_DBGCMD(0));
	tmp |= 0x10; 
	mmio_write_32(DDRC_DBGCMD(0),tmp);
	do {
		tmp= mmio_read_32(DDRC_DBGSTAT(0));
	} while ((tmp & 0x10 ) != 0x0);

	/* 33. Reset ZQCTL0.dis_srx_zqcl=0 */
	if (target_freq > 0) {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp &= ~0x40000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	} else {
		tmp = mmio_read_32(DDRC_ZQCTL0(0));
		tmp &= ~0x40000000;
		mmio_write_32(DDRC_ZQCTL0(0), tmp);
	}

	/* set SWCTL.dw_done to 1 and poll SWSTAT.sw_done_ack=1 */
	mmio_write_32(DDRC_SWCTL(0), 0x0001); 

	/* wait SWSTAT.sw_done_ack to 1 */
	tmp_t = 0;
	while(tmp_t==0) {
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x01;
	}

	/* 34. set PWRCTL.stay_in_selfreh=0, exit SR */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~(0x40);
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	/* 35. Poll STAT.selfref_state in "Idle" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0)); // operating_mode
	} while ((tmp & 0x300)!=0x0);

#ifdef DFILP_SPT 
	/* 36. restore dfi_lp.dfi_lp_en_sr */
	tmp = mmio_read_32(DDRC_DFILPCFG0(0));
	tmp |= 0x100;
	mmio_write_32(DDRC_DFILPCFG0(0),tmp);
#endif

	/* 37. re-enable CAM: dis_dq */
	tmp= mmio_read_32(DDRC_DBG1(0));
	tmp &= 0xFFFFFFFE;
	mmio_write_32(DDRC_DBG1(0), tmp);

	/* 38. re-enable automatic SR: selfref_en */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp |= 0x1;
	mmio_write_32(DDRC_PWRCTL(0), tmp);

	/* 39. re-enable automatic ZQ: dis_auto_zq=0 */
	/* disable automatic ZQ calibration */
	if (target_freq == 1) {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp &= ~0x80000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	} else {
		tmp = mmio_read_32(DDRC_ZQCTL0(0));
		tmp &= ~0x80000000;
		mmio_write_32(DDRC_ZQCTL0(0), tmp);
	}

	/* 40. re-emable automatic derating: derate_enable */
	tmp= mmio_read_32(DDRC_DERATEEN(0));
	tmp &= 0xFFFFFFFE;
	mmio_write_32(DDRC_DERATEEN(0), tmp);

	/* 41. write 1 to PCTRL.port_en */
	mmio_write_32(DDRC_PCTRL_0(0), 0x1);
}
