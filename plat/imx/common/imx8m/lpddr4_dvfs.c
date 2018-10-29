/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>

#include "lpddr4_define.h"

extern void dram_clock_switch(unsigned target_freq);

void lpddr4_swffc(struct dram_info *info, unsigned int init_fsp,
	 unsigned int tgt_freq, bool bypass_mode)

{
	unsigned int mr, emr, emr2, emr3;
	unsigned int mr11, mr12, mr22, mr14;
	unsigned int tmp;

	/* 1. program targetd UMCTL2_REGS_FREQ1/2/3,already done, skip it. */

	/* 2. MR13.FSP-WR=1, MRW to update MR registers */
	if (tgt_freq == 0) {
		mr   = mmio_read_32(DDRC_INIT3(0)) >> 16;
		emr  = mmio_read_32(DDRC_INIT3(0)) & 0xFFFF;
		emr2 = mmio_read_32(DDRC_INIT4(0)) >> 16;
		emr3 = mmio_read_32(DDRC_INIT4(0)) & 0xFFFF;
		mr11 = mmio_read_32(DDRC_INIT6(0)) >> 16;
		mr12 = mmio_read_32(DDRC_INIT6(0)) & 0xFFFF;
		mr22 = mmio_read_32(DDRC_INIT7(0)) >> 16;
		mr14 = mmio_read_32(DDRC_INIT7(0)) & 0xFFFF;
	} else if (tgt_freq == 1) {
		mr   = mmio_read_32(DDRC_FREQ1_INIT3(0)) >> 16;
		emr  = mmio_read_32(DDRC_FREQ1_INIT3(0)) & 0xFFFF;
		emr2 = mmio_read_32(DDRC_FREQ1_INIT4(0)) >> 16;
		emr3 = mmio_read_32(DDRC_FREQ1_INIT4(0)) & 0xFFFF;
		mr11 = mmio_read_32(DDRC_FREQ1_INIT6(0)) >> 16;
		mr12 = mmio_read_32(DDRC_FREQ1_INIT6(0)) & 0xFFFF;
		mr22 = mmio_read_32(DDRC_FREQ1_INIT7(0)) >> 16;
		mr14 = mmio_read_32(DDRC_FREQ1_INIT7(0)) & 0xFFFF;
	} else {
		mr   = mmio_read_32(DDRC_FREQ2_INIT3(0)) >> 16;
		emr  = mmio_read_32(DDRC_FREQ2_INIT3(0)) & 0xFFFF;
		emr2 = mmio_read_32(DDRC_FREQ2_INIT4(0)) >> 16;
		emr3 = mmio_read_32(DDRC_FREQ2_INIT4(0)) & 0xFFFF;
		mr11 = mmio_read_32(DDRC_FREQ2_INIT6(0)) >> 16;
		mr12 = mmio_read_32(DDRC_FREQ2_INIT6(0)) & 0xFFFF;
		mr22 = mmio_read_32(DDRC_FREQ2_INIT7(0)) >> 16;
		mr14 = mmio_read_32(DDRC_FREQ2_INIT7(0)) & 0xFFFF;
	}

	tmp = (init_fsp == 1) ? 0x2 << 6 : 0x1 << 6;
	emr3 = (emr3 & 0x003f) | tmp | 0x0d00;

	/* 12. set PWRCTL.selfref_en=0 */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~0xf; 
	mmio_write_32(DDRC_PWRCTL(0), tmp);

	/* more safe */
	mmio_write_32(DDRC_DFIPHYMSTR(0), 0x00000000);

	lpddr4_mr_write(3, 13, emr3);
	lpddr4_mr_write(3, 1, mr);
	lpddr4_mr_write(3, 2, emr);
	lpddr4_mr_write(3, 3, emr2);
	lpddr4_mr_write(3, 11, mr11);
	lpddr4_mr_write(3, 12, mr12);
	lpddr4_mr_write(3, 14, mr14);
	lpddr4_mr_write(3, 22, mr22);

	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	/* 3. disable AXI ports */
	mmio_write_32(DDRC_PCTRL_0(0), 0x0);

	/* 4.Poll PSTAT.rd_port_busy_n=0 and PSTAT.wr_port_busy_n=0. */
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
	} while (tmp != 0);

	/* 6.disable SBRCTL.scrub_en, skip if never enable it */
	/* 7.poll SBRSTAT.scrub_busy  Q2: should skip phy master if never enable it */
	/* Disable phy master */

#ifdef DFILP_SPT 
	/* 8. disable DFI LP */
	/* DFILPCFG0.dfi_lp_en_sr */
	tmp = mmio_read_32(DDRC_DFILPCFG0(0));
	if (tmp & 0x100) { 
		mmio_write_32(DDRC_DFILPCFG0(0), 0x0);
		do {
			tmp = mmio_read_32(DDRC_DFISTAT(0)); // dfi_lp_ack
			tmp2= mmio_read_32(DDRC_STAT(0)); // operating_mode
		} while (((tmp & 0x2) == 0x2) && ((tmp2 & 0x7) == 3));
	}
#endif
	/* 9. wait until in normal or power down states */
	do {
		/* operating_mode */
		tmp= mmio_read_32(DDRC_STAT(0));
	} while (((tmp & 0x7) != 1) && ((tmp & 0x7) != 2));

	/* 10. Disable automatic derating: derate_enable */
	tmp= mmio_read_32(DDRC_DERATEEN(0));
	tmp &= ~0x1;
	mmio_write_32(DDRC_DERATEEN(0), tmp);

	tmp= mmio_read_32(DDRC_FREQ1_DERATEEN(0));
	tmp &= ~0x1;
	mmio_write_32(DDRC_FREQ1_DERATEEN(0), tmp);

	tmp= mmio_read_32(DDRC_FREQ2_DERATEEN(0));
	tmp &= ~0x1;
	mmio_write_32(DDRC_FREQ2_DERATEEN(0), tmp);

	/* 11. disable automatic ZQ calibration */
	tmp= mmio_read_32(DDRC_ZQCTL0(0));
	tmp |= 0x80000000;
	mmio_write_32(DDRC_ZQCTL0(0), tmp);

	tmp= mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
	tmp |= 0x80000000;
	mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);

	tmp= mmio_read_32(DDRC_FREQ2_ZQCTL0(0));
	tmp |= 0x80000000;
	mmio_write_32(DDRC_FREQ2_ZQCTL0(0), tmp);

	/* 12. set PWRCTL.selfref_en=0 */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~0x1; 
	mmio_write_32(DDRC_PWRCTL(0), tmp);

	/* 13.Poll STAT.operating_mode is in "Normal" (001) or "Power-down" (010) */
	do {
		/* operating_mode */
		tmp= mmio_read_32(DDRC_STAT(0));
	} while (((tmp & 0x7) != 1) && ((tmp & 0x7) != 2));

	/* 14-15. trigger SW SR */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
 	/* bit 5: selfref_sw, bit 6: stay_in_selfref */
	tmp |= 0x60;
	mmio_write_32(DDRC_PWRCTL(0),tmp);

	/* 16. Poll STAT.selfref_state in "Self Refresh 1" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0));
	} while ((tmp & 0x300) != 0x100);

	/* 17. disable dq */
	tmp= mmio_read_32(DDRC_DBG1(0));
	tmp |= 0x1;
	mmio_write_32(DDRC_DBG1(0), tmp);

	/* 18. Poll DBGCAM.wr_data_pipeline_empty and DBGCAM.rd_data_pipeline_empty */
	do {
		tmp  = mmio_read_32(DDRC_DBGCAM(0));
		tmp &= 0x30000000;
	} while (tmp != 0x30000000);

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
	mmio_write_32(DDRC_MSTR2(0), tgt_freq);

	/* 25. DBICTL for FSP-OP[1], skip it if never enable it */

	/* 26.trigger initialization in the PHY */

	/* Q3: if refresh level is updated, then should program */
	/* as updating refresh, need to toggle refresh_update_level signal */
	tmp= mmio_read_32(DDRC_RFSHCTL3(0));
	tmp = tmp ^ 0x2;
	mmio_write_32(DDRC_RFSHCTL3(0), tmp);

	/* Q4: only for legacy PHY, so here can skipped */

	/* dfi_frequency -> 0x1x */
	tmp= mmio_read_32(DDRC_DFIMISC(0));
	tmp &= 0xFE; 
	tmp |= (tgt_freq << 8);
	mmio_write_32(DDRC_DFIMISC(0), tmp);
	/* dfi_init_start */
	tmp |= 0x20;
	mmio_write_32(DDRC_DFIMISC(0), tmp);

	/* polling dfi_init_complete de-assert */
	do {
		tmp = mmio_read_32(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x1);

	/* change the clock frequency */
#if defined(PLAT_IMX8M)
	if (bypass_mode)
		dram_clock_switch(tgt_freq);
	else
		dram_pll_init(info->timing_info->fsp_table[tgt_freq]);
#else
	dram_clock_switch(tgt_freq);
#endif

	/* dfi_init_start de-assert */
	tmp= mmio_read_32(DDRC_DFIMISC(0));
	tmp &= ~0x20;
	mmio_write_32(DDRC_DFIMISC(0),tmp);

	/* polling dfi_init_complete re-assert */
	do {
		tmp = mmio_read_32(DDRC_DFISTAT(0));
	} while ((tmp & 0x1) == 0x0);
  
	/* 27. set ZQCTL0.dis_srx_zqcl = 1 */
	if (tgt_freq == 0) {
		tmp = mmio_read_32(DDRC_ZQCTL0(0));
		tmp |= 0x40000000;
		mmio_write_32(DDRC_ZQCTL0(0), tmp);
	} else  if (tgt_freq == 1) {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp |= 0x40000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	} else {
		tmp = mmio_read_32(DDRC_FREQ2_ZQCTL0(0));
		tmp |= 0x40000000;
		mmio_write_32(DDRC_FREQ2_ZQCTL0(0), tmp);
	}

	/* 28,29. exit "self refresh power down" to stay "self refresh 2" */
	/* exit SR power down */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~(0x60);
	tmp |= 0x40; 
	mmio_write_32(DDRC_PWRCTL(0), tmp);
	/* 30. Poll STAT.selfref_state in "Self refresh 2" */
	do {
		tmp= mmio_read_32(DDRC_STAT(0));
	} while ((tmp & 0x300) != 0x300);

	/* 31. change MR13.VRCG to normal */
	emr3 = (emr3 & 0x00f7) | 0x0d00;
	lpddr4_mr_write(3, 13, emr3);

	/* enable PHY master */
	mmio_write_32(DDRC_DFIPHYMSTR(0), 0x1);

	/* 32. issue ZQ if required: zq_calib_short, bit 4 */
	/* polling zq_calib_short_busy */
	tmp= mmio_read_32(DDRC_DBGCMD(0));
	tmp |= 0x10; 
	mmio_write_32(DDRC_DBGCMD(0), tmp);

	do {
		tmp= mmio_read_32(DDRC_DBGSTAT(0));
	} while ((tmp & 0x10 ) != 0x0);

	/* 33. Reset ZQCTL0.dis_srx_zqcl=0 */
	if (tgt_freq == 1) {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp &= ~0x40000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	}  else if (tgt_freq == 2) {
		tmp = mmio_read_32(DDRC_FREQ2_ZQCTL0(0));
		tmp &= ~0x40000000;
		mmio_write_32(DDRC_FREQ2_ZQCTL0(0), tmp);
	} else {
		tmp = mmio_read_32(DDRC_ZQCTL0(0));
		tmp &= ~0x40000000;
		mmio_write_32(DDRC_ZQCTL0(0), tmp);
	}

	/* set SWCTL.dw_done to 1 and poll SWSTAT.sw_done_ack=1 */
	mmio_write_32(DDRC_SWCTL(0), 0x0001); 

	/* wait SWSTAT.sw_done_ack to 1 */
	do {
		tmp = mmio_read_32(DDRC_SWSTAT(0));
	} while ((tmp & 0x1) == 0x0);

	/* 34. set PWRCTL.stay_in_selfreh=0, exit SR */
	tmp= mmio_read_32(DDRC_PWRCTL(0));
	tmp &= ~(0x40);
	mmio_write_32(DDRC_PWRCTL(0),tmp);
	/* wait tXSR */

	/* 35. Poll STAT.selfref_state in "Idle" */
	do {
		tmp = mmio_read_32(DDRC_STAT(0));
	} while ((tmp & 0x300) != 0x0);

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
	if (tgt_freq == 1) {
		tmp = mmio_read_32(DDRC_FREQ1_ZQCTL0(0));
		tmp &= ~0x80000000;
		mmio_write_32(DDRC_FREQ1_ZQCTL0(0), tmp);
	} else if (tgt_freq == 2) {
		tmp = mmio_read_32(DDRC_FREQ2_ZQCTL0(0));
		tmp &= ~0x80000000;
		mmio_write_32(DDRC_FREQ2_ZQCTL0(0), tmp);
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

	/* 42. enable SBRCTL.scrub_en, skip if never enable it */
}
