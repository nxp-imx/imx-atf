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
		    u_register_t x3, void *handle)
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

int imx_noc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	if (IMX_SIP_NOC_LCDIF == x1) {
		/* config NOC for VPU */
		mmio_write_32(IMX_NOC_BASE + 0x108, 0x34);
		mmio_write_32(IMX_NOC_BASE + 0x10c, 0x1);
		mmio_write_32(IMX_NOC_BASE + 0x110, 0x500);
		mmio_write_32(IMX_NOC_BASE + 0x114, 0x30);
		/* config NOC for CPU */
		mmio_write_32(IMX_NOC_BASE + 0x188, 0x34);
		mmio_write_32(IMX_NOC_BASE + 0x18c, 0x1);
		mmio_write_32(IMX_NOC_BASE + 0x190, 0x500);
		mmio_write_32(IMX_NOC_BASE + 0x194, 0x30);
	} else if (IMX_SIP_NOC_PRIORITY == x1) {
		switch(x2) {
		case NOC_GPU_PRIORITY:
			mmio_write_32(IMX_NOC_BASE + 0x008, x3);
			break;
		case NOC_DCSS_PRIORITY:
			mmio_write_32(IMX_NOC_BASE + 0x088, x3);
			break;
		case NOC_VPU_PRIORITY:
			mmio_write_32(IMX_NOC_BASE + 0x108, x3);
			break;
		case NOC_CPU_PRIORITY:
			mmio_write_32(IMX_NOC_BASE + 0x188, x3);
			break;
		case NOC_MIX_PRIORITY:
			mmio_write_32(IMX_NOC_BASE + 0x288, x3);
			break;
		default:
			return SMC_UNK;
		};
	} else {
		return SMC_UNK;
	}

	return 0;
}
