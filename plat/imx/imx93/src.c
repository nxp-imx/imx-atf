/*
 * Copyright 2022 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <drivers/delay_timer.h>
#include <lib/libc/errno.h>
#include <plat_imx8.h>

#include <imx_sip_svc.h>

#define M33_CFG_OFF		0x60
#define M33_INITSVTOR		0x5C
#define M33_INITSVTOR_POLL	0x9C
#define M33_CPU_WAIT_MASK	BIT(2)

#define SRC_IPS_BASE_ADDR		(0x44460000)
#define SRC_GLOBAL_RBASE		(SRC_IPS_BASE_ADDR + 0x0000)
#define SRC_M33P_RBASE			(SRC_IPS_BASE_ADDR + 0x2800)
#define REG_SRC_GLOBAL_CM_QUIESCE	(SRC_GLOBAL_RBASE + 0x104)
#define REG_SRC_GLOBAL_SCR		(SRC_GLOBAL_RBASE + 0x10)
#define REG_SRC_GLOBAL_SRTMR		(SRC_GLOBAL_RBASE + 0x14)
#define REG_SRC_GLOBAL_SRMASK		(SRC_GLOBAL_RBASE + 0x18)
#define REG_SRC_GLOBAL_SRMR1		(SRC_GLOBAL_RBASE + 0x20)
#define REG_SRC_GLOBAL_SRMR2		(SRC_GLOBAL_RBASE + 0x24)
#define REG_SRC_GLOBAL_SRMR3		(SRC_GLOBAL_RBASE + 0x28)
#define REG_SRC_GLOBAL_SRMR4		(SRC_GLOBAL_RBASE + 0x2c)
#define REG_SRC_GLOBAL_SRMR5		(SRC_GLOBAL_RBASE + 0x30)
#define REG_SRC_GLOBAL_SRMR6		(SRC_GLOBAL_RBASE + 0x34)
#define REG_SRC_GLOBAL_SBMR1		(SRC_GLOBAL_RBASE + 0x40)
#define REG_SRC_GLOBAL_SBMR2		(SRC_GLOBAL_RBASE + 0x44)
#define REG_SRC_GLOBAL_SRSR 		(SRC_GLOBAL_RBASE + 0x50)
#define REG_SRC_GLOBAL_GPR9		(SRC_GLOBAL_RBASE + 0x9C)

#define REG_SRC_M33P_SW_CTRL (SRC_M33P_RBASE + 0x20)
#define REG_SRC_M33P_FUNC_STAT (SRC_M33P_RBASE + 0xB4)

#define SRC_MIX_SLICE_FUNC_STAT_PSW_STAT BIT(0)
#define SRC_MIX_SLICE_FUNC_STAT_RST_STAT BIT(2)
#define SRC_MIX_SLICE_FUNC_STAT_ISO_STAT BIT(4)

#define BLK_CTL_S_AONMIX_BASE	0x444F0000

struct wdog_regs {
	uint32_t cs;
	uint32_t cnt;
	uint32_t toval;
	uint32_t win;
};

#define WDG1_BASE_ADDR		0x442D0000UL

#define UNLOCK_WORD		0xD928C520 /* unlock word */
#define REFRESH_WORD		0xB480A602 /* refresh word */
#define WDGCS_WDGE		BIT(7)
#define WDGCS_WDGUPDATE		BIT(5)

#define WDGCS_RCS		BIT(10)
#define WDGCS_ULK               BIT(11)
#define WDGCS_CMD32EN           BIT(13)
#define WDGCS_FLG               BIT(14)

#define WDG_BUS_CLK		(0x0)
#define WDG_LPO_CLK		(0x1)
#define WDG_32KHZ_CLK		(0x2)
#define WDG_EXT_CLK		(0x3)

void disable_wdog(uint32_t wdog_base)
{
	uint32_t val_cs = mmio_read_32(wdog_base + 0x00);

	if (!(val_cs & 0x80))
		return;

	/* default is 32bits cmd */
	mmio_write_32((wdog_base + 0x04), REFRESH_WORD); /* Refresh the CNT */

	if (!(val_cs & 0x800)) {
		mmio_write_32((wdog_base + 0x04), UNLOCK_WORD);
		while (!(mmio_read_32(wdog_base + 0x00) & 0x800))
			;
	}
	mmio_write_32((wdog_base + 0x0C), 0); /* Set WIN to 0 */
	mmio_write_32((wdog_base + 0x08), 0x400); /* Set timeout to default 0x400 */
	mmio_write_32((wdog_base + 0x00), 0x2120); /* Disable it and set update */

	while (!(mmio_read_32(wdog_base + 0x00) & 0x400))
		;
}
static void wdog1_cpu(uint32_t addr)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDG1_BASE_ADDR;

	disable_wdog(WDG1_BASE_ADDR);

	/* CMD32EN=1 by default, so use 32bit unlock sequence. */
	mmio_write_32((uintptr_t)&wdog->cnt, UNLOCK_WORD);

	/* Wait WDOG Unlock */
	while (!(mmio_read_32((uintptr_t)&wdog->cs) & WDGCS_ULK)) {
	};

	mmio_write_32((uintptr_t)&wdog->toval, 1000);
	mmio_write_32((uintptr_t)&wdog->win, 0);

	/* enable counter running */
	mmio_write_32((uintptr_t)&wdog->cs, (0x2900 |  WDGCS_CMD32EN | WDGCS_WDGE |
		WDGCS_WDGUPDATE |(WDG_LPO_CLK << 8) | WDGCS_FLG));

	/* Wait WDOG reconfiguration */
	while (!(mmio_read_32((uintptr_t)&wdog->cs) & WDGCS_RCS)) {
	};
}

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	uint32_t val = 1;

	switch(x1) {
	case IMX_SIP_SRC_M4_START:
		mmio_write_32(BLK_CTL_S_AONMIX_BASE + M33_INITSVTOR, 0);
		if (x2)
			mmio_write_32(REG_SRC_GLOBAL_GPR9, x2);
		else
			mmio_write_32(REG_SRC_GLOBAL_GPR9, 0xFFE0000); /* for SPL kick M33, Uboot load m33 firmware */
		mmio_clrbits_32(BLK_CTL_S_AONMIX_BASE + M33_CFG_OFF, M33_CPU_WAIT_MASK);
		break;
	case IMX_SIP_SRC_M4_STARTED:
		val =  mmio_read_32(BLK_CTL_S_AONMIX_BASE + M33_CFG_OFF) & M33_CPU_WAIT_MASK;
		return val ? 0 : 1;
	case IMX_SIP_SRC_M4_STOP:
		mmio_setbits_32(REG_SRC_GLOBAL_SRMR1, BIT(8) | BIT(24));
		mmio_setbits_32(REG_SRC_GLOBAL_SRMR5, BIT(8));
		mmio_clrbits_32(REG_SRC_GLOBAL_SRMASK, BIT(1) | BIT(0));

		mmio_setbits_32(BLK_CTL_S_AONMIX_BASE + M33_CFG_OFF, M33_CPU_WAIT_MASK);
		mmio_clrbits_32(REG_SRC_GLOBAL_CM_QUIESCE, BIT(0));
		wdog1_cpu(0);
		val = mmio_read_32(REG_SRC_GLOBAL_CM_QUIESCE);

		udelay(5);

		val = mmio_read_32(REG_SRC_GLOBAL_CM_QUIESCE);

		while ((val & BIT(1)) == 2) {
			val = mmio_read_32(REG_SRC_GLOBAL_CM_QUIESCE);
		}

		val = mmio_read_32(REG_SRC_M33P_FUNC_STAT);
		while((val & SRC_MIX_SLICE_FUNC_STAT_RST_STAT) == 0) {
			val = mmio_read_32(REG_SRC_M33P_FUNC_STAT);
		}

		SMC_SET_GP(handle, CTX_GPREG_X1, 0);
		SMC_SET_GP(handle, CTX_GPREG_X2, 0);
		break;
	default:
		return SMC_UNK;

	};

	return 0;
}
