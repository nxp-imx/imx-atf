/*
 * Copyright 2017 NXP
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

#define M4RCR (0xC)
#define SRC_SCR_M4_ENABLE_OFFSET		3
#define SRC_SCR_M4_ENABLE_MASK			(1 << 3)
#define SRC_SCR_M4C_NON_SCLR_RST_OFFSET		0
#define SRC_SCR_M4C_NON_SCLR_RST_MASK		(1 << 0)

#define DIGPROG		0x6c
#define SW_INFO_A0	0x800
#define SW_INFO_B0	0x83C
#define SW_INFO_B1	0x40

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

int imx_soc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	uint32_t val;
	uint32_t rom_version;

	val = mmio_read_32(IMX_ANAMIX_BASE + DIGPROG);
	rom_version = mmio_read_8(IMX_ROM_BASE + SW_INFO_A0);
	if (rom_version != 0x10) {
		rom_version = mmio_read_8(IMX_ROM_BASE + SW_INFO_B0);
		if (rom_version == 0x20) {
			val &= ~0xff;
			val |= rom_version;
		} else if (mmio_read_32(IMX_OCOTP_BASE + SW_INFO_B1)
			   == 0xff0055aa) {
			/* 0xff0055aa is magic number for B1 */
			val &= ~0xff;
			val |= 0x21;
		}
	}

	return val;
}

int imx_noc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3)
{
	if (FSL_SIP_NOC_LCDIF == x1) {
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
	} else if (FSL_SIP_NOC_PRIORITY == x1) {
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
