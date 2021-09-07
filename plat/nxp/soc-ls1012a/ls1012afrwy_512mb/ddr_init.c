/*
 * Copyright 2018, 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <fsl_mmdc.h>
#include <plat_common.h>
#include <platform_def.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

long long init_ddr(void)
{
	static const struct fsl_mmdc_info mparam = {
		.mdctl = 0x04180000,
		.mdpdc = 0x00030035,
		.mdotc = 0x12554000,
		.mdcfg0 = 0xbabf7954,
		.mdcfg1 = 0xdb328f64,
		.mdcfg2 = 0x01ff00db,
		.mdmisc = 0x00001680,
		.mdref = 0x0f3c8000,
		.mdrwd = 0x00002000,
		.mdor = 0x00bf1023,
		.mdasp = 0x0000003f,
		.mpodtctrl = 0x0000022a,
		.mpzqhwctrl = 0xa1390003,
	};

	mmdc_init(&mparam, NXP_DDR_ADDR);
	NOTICE("DDR Init Done\n");

	return NXP_DRAM0_SIZE;
}
