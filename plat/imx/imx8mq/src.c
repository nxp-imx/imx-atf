/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <types.h>
#include <mmio.h>
#include <platform_def.h>
#include <fsl_sip.h>
#include <soc.h>

#define M4RCR (0xC)
#define SRC_SCR_M4_ENABLE_OFFSET		3
#define SRC_SCR_M4_ENABLE_MASK			(1 << 3)
#define SRC_SCR_M4C_NON_SCLR_RST_OFFSET		0
#define SRC_SCR_M4C_NON_SCLR_RST_MASK		(1 << 0)
int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	uint32_t val;

	switch(x1) {
	case FSL_SIP_SRC_M4_START:
		val = mmio_read_32(IMX_SRC_BASE + M4RCR);
		val &= ~SRC_SCR_M4C_NON_SCLR_RST_MASK;
		val |= SRC_SCR_M4_ENABLE_MASK;
		mmio_write_32(IMX_SRC_BASE + M4RCR, val);
		break;
	case FSL_SIP_SRC_M4_STARTED:
		val = mmio_read_32(IMX_SRC_BASE + M4RCR);
		return !(val & SRC_SCR_M4C_NON_SCLR_RST_MASK);
	default:
		return SMC_UNK;

	};

	return 0;
}
