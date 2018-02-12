/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

void lpddr4_dvfs_hwffc(int init_vrcg, int init_fsp, int target_freq,
		int discamdrain)
{
	uint32_t tmp, tmp_t;

	INFO("hwffc enter\n");

	/* step 1: hwffc flow enter, set the HWFCCCTL */
	tmp = ((init_fsp << 4) & 0x10) | ((init_vrcg << 5) & 0x20) | 0x40;
	tmp |= 0x3;
	mmio_write_32(IMX_DDRC_BASE + DDRC_HWFFCCTL, tmp);

	/*
	 * set SWCTL.sw_done to disable quasi-dynamic register programming
	 * outside reset
	 */
	mmio_write_32(IMX_DDRC_BASE + DDRC_SWCTL, 0x0);

	mmio_write_32(IMX_DDRC_BASE + DDRC_DFIMISC, 0x11);

	/*
	 * set SWCTL.sw_done to enable quasi-dynamic register programming
	 * outside reset.
	 */
	mmio_write_32(IMX_DDRC_BASE + DDRC_SWCTL, 0x1);
	/*wait SWSTAT.sw_done_ack to 1 */
	while (!(mmio_read_32(IMX_DDRC_BASE + DDRC_SWSTAT) & 0x1))
		;

	/* wait for DBGCAM is empty */
	if (discamdrain)
		while ((mmio_read_32(IMX_DDRC_BASE + DDRC_DBGCAM) &
			 0x30000000) != 0x30000000)
			;

	/* set HWFFC requirements, ddrc_csysfreqency_ddrc[1:0] = 2b'10;
	 * [15]: ccmsrcgpcmix_ddrc_csysdisdrain_ddrc;
	 * [14:13]: ccmsrcgpcmix_ddrc_csysfrequency_ddrc[1:0];
	 * [12]: ccmsrcgpcmix_ddrc_csysmode_ddrc;
	 */
	tmp = mmio_read_32(0x303a0164);
	tmp &= 0xFFFF0FFF;
	tmp |= ((discamdrain << 15) & 0x8000);
	tmp |= ((target_freq << 13) & 0x6000);
	tmp |= 0x1000;
	mmio_write_32(0x303a0164, tmp);

	/* set the gpc_ddr1_core_csysreq to active low */
	tmp = mmio_read_32(0x303a01fc);
	tmp &= 0xfffffffe;
	mmio_write_32(0x303a01fc, tmp);

	/* wait gpc_ddr1_core_csysack to active low */
	while ((mmio_read_32(0x303a01fc) & 0x10000))
		;

	/* step B: switch the clock */
	if (target_freq == 0x1) {
		/* change the clock source of dram_alt_clk_root to source 5 --400MHz */
		mmio_write_32(0x3038a008, (0x7 << 24) | (0x7 << 16));
		mmio_write_32(0x303aa004, (0x5 << 24));

		/* change the clock source of dram_apb_clk_root to source 2 --40MHz/2 */
		mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
		mmio_write_32(0x3038a084, (0x2 << 24) | (0x1 << 16));;

		/* bypass the DRAM PLL */
		mmio_write_32(0x30389804, 1 << 24);

	} else if (target_freq == 0x2) {
		/* change the clock source of dram_alt_clk_root to source 2 --100MHz */
		mmio_write_32(0x3038a008, (0x7 << 24) | (0x7 << 16));
		mmio_write_32(0x3038a004, (0x2 << 24));

		/* change the clock source of dram_apb_clk_root to source 2 --40Mhz/2 */
		mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
		mmio_write_32(0x3038a084, (0x2 << 24) | (0x1 << 16));;

		/* bypass the DRAM PLL */
		mmio_write_32(0x30389804, 1 << 24);
	} else {
		/* switch to the default freq fsp = 0x1 3200mts */
		mmio_write_32(0x3038a088,(0x7<<24)|(0x7<<16));
		/* to source 4 --800MHz/ 4 */
		mmio_write_32(0x3038a084,(0x4<<24)|(0x3<<16));
		mmio_write_32(0x30389808, 1<<24);
	}

	/* step C: exit hwffc flow */
	tmp = mmio_read_32(0x303a01fc);
	tmp |= 0x1;
	mmio_write_32(0x303a01fc, tmp);
	/* wait gpc_ddr1_core_csysack to high */
	while (!(mmio_read_32(0x303a01fc) & 0x10000))
		;

	/* [1:0]-->2b'10, to disable HWFFC */
	mmio_write_32(IMX_DDRC_BASE + DDRC_HWFFCCTL, 0x72);

	/*wait until hwffc_in_progress = 0 */
	while(mmio_read_32(IMX_DDRC_BASE + DDRC_HWFFCSTAT) & 0x1)
		;

	mmio_write_32(IMX_DDRC_BASE + DDRC_SWCTL, 0x0);
	/* set MSTR.frequency_mode MSTR.[29], MSTR2.target_frequency[1:0] */
	tmp = mmio_read_32(IMX_DDRC_BASE + DDRC_MSTR);
	tmp |= 0x20000000;
	mmio_write_32(IMX_DDRC_BASE + DDRC_MSTR, tmp);

	tmp = mmio_read_32(IMX_DDRC_BASE + DDRC_HWFFCSTAT);
	tmp_t = (tmp >> 4) & 0x3;
	tmp = mmio_read_32(IMX_DDRC_BASE + DDRC_MSTR2);
	tmp = (tmp & 0xfffffffc) | tmp_t;
	mmio_write_32(IMX_DDRC_BASE + DDRC_MSTR2, tmp);

	mmio_write_32(IMX_DDRC_BASE + DDRC_SWCTL, 0x1);
	mmio_write_32(IMX_DDRC_BASE + DDRC_HWFFCCTL, 0x70);

	/* why we need below flow */
	mmio_write_32(IMX_DDRC_BASE + DDRC_PWRCTL, 0x1a8);
	while ((mmio_read_32(IMX_DDRC_BASE + DDRC_STAT) & 0x3) != 0x3)
		;
	/* wait SDRAM into self refresh state */
	while(!(mmio_read_32(IMX_DDRC_BASE + DDRC_STAT) & 0x30))
		;

	tmp = mmio_read_32(IMX_DDRC_BASE + DDRC_CRCPARSTAT);
	tmp = mmio_read_32(IMX_DDRC_BASE + DDRC_CRCPARSTAT);

	mmio_write_32(IMX_DDRC_BASE + DDRC_PWRCTL, 0x188);

	/* wait STAT to normal mode */
	while ((mmio_read_32(IMX_DDRC_BASE + DDRC_STAT) & 0x3) != 0x1)
		;

	INFO("hwffc exit\n");
}
