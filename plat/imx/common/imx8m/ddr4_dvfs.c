/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>

unsigned fsp_init_reg[3][4] = {
	{ DDRC_INIT3(0), DDRC_INIT4(0), DDRC_INIT6(0), DDRC_INIT7(0) },
	{ DDRC_FREQ1_INIT3(0), DDRC_FREQ1_INIT4(0), DDRC_FREQ1_INIT6(0), DDRC_FREQ1_INIT7(0) },
	{ DDRC_FREQ2_INIT3(0), DDRC_FREQ2_INIT4(0), DDRC_FREQ2_INIT6(0), DDRC_FREQ2_INIT7(0) },
};

static unsigned int mr_value[7];

void ddr4_mr_write(unsigned int mr, unsigned int data, unsigned int mr_type, unsigned int rank)
{
	unsigned int tmp, mr_mirror, data_mirror;
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0 to make sure that there is
	 * no outstanding MR transAction. No
	 */
	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank
	 * and (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	tmp = mmio_read_32(DDRC_DIMMCTL(0));
	if ((tmp & 0x2) && (rank == 0x2)) {
		mr_mirror = (mr & 0x4) | ((mr & 0x1) << 1) | ((mr & 0x2) >> 1); /* BA0, BA1 swap */
		data_mirror = (data & 0x1607) | ((data & 0x8) << 1) | ((data & 0x10) >> 1) |
				((data & 0x20) << 1) | ((data & 0x40) >> 1) | ((data & 0x80) << 1) |
				 ((data & 0x100) >> 1) | ((data & 0x800) << 2) | ((data & 0x2000) >> 2) ;
	} else {
		mr_mirror = mr;
		data_mirror = data;
	}

	mmio_write_32(DDRC_MRCTRL0(0), mr_type | (mr_mirror << 12) | (rank << 4) );
	mmio_write_32(DDRC_MRCTRL1(0), data_mirror );

	/*
	 * 3. In a separate APB transaction, write the MRCTRL0.mr_wr to 1. This bit is
	 * self-clearing, and triggers the MR transaction. The uMCTL2 then asserts the
	 * MRSTAT.mr_wr_busy while it performs the MR transaction to SDRAM, and no further
	 * accesses can be initiated until it is deasserted.
	 */
	mmio_setbits_32(DDRC_MRCTRL0(0), (1 << 31));
	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while (tmp);
}

void dram_cfg_all_mr(unsigned int num_rank, unsigned int pstate)
{
	/*
	 * 15. Perform MRS commands as required to re-program timing registers
	 * in the SDRAM for the new frequency (in particular, CL, CWL and WR
	 * may need to be changed).
	 */

	for (int i = 1; i <= num_rank; i++) {
		for (int j = 0; j < 7; j++)
			ddr4_mr_write(j, mr_value[j], 0, i);
	}
}

void sw_pstate(unsigned int pstate)
{
	volatile unsigned int tmp;
	unsigned int i;

	mmio_write_32(DDRC_SWCTL(0), 0x0000);
	/* 12. Change the clock frequency to the desired value. */

	/*
	 * 13. Update any registers which may be required to change for the new frequency. This includes
	 * quasidynamic and dynamic registers. This includes both uMCTL2 registers and PHY registers.
	 */
	mmio_write_32(DDRC_MSTR2(0), pstate); /* UMCTL2_REGS_FREQ1 */
	mmio_setbits_32(DDRC_MSTR(0), (0x1 << 29));

	/*
	 * dvfs.18. Toggle RFSHCTL3.refresh_update_level to allow the new refresh-related
	 * register values to propagate to the refresh logic.
	 */
	tmp = mmio_read_32(DDRC_RFSHCTL3(0));
	if ((tmp & 0x2) == 0x2)
		mmio_write_32(DDRC_RFSHCTL3(0), tmp & 0xFFFFFFFD);
	else
		mmio_write_32(DDRC_RFSHCTL3(0), tmp | 0x2);
	/*
	 *dvfs.19. If required, trigger the initialization in the PHY. If using the gen2
	 * multiPHY, PLL initialization should be triggered at this point. See the PHY
	 * databook for details about the frequency change procedure.
	 */
	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000 | (pstate << 8));//pstate1
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020 | (pstate << 8));

	/* wait DFISTAT.dfi_init_complete to 0 */
	do {
		tmp = 0x1 & mmio_read_32(DDRC_DFISTAT(0));
	} while (tmp);

	/* change the clock to the target frequency */
	dram_clock_switch(pstate);

	mmio_write_32(DDRC_DFIMISC(0), 0x00000000 | (pstate << 8));
	/* wait DFISTAT.dfi_init_complete to 1 */
	do {
		tmp = 0x1 & mmio_read_32(DDRC_DFISTAT(0));
	} while (!tmp);

	/*
	 * When changing frequencies the controller may violate the JEDEC
	 * requirement that no more than 16 refreshes should be issued within
	 * 2*tREFI. These extra refreshes are not expected to cause a problem
	 * in the SDRAM. This issue can be avoided by waiting for at least 2*tREFI
	 * before exiting self-refresh in step 19.
	 */
	for (i = 20; i > 0; i--)
		printf("waiting for 2*tREF (2*7.8us)\n");

	/* 14. Exit the self-refresh state by setting PWRCTL.selfref_sw = 0. */
	mmio_clrbits_32(DDRC_PWRCTL(0), (1 << 5));
	do {
		tmp  = 0x3f & (mmio_read_32((DDRC_STAT(0))));
	} while (tmp == 0x23);
}

void ddr4_dll_change(unsigned int num_rank, unsigned int pstate, unsigned int cur_pstate)
{
	volatile unsigned int tmp;
	enum DLL_STATE { NO_CHANGE=0, ON2OFF=1, OFF2ON=2} dll_sw; /* 0-no change, 1-on2off, 2-off2on.; */

	if (pstate != 0 && cur_pstate == 0)
		dll_sw = ON2OFF;
	else if (pstate == 0 && cur_pstate != 0)
		dll_sw = OFF2ON;
	else
		dll_sw = NO_CHANGE;

	/* the the following software programming sequence to switch from DLL-on to DLL-off, or reverse: */
	mmio_write_32(DDRC_SWCTL(0), 0x0000);

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000000);

	/* 1. Set the DBG1.dis_hif = 1. This prevents further reads/writes being received on the HIF. */
	mmio_setbits_32(DDRC_DBG1(0), (0x1 << 1));

	/* 2. Set ZQCTL0.dis_auto_zq=1, to disable automatic generation of ZQCS/MPC(ZQ calibration) */
	mmio_setbits_32(DDRC_FREQ1_ZQCTL0(0), (1 << 31));
	mmio_setbits_32(DDRC_FREQ2_ZQCTL0(0), (1 << 31));
	mmio_setbits_32(DDRC_ZQCTL0(0), (1 << 31));

	/* 3. Set RFSHCTL3.dis_auto_refresh=1, to disable automatic refreshes */
	mmio_setbits_32(DDRC_RFSHCTL3(0), 0x1);
	/* 4. Ensure all commands have been flushed from the uMCTL2 by polling */
	do {
		tmp = 0x36000000 & mmio_read_32(DDRC_DBGCAM(0));
	} while (tmp != 0x36000000);

	/* mmio_write_32(DDRC_PCTRL_0(0), 0x00000000); */

	/* 5. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable RTT_NOM: */
	/* a. DDR3: Write 0 to MR1[9], MR1[6] and MR1[2] */
	/* b. DDR4: Write 0 to MR1[10:8] */
	for (int i = 1; i <= num_rank; i++) {
		if (mr_value[1] & 0x700)
			ddr4_mr_write(1, mr_value[1] & 0xF8FF, 0, i);
	}

	/* 6. For DDR4 only: Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to write 0 to */
	/* MR5[8:6] to disable RTT_PARK */
	for (int i = 1; i < num_rank; i++) {
		if (mr_value[5] & 0x1C0)
			ddr4_mr_write(5, mr_value[5] & 0xFE3F, 0, i);
	}

	if(dll_sw == ON2OFF) {
	        /* 7. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to write 0 to MR2[11:9], to */
		/* disable RTT_WR (and therefore disable dynamic ODT). This applies for both DDR3 and DDR4. */
		for (int i = 1; i <= num_rank; i++) {
			if (mr_value[2] & 0xE00)
				ddr4_mr_write(2, mr_value[2] & 0xF1FF, 0, i);
		}

		/* 8. Perform an MRS command (using MRCTRL0 and MRCTRL1 registers) to disable the DLL. The */
		/* timing of this MRS is automatically handled by the uMCTL2. */
		/* a. DDR3: Write 1 to MR1[0] */
		/* b. DDR4: Write 0 to MR1[0] */
		for (int i = 1; i <= num_rank; i++) {
			ddr4_mr_write(1, mr_value[1] & 0xFFFE, 0, i);
		}
	}

	/* 9. Put the SDRAM into self-refresh mode by setting PWRCTL.selfref_sw = 1, and polling */
	/* STAT.operating_mode to ensure the DDRC has entered self-refresh. */
	mmio_setbits_32(DDRC_PWRCTL(0), (1 << 5));
	/*
	 * 10. Wait until STAT.operating_mode[1:0]==11 indicating that the DWC_ddr_umctl2
	 * core is in selfrefresh mode. Ensure transition to self-refresh was due to software
	 * by checking that
	 */
	/* STAT.selfref_type[1:0]=2`b10. */
	do {
		tmp  = 0x3f & (mmio_read_32((DDRC_STAT(0))));
	} while (tmp != 0x23);

	/* 11. Set the MSTR.dll_off_mode = 1 or 0. */
	if (dll_sw == ON2OFF)
		mmio_setbits_32(DDRC_MSTR(0), (1 << 15));

	if(dll_sw == OFF2ON)
		mmio_clrbits_32(DDRC_MSTR(0), (1 << 15));

	sw_pstate(pstate);

	/* DRAM dll enable */
	if (dll_sw == OFF2ON) {
		for (int i = 1; i <= num_rank; i++) {
			ddr4_mr_write(1, mr_value[1] | 0x1, 0, i);
		}

		for (int i = 1; i <= num_rank; i++) {
			ddr4_mr_write(0, mr_value[0] | 0x100, 0, i);
		}
	}

	dram_cfg_all_mr(num_rank, pstate);

	/* 16. Re-enable automatic generation of ZQCS/MPC(ZQ calibration) commands */
	mmio_clrbits_32(DDRC_FREQ1_ZQCTL0(0), (1 << 31));
	mmio_clrbits_32(DDRC_FREQ2_ZQCTL0(0), (1 << 31));
	mmio_clrbits_32(DDRC_ZQCTL0(0), (1 << 31));

	/* 17. Re-enable automatic refreshes (RFSHCTL3.dis_auto_refresh = 0) if they have been previously disabled. */
	mmio_clrbits_32(DDRC_RFSHCTL3(0), 0x1);
	/* 18. Restore ZQCTL0.dis_srx_zqcl */
	/* 19. Write DBG1.dis_hif = 0 to re-enable reads and writes. */
	mmio_clrbits_32(DDRC_DBG1(0), (0x1 << 1));

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001);
	/* 27. Write 1 to SBRCTL.scrub_en. Enable SBR if desired, only required if SBR instantiated. */

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x0001);

	/* wait SWSTAT.sw_done_ack to 1 */
	do {
		tmp  = 0x1 & mmio_read_32(DDRC_SWSTAT(0));
	} while (!tmp);
}

void ddr4_dll_no_change(unsigned int num_rank, unsigned int pstate)
{
	volatile unsigned int tmp;
	/* 1. set SWCTL.sw_done to disable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x0000);

	/* 2. Write 0 to PCTRL_n.port_en. This blocks AXI port(s) from taking any transaction (blocks traffic on
	AXI ports). */
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000000);

	/* 3. Poll PSTAT.rd_port_busy_n=0 and PSTAT.wr_port_busy_n=0. Wait until all AXI ports are idle (the
	 uMCTL2 core has to be idle). */
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
	} while (tmp & 0x10001);

	/* 4. Write 0 to SBRCTL.scrub_en. Disable SBR, required only if SBR instantiated. */
	/* 5. Poll SBRSTAT.scrub_busy=0. Indicates that there are no outstanding SBR read commands */
	/* 6. Set DERATEEN.derate_enable = 0, if DERATEEN.derate_eanble = 1 and the read latency (RL) value */
	//needs to change after the frequency change (LPDDR2/3/4 only).
	/* 7. Set DBG1.dis_hif=1 so that no new commands will be accepted by the uMCTL2. */
	mmio_setbits_32(DDRC_DBG1(0), (0x1 << 1));
	/* 8. Poll DBGCAM.dbg_wr_q_empty and DBGCAM.dbg_rd_q_empty to ensure that write and read data buffers are empty. */
	do {
		tmp = 0x06000000 & mmio_read_32(DDRC_DBGCAM(0));
	} while (tmp != 0x06000000);
	/* 9. For DDR4, update MR6 with the new tDLLK value via the Mode Register Write signals */

	/* 10. Set DFILPCFG0.dfi_lp_en_sr = 0, if DFILPCFG0.dfi_lp_en_sr = 1, and wait until DFISTAT.dfi_lp_ack */
	/* 11. If DFI PHY Master interface is active in uMCTL2 (DFIPHYMSTR.phymstr_en == 1'b1) then disable it */
	/* 12. Wait until STAT.operating_mode[1:0]!=11 indicating that the DWC_ddr_umctl2 controller is not in self-refresh mode. */
	tmp  = 0x3 & (mmio_read_32((DDRC_STAT(0))));
	if (tmp == 0x3) {
		printf("C: Error DRAM should not in Self Refresh\n");
	}
	/* 13. Assert PWRCTL.selfref_sw for the DWC_ddr_umctl2 core to enter the self-refresh mode. */
	mmio_setbits_32(DDRC_PWRCTL(0), (1 << 5));
	/* 14. Wait until STAT.operating_mode[1:0]==11 indicating that the DWC_ddr_umctl2 core is in selfrefresh mode. */
	do {
		tmp  = 0x3f & (mmio_read_32((DDRC_STAT(0))));
	} while (tmp != 0x23);

	sw_pstate(pstate);
	dram_cfg_all_mr(num_rank, pstate);

	/* 23. Enable HIF commands by setting DBG1.dis_hif=0. */
	mmio_clrbits_32(DDRC_DBG1(0), (0x1 << 1));
	/* 24. Reset DERATEEN.derate_enable = 1 if DERATEEN.derate_enable has been set to 0 in step 6. */
	/* 25. If DFI PHY Master interface was active in uMCTL2 (DFIPHYMSTR.phymstr_en == 1'b1) before the */
	/* step 11 then enable it back by programming DFIPHYMSTR.phymstr_en = 1'b1. */
	/* 26. Write 1 to PCTRL_n.port_en. AXI port(s) are no longer blocked from taking transactions (Re-enable traffic on AXI ports). */
	/* Re-enable traffic on the AXI ports */
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001);
	/* 27. Write 1 to SBRCTL.scrub_en. Enable SBR if desired, only required if SBR instantiated. */

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x0001);

	/* wait SWSTAT.sw_done_ack to 1 */
	do {
		tmp  = 0x1 & mmio_read_32(DDRC_SWSTAT(0));
	} while (!tmp);
}

void get_mr_values(unsigned int pstate)
{
	uint32_t init_val;

	for (int i = 0; i < 3; i++) {
		init_val = mmio_read_32(fsp_init_reg[pstate][i]);
		mr_value[2*i] = init_val >> 16;
		mr_value[2*i + 1] = init_val & 0xFFFF;
	}

	mr_value[6] = mmio_read_32(fsp_init_reg[pstate][3]) & 0xFFFF;
}

void ddr4_swffc(struct dram_info *dram_info, unsigned int pstate)
{
	unsigned int cur_pstate = dram_info->current_fsp;

	get_mr_values(pstate);

	if ((pstate == 0 && cur_pstate !=0) || (pstate != 0 && cur_pstate == 0))
		ddr4_dll_change(dram_info->num_rank, pstate, cur_pstate);
	else
		ddr4_dll_no_change(dram_info->num_rank, pstate);

	dram_info->current_fsp = pstate;
}
