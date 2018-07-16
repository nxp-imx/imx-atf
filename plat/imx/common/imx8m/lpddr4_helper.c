/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>

void lpddr4_mr_write(uint32_t mr_rank, uint32_t mr_addr, uint32_t mr_data)
{
	uint32_t tmp;
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0. This checks that there
	 * is no outstanding MR transaction. No
	 * writes should be performed to MRCTRL0 and MRCTRL1 if MRSTAT.mr_wr_busy = 1.
	 */
	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while(tmp & 0x1);

	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr,
	 * MRCTRL0.mr_rank and (for MRWs)
	 * MRCTRL1.mr_data to define the MR transaction.
	 */
	mmio_write_32(DDRC_MRCTRL0(0), (mr_rank << 4));
	mmio_write_32(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	mmio_setbits_32(DDRC_MRCTRL0(0), (1 << 31));
}

uint32_t lpddr4_mr_read(uint32_t mr_rank, uint32_t mr_addr)
{
	uint32_t tmp, mr_data;

	mmio_write_32(DRC_PERF_MON_MRR0_DAT(0), 0x1);
	do {
		tmp = mmio_read_32(DDRC_MRSTAT(0));
	} while(tmp & 0x1);

	mmio_write_32(DDRC_MRCTRL0(0), (mr_rank << 4) | 0x1);
	mmio_write_32(DDRC_MRCTRL1(0), (mr_addr << 8));
	mmio_setbits_32(DDRC_MRCTRL0(0), (1 << 31));

	do {
		tmp = mmio_read_32(DRC_PERF_MON_MRR0_DAT(0));
	} while((tmp & 0x8) == 0);

	tmp = mmio_read_32(DRC_PERF_MON_MRR1_DAT(0));
	mr_data = tmp & 0xff;
	mmio_write_32(DRC_PERF_MON_MRR0_DAT(0), 0x4);

	return mr_data;
}
