/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <fsl_mmdc.h>

#include <platform_def.h>

long long init_ddr(void)
{
	static const struct fsl_mmdc_info mparam = {
		.mdctl = U(0x04180000),
		.mdpdc = U(0x00030035),
		.mdotc = U(0x12554000),
		.mdcfg0 = U(0xbabf7954),
		.mdcfg1 = U(0xdb328f64),
		.mdcfg2 = U(0x01ff00db),
		.mdmisc = U(0x00001680),
		.mdref = U(0x0f3c8000),
		.mdrwd = U(0x00002000),
		.mdor = U(0x00bf1023),
		.mdasp = U(0x0000003f),
		.mpodtctrl = U(0x0000022a),
		.mpzqhwctrl = U(0xa1390003),
	};

	mmdc_init(&mparam, NXP_DDR_ADDR);
	NOTICE("DDR Init Done\n");

	return NXP_DRAM0_SIZE;
}
