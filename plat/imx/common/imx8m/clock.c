/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <mmio.h>
#include <platform_def.h>

#define IMX_CCM_IP_BASE				(IMX_CCM_BASE + 0xa000)
#define CCM_IP_CLK_ROOT_GEN_TAGET(i)		(IMX_CCM_IP_BASE + 0x80 * (i) + 0x00)
#define CCM_IP_CLK_ROOT_GEN_TAGET_SET(i)	(IMX_CCM_IP_BASE + 0x80 * (i) + 0x04)
#define CCM_IP_CLK_ROOT_GEN_TAGET_CLR(i)	(IMX_CCM_IP_BASE + 0x80 * (i) + 0x08)

void ddr_pll_bypass_100mts(void)
{
	/* change the clock source of dram_alt_clk_root to source 2 --100MHz */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(0), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(0), (0x2 << 24));

	/* change the clock source of dram_apb_clk_root to source 2 --40MHz/2 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x2 << 24) | (0x1 << 16));

	/* configure pll bypass mode */
	mmio_write_32(0x30389804, 1 << 24);
}

void ddr_pll_bypass_400mts(void)
{
	/* change the clock source of dram_alt_clk_root to source 1 --400MHz */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(0), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(0), (0x1 << 24) | (0x1 << 16));

	/* change the clock source of dram_apb_clk_root to source 3 --160MHz/2 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x3 << 24) | (0x1 << 16));

	/* configure pll bypass mode */
	mmio_write_32(0x30389804, 1 << 24);
}

void ddr_pll_bypass_dis(void)
{
	mmio_write_32(0x30389808, 1 << 24);
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (0x7 << 24) | (0x7 << 16));
	/* to source 4 --800MHz/5 */
	mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (0x4 << 24) | (0x4 << 16));
}

/* change the dram clock frequency */
void dram_clock_switch(unsigned int target_freq)
{
	if(target_freq == 2) {  //25M
		ddr_pll_bypass_100mts();
	} else if (target_freq == 0x1) {
		ddr_pll_bypass_400mts();
	} else {
		/* to reduce the dvfs latency. skip re-init pll */
		ddr_pll_bypass_dis();
	}
}

#if defined(PLAT_IMX8M)
void dram_pll_init(unsigned int drate)
{
	/* bypass the PLL */
	mmio_setbits_32(HW_DRAM_PLL_CFG0, 0x30);

	switch (drate) {
	case 3200:
		mmio_write_32(HW_DRAM_PLL_CFG2, 0x00ece580);
		break;
	case 1600:
		mmio_write_32(HW_DRAM_PLL_CFG2, 0x00ec6984);
		break;
	case 667:
		mmio_write_32(HW_DRAM_PLL_CFG2, 0x00f5a406);
		break;
	default:
		break;
	}

	/* bypass the PLL */
	mmio_clrbits_32(HW_DRAM_PLL_CFG0, 0x30);
	while(!(mmio_read_32(HW_DRAM_PLL_CFG0) &(1 << 31)))
		;
}
#endif
