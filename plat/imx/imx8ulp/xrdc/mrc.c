/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#define XRDC_ADDR	0x292f0000
#define MRC_OFFSET	0x2000
#define MRC_STEP	0x200

#define SP(X)		((X) << 9)
#define SU(X)		((X) << 6)
#define NP(X)		((X) << 3)
#define NU(X)		((X) << 0)

#define RWX		7
#define RW		6
#define R		4
#define X		1

#define D7SEL_CODE	(SP(RW) | SU(RW) | NP(RWX) | NU(RWX))
#define D6SEL_CODE	(SP(RW) | SU(RW) | NP(RWX))
#define D5SEL_CODE	(SP(RW) | SU(RWX))
#define D4SEL_CODE	SP(RWX)
#define D3SEL_CODE	(SP(X) | SU(X) | NP(X) | NU(X))
#define D0SEL_CODE	0

#define D7SEL_DAT	(SP(RW) | SU(RW) | NP(RW) | NU(RW))
#define D6SEL_DAT	(SP(RW) | SU(RW) | NP(RW))
#define D5SEL_DAT	(SP(RW) | SU(RW) | NP(R) | NU(R))
#define D4SEL_DAT	(SP(RW) | SU(RW))
#define D3SEL_DAT	SP(RW)

union dxsel_perm {
	struct {
		uint8_t dx;
		uint8_t perm;
	};

	uint32_t dom_perm;
};

static int xrdc_config_mrc_dx_perm(uint32_t mrc_con, uint32_t region, uint32_t dom, uint32_t dxsel)
{
	uint32_t w2_addr, val = 0;

	w2_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0x8;

	val = (mmio_read_32(w2_addr) & (~(7 << (3 * dom)))) | (dxsel << (3 * dom));
	mmio_write_32(w2_addr, val);

	return 0;
}

static int xrdc_config_mrc_w0_w1(uint32_t mrc_con, uint32_t region, uint32_t w0, uint32_t size)
{

	uint32_t w0_addr, w1_addr;

	w0_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20;	
	w1_addr = w0_addr + 4;

	if ((size % 32) != 0)
		return -EINVAL;

	mmio_write_32(w0_addr, w0 & ~0x1f);
	mmio_write_32(w1_addr, w0 + size - 1);

	return 0;
}

static int xrdc_config_mrc_w3_w4(uint32_t mrc_con, uint32_t region, uint32_t w3, uint32_t w4)
{
	uint32_t w3_addr = XRDC_ADDR + MRC_OFFSET + mrc_con * 0x200 + region * 0x20 + 0xC;
	uint32_t w4_addr = w3_addr + 4;

	mmio_write_32(w3_addr, w3);
	mmio_write_32(w4_addr, w4);

	return 0;
}

int xrdc_config_pdac(uint32_t bridge, uint32_t index, uint32_t dom, uint32_t perm)
{
	uint32_t w0_addr;
	uint32_t val;

	switch (bridge) {
	case 3:
		w0_addr = XRDC_ADDR + 0x1000 + 0x8 * index;
		break;
	case 4:
		w0_addr = XRDC_ADDR + 0x1400 + 0x8 * index;
		break;
	case 5:
		w0_addr = XRDC_ADDR + 0x1800 + 0x8 * index;
		break;
	default:
		return -EINVAL;
	}
	val = mmio_read_32(w0_addr);
	mmio_write_32(w0_addr, (val & ~(0x7 << (dom * 3))) | (perm << (dom * 3)));

	val = mmio_read_32(w0_addr + 4);
	mmio_write_32(w0_addr + 4, val | BIT_32(31));

	return 0;
}

int xrdc_config_mda(uint32_t mda_con, uint32_t dom)
{
	uint32_t w0_addr;
	uint32_t val;

	w0_addr = XRDC_ADDR + 0x800 + mda_con * 0x20;

	val = mmio_read_32(w0_addr);
	mmio_write_32(w0_addr, (val & (~0xF)) | dom | BIT_32(31));

	return 0;
}

int xrdc_config_mrc11_hifi_itcm(void)
{
	xrdc_config_mrc_w0_w1(11, 0, 0x21170000, 64 * 1024);
	/* Dom7 RW, accset2 */
	xrdc_config_mrc_dx_perm(11, 0, 7, 2);
	/* Dom2 RX, accset1 */
	xrdc_config_mrc_dx_perm(11, 0, 2, 1);
	xrdc_config_mrc_w3_w4(11, 0, BIT_32(31), BIT_32(31) | ((SP(RW) | SU(RW) | NP(RW)) << 16) |0xFFF);

	return 0;
}

int xrdc_config_mrc11_hifi_dtcm(void)
{
	xrdc_config_mrc_w0_w1(11, 1, 0x21180000, 64 * 1024);
	/* Dom7 RW, accset2 */
	xrdc_config_mrc_dx_perm(11, 1, 7, 2);
	/* Dom2 RX, accset1 */
	xrdc_config_mrc_dx_perm(11, 1, 2, 1);
	xrdc_config_mrc_w3_w4(11, 1, 0, BIT_32(31) | ((SP(RW) | SU(RW) | NP(RW)) << 16) | (SP(RW) | SU(RW) | NP(RW) | NU(RW)));

	return 0;
}

int xrdc_config_mrc6_dma2_ddr(void)
{
	uint32_t accset1 = SP(RW) | SU(RW) | NP(RW) | NU(RW);
	uint32_t accset2 = 0;

	xrdc_config_mrc_w0_w1(6, 0, 0xe0000000, 256 * 1024 * 1024);
	/* Dom0 RW, accset1 */
	xrdc_config_mrc_dx_perm(6, 0, 0, 1);
	xrdc_config_mrc_w3_w4(6, 0, 0, BIT_32(31) | (accset2 << 16) | accset1);

	return 0;
}

int xrdc_config_mrc7_hifi_ddr(void)
{
	xrdc_config_mrc_w0_w1(7, 0, 0x90000000, 256 * 1024 * 1024);
	/* Dom2 RX, accset1 */
	xrdc_config_mrc_dx_perm(7, 0, 2, 1);
	xrdc_config_mrc_w3_w4(7, 0, BIT_32(31), BIT_32(31) | ((SP(RW) | SU(RW) | NP(RW)) << 16) | 0xFFF);

	xrdc_config_mrc_w0_w1(7, 1, 0x80000000, 256 * 1024 * 1024);
	/* Dom2 RX, accset1 */
	xrdc_config_mrc_dx_perm(7, 1, 2, 1);
	xrdc_config_mrc_w3_w4(7, 1, BIT_32(31), BIT_32(31) | ((SP(RW) | SU(RW) | NP(RW)) << 16) | 0xFFF);

	return 0;
}
