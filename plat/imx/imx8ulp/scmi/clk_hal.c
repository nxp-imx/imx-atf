/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>

#define CGC1_BASE_ADDR	0x292c0000
#define CA35CLK_OFF	0x14
#define PLL2CSR_OFF	0x500
#define PLL2CFG_OFF	0x510

#define CA35CLK_ADDR	(CGC1_BASE_ADDR + CA35CLK_OFF)
#define PLL2CSR_ADDR	(CGC1_BASE_ADDR + PLL2CSR_OFF)
#define PLL2CFG_ADDR	(CGC1_BASE_ADDR + PLL2CFG_OFF)

static void cgc1_set_pll2(uint32_t performance_level)
{
	uint32_t reg;

	if (mmio_read_32(PLL2CSR_ADDR) & BIT_32(23))
		mmio_clrbits_32(PLL2CSR_ADDR, BIT_32(23));

	/* Disable PLL2 */
	mmio_clrbits_32(PLL2CSR_ADDR, BIT_32(0));
	udelay(2);

	/* wait valid bit false */
	while ((mmio_read_32(PLL2CSR_ADDR) & BIT_32(24)))
		;

	/* Select SOSC as source */
	reg = (performance_level / 24000) << 16;
	mmio_write_32(PLL2CFG_ADDR, reg);

	/* Enable PLL2 */
	mmio_setbits_32(PLL2CSR_ADDR, BIT_32(0));

	/* Wait for PLL2 clock ready */
	while (!(mmio_read_32(PLL2CSR_ADDR) & BIT_32(24)))
		;
}

static void cgc1_set_a35_clk(uint32_t clk_src, uint32_t div_core)
{
	uint32_t reg;

	/* ulock */
	if (mmio_read_32(CA35CLK_ADDR) & BIT_32(31))
		mmio_clrbits_32(CA35CLK_ADDR, BIT_32(31));

	reg = mmio_read_32(CA35CLK_ADDR);
	reg &= ~GENMASK(29, 21);
	reg |= ((clk_src & 0x3) << 28);
	reg |= (((div_core - 1) & 0x3f) << 21);
	mmio_write_32(CA35CLK_ADDR, reg);

	/* TODO: timeout delay */
	while (!(mmio_read_32(CA35CLK_ADDR) & BIT_32(27)))
        ;
}

int scmi_set_a35_clk(uint32_t performance_level)
{
	uint32_t reg = mmio_read_32(CA35CLK_ADDR);

	/* if already selected to PLL2, switch to FRO firstly */
	if (((reg >> 28) & 0x3) == 0x1)
		cgc1_set_a35_clk(0, 1);

	/* Set pll2 to 1Ghz */
	cgc1_set_pll2(performance_level);

	/* Set A35 clock to 1GHz */
	cgc1_set_a35_clk(1, 1);

	return 0;
}

uint32_t scmi_get_a35_clk(void)
{
	uint32_t reg = mmio_read_32(CA35CLK_ADDR);

	if (((reg >> 28) & 0x3) == 0x1)
		return (mmio_read_32(PLL2CFG_ADDR) >> 16) * 24000;

	return /* FIRO? */ 48000;
}
