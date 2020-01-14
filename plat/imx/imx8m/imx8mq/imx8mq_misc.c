/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <imx_sip_svc.h>

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	uint32_t val;

	switch(x1) {
	case IMX_SIP_SRC_M4_START:
		val = mmio_read_32(IMX_SRC_BASE + SRC_M4RCR);
		val &= ~SRC_SCR_M4C_NON_SCLR_RST_MASK;
		val |= SRC_SCR_M4_ENABLE_MASK;
		mmio_write_32(IMX_SRC_BASE + SRC_M4RCR, val);
		break;
	case IMX_SIP_SRC_M4_STARTED:
		val = mmio_read_32(IMX_SRC_BASE + SRC_M4RCR);
		return !(val & SRC_SCR_M4C_NON_SCLR_RST_MASK);
	case IMX_SIP_SRC_SET_SECONDARY_BOOT:
		if (x2 != 0U) {
			mmio_setbits_32(IMX_SRC_BASE + SRC_GPR10_OFFSET,
					SRC_GPR10_PERSIST_SECONDARY_BOOT);
		} else {
			mmio_clrbits_32(IMX_SRC_BASE + SRC_GPR10_OFFSET,
					SRC_GPR10_PERSIST_SECONDARY_BOOT);
		}
		break;
	case IMX_SIP_SRC_IS_SECONDARY_BOOT:
		val = mmio_read_32(IMX_SRC_BASE + SRC_GPR10_OFFSET);
		return !!(val & SRC_GPR10_PERSIST_SECONDARY_BOOT);

	default:
		return SMC_UNK;

	};

	return 0;
}
