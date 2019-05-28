/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <runtime_svc.h>
#include <std_svc.h>
#include <mmio.h>
#include <platform_def.h>
#include <imx_sip.h>
#include <soc.h>

#define DIGPROG		0x800

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	uint32_t val;

	switch(x1) {
	case FSL_SIP_SRC_M4_START:
		val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x58);
		val &= ~0x1;
		mmio_write_32(IMX_IOMUX_GPR_BASE + 0x58, val);
		break;
	case FSL_SIP_SRC_M4_STARTED:
		val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x58);
		return !(val & 0x1);
	default:
		return SMC_UNK;

	};

	return 0;
}

int imx_soc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	return mmio_read_32(IMX_ANAMIX_BASE + DIGPROG);
}

int imx_noc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	return 0;
}
