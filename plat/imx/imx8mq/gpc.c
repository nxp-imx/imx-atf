/*
 * Copyright 2017 NXP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

#define GPC_LPCR_A53_BSC	0x0
#define GPC_LPCR_A53_BSC2	0x108
#define GPC_LPCR_A53_AD		0x4
#define GPC_LPCR_M4		0x8
#define GPC_SLPCR		0x14
#define GPC_MST_CPU_MAPPING	0x18
#define GPC_PGC_ACK_SEL_A53	0x24
#define GPC_IMR1_CORE0_A53	0x30
#define GPC_IMR2_CORE0_A53	0x34
#define GPC_IMR3_CORE0_A53	0x38
#define GPC_IMR4_CORE0_A53	0x3c
#define GPC_IMR1_CORE1_A53	0x40
#define GPC_IMR2_CORE1_A53	0x44
#define GPC_IMR3_CORE1_A53	0x48
#define GPC_IMR4_CORE1_A53	0x4c
#define GPC_IMR1_CORE0_M4		0x50
#define GPC_IMR1_CORE2_A53	0x1c0
#define GPC_IMR2_CORE2_A53	0x1c4
#define GPC_IMR3_CORE2_A53	0x1c8
#define GPC_IMR4_CORE2_A53	0x1cc
#define GPC_IMR1_CORE3_A53	0x1d0
#define GPC_IMR2_CORE3_A53	0x1d4
#define GPC_IMR3_CORE3_A53	0x1d8
#define GPC_IMR4_CORE3_A53	0x1dc
#define GPC_PGC_CPU_0_1_MAPPING	0xec
#define GPC_PGC_CORE0_TIMMING	0x804
#define GPC_PGC_CORE1_TIMMING	0x844
#define GPC_PGC_CORE2_TIMMING	0x884
#define GPC_PGC_CORE3_TIMMING	0x8c4
#define GPC_PGC_SCU_TIMMING	0x910
#define GPC_SLOT0_CFG		0xb0

#define LPCR_A53_BSC_CPU_CLK_ON_LPM	0x4000
#define LPCR_A53_BSC_LPM0		0x3
#define LPCR_A53_BSC_LPM1		0xc
#define LPCR_A53_BSC2_LPM2		0x3
#define LPCR_A53_BSC2_LPM3		0xc

#define LPCR_A53_AD_L2PGE		0x80000000
#define LPCR_A53_AD_EN_C3_PUP		0x8000000
#define LPCR_A53_AD_EN_C3_IRQ_PUP	0x4000000
#define LPCR_A53_AD_EN_C2_PUP		0x2000000
#define LPCR_A53_AD_EN_C2_IRQ_PUP	0x1000000
#define LPCR_A53_AD_EN_C3_PDN		0x80000
#define LPCR_A53_AD_EN_C3_WFI_PDN	0x40000
#define LPCR_A53_AD_EN_C2_PDN		0x20000
#define LPCR_A53_AD_EN_C2_WFI_PDN	0x10000
#define LPCR_A53_AD_EN_C1_PUP		0x800
#define LPCR_A53_AD_EN_C1_IRQ_PUP	0x400
#define LPCR_A53_AD_EN_C0_PUP		0x200
#define LPCR_A53_AD_EN_C0_IRQ_PUP	0x100
#define LPCR_A53_AD_EN_L2_WFI_PDN	0x20
#define LPCR_A53_AD_EN_PLAT_PDN		0x10
#define LPCR_A53_AD_EN_C1_PDN		0x8
#define LPCR_A53_AD_EN_C1_WFI_PDN	0x4
#define LPCR_A53_AD_EN_C0_PDN		0x2
#define LPCR_A53_AD_EN_C0_WFI_PDN	0x1

#define A53_LPM_WAIT			0x5
#define A53_LPM_STOP			0xa

#define SLPCR_EN_DSM			0x80000000
#define SLPCR_RBC_EN			0x40000000
#define SLPCR_VSTBY			0x4
#define SLPCR_SBYOS			0x2
#define SLPCR_BYPASS_PMIC_READY		0x1
#define SLPCR_A53_FASTWUP_STOP		(1 << 17)
#define SLPCR_A53_FASTWUP_WAIT		(1 << 16)

#define GPC_CPU_PGC_SW_PUP_REQ		0xf0
#define GPC_CPU_PGC_SW_PDN_REQ		0xf4
#define BM_CPU_PGC_SW_PDN_PUP_REQ 	0x1

#define GPC_ARM_PGC		0x800
#define GPC_SCU_PGC		0x900
#define PGC_PCR			0

#define ANAMIX_HW_AUDIO_PLL1_CFG0	0x0
#define ANAMIX_HW_AUDIO_PLL2_CFG0	0x8
#define ANAMIX_HW_VIDEO_PLL1_CFG0	0x10
#define ANAMIX_HW_GPU_PLL_CFG0		0x18
#define ANAMIX_HW_VPU_PLL_CFG0		0x20
#define ANAMIX_HW_ARM_PLL_CFG0		0x28
#define ANAMIX_HW_SYS_PLL1_CFG0		0x30
#define ANAMIX_HW_SYS_PLL2_CFG0		0x3c
#define ANAMIX_HW_SYS_PLL3_CFG0		0x48
#define ANAMIX_HW_VIDEO_PLL2_CFG0	0x54
#define ANAMIX_HW_DRAM_PLL_CFG0		0x60
#define ANAMIX_HW_ANAMIX_MISC_CFG	0x70

static uint32_t gpc_saved_imrs[128];
static uint32_t gpc_wake_irqs[128];

static uint32_t gpc_pu_m_core_offset[11] = {
	0xc00, 0xc40, 0xc80, 0xcc0,
	0xdc0, 0xe00, 0xe40, 0xe80,
	0xec0, 0xf00, 0xf40,
};

void imx_gpc_set_m_core_pgc(unsigned int offset, bool pdn)
{
	uintptr_t val;

	val = mmio_read_32(IMX_GPC_BASE + offset);
	val &= ~(0x1 << PGC_PCR);

	if(pdn)
		val |= 0x1 << PGC_PCR;
	mmio_write_32(IMX_GPC_BASE + offset, val);
}

void imx_gpc_set_lpm_mode(enum imx_cpu_pwr_mode mode)
{
	uint32_t val1, val2, val3, val4;

	val1 = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC);
	val2 = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC2);
	val3 = mmio_read_32(IMX_GPC_BASE + GPC_SLPCR);
	val4 = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_AD);

	/* all core's LPM setting must be same */
	val1 &= ~(LPCR_A53_BSC_LPM0 | LPCR_A53_BSC_LPM1);
	val2 &= ~(LPCR_A53_BSC2_LPM2 | LPCR_A53_BSC2_LPM3);
	val3 &= ~(SLPCR_EN_DSM | SLPCR_RBC_EN | SLPCR_VSTBY |
		  SLPCR_SBYOS | SLPCR_BYPASS_PMIC_READY);
	val4 |= (1 << 5);

	switch(mode) {
	case WAIT_CLOCKED:
		break;
	case WAIT_UNCLOCKED:
		val1 |= A53_LPM_WAIT;
		val2 |= A53_LPM_WAIT;
		val1 &= ~LPCR_A53_BSC_CPU_CLK_ON_LPM;
		break;
	case STOP_POWER_ON:
		val1 |= A53_LPM_STOP;
		val2 |= A53_LPM_STOP;
		val1 &= ~LPCR_A53_BSC_CPU_CLK_ON_LPM;
		val3 |= SLPCR_EN_DSM;
		val3 |= SLPCR_RBC_EN;
		val3 |= SLPCR_BYPASS_PMIC_READY;
		break;
	case STOP_POWER_OFF:
		val1 |= A53_LPM_STOP;
		val2 |= A53_LPM_STOP;
		val1 &= ~LPCR_A53_BSC_CPU_CLK_ON_LPM;
		val3 |= SLPCR_EN_DSM;
		val3 |= SLPCR_A53_FASTWUP_STOP;
		val3 |= SLPCR_RBC_EN;
		val3 |= SLPCR_VSTBY;
		val3 |= SLPCR_SBYOS;
		val3 |= SLPCR_BYPASS_PMIC_READY;
		val4 &= ~(1 << 5);
		break;
	default:
		break;
	}

	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC, val1);
	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC2, val2);
	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_AD, val4);
	mmio_write_32(IMX_GPC_BASE + GPC_SLPCR, val3);
}

/* enable CORE LPM PDN/WUP in AD register */
void imx_gpc_set_cpu_power_gate_by_lpm(unsigned int cpu, bool pdn)
{
	uint32_t mask, val;

	val = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_AD);

	switch(cpu) {
	case 0:
		mask = LPCR_A53_AD_EN_C0_PDN | LPCR_A53_AD_EN_C0_PUP;
		break;
	case 1:
		mask = LPCR_A53_AD_EN_C1_PDN | LPCR_A53_AD_EN_C1_PUP;
		break;
	case 2:
		mask = LPCR_A53_AD_EN_C2_PDN | LPCR_A53_AD_EN_C3_PUP;
		break;
	case 3:
		mask = LPCR_A53_AD_EN_C3_PDN | LPCR_A53_AD_EN_C3_PUP;
		break;
	default:
		return;
	}

	if (pdn)
		val |= mask;
	else
		val &= ~mask;

	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_AD, val);
}

/* enable PLAT/SCU power down in AD register */
void imx_gpc_set_plat_power_gate_by_lpm(bool pdn)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_AD);
	val &= ~(LPCR_A53_AD_EN_PLAT_PDN | LPCR_A53_AD_L2PGE);

	if (pdn)
		val |= LPCR_A53_AD_EN_PLAT_PDN | LPCR_A53_AD_L2PGE;

	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_AD, val);
}

void imx_gpc_set_slot_ack(uint32_t index, enum imx_gpc_slot m_core,
			  bool mode, bool ack)
{
	uint32_t val, shift;

	if (index > 10) {
		tf_printf("Invalid slot index\n");
		return;
	}
	/* set slot */
	val = mmio_read_32(IMX_GPC_BASE + GPC_SLOT0_CFG + index * 4);
	val |= (mode + 1) << m_core * 2;
	mmio_write_32(IMX_GPC_BASE + GPC_SLOT0_CFG + index * 4, val);
	/* set ack */
	if (ack) {
		shift = m_core >= A53_SCU  ? 2 : 0;
		val = mmio_read_32(IMX_GPC_BASE + GPC_PGC_ACK_SEL_A53);
		/* clear dummy ack */
		val &= ~(1 << (15 + (mode ? 16: 0)));
		val |= 1 << (shift + (mode ? 16 : 0));
		mmio_write_32(IMX_GPC_BASE + GPC_PGC_ACK_SEL_A53, val);
	}
}

/* cpu: cpu index */
void imx_gpc_set_core_pdn_pup_by_software(unsigned int cpu, bool pdn)
{
	uint32_t val, index;

	val = mmio_read_32(IMX_GPC_BASE	+ (pdn ?
		GPC_CPU_PGC_SW_PDN_REQ : GPC_CPU_PGC_SW_PUP_REQ));

	/*Set the core PCR bit before sw PUP/PDN trigger */
	imx_gpc_set_m_core_pgc(GPC_ARM_PGC + cpu * 0x40, true);

	index = cpu < 2 ? cpu : cpu + 1;
	val |= (BM_CPU_PGC_SW_PDN_PUP_REQ << index);
	mmio_write_32(IMX_GPC_BASE + (pdn ?
		GPC_CPU_PGC_SW_PDN_REQ : GPC_CPU_PGC_SW_PUP_REQ), val);

	while((mmio_read_32(IMX_GPC_BASE + (pdn ?
		GPC_CPU_PGC_SW_PDN_REQ : GPC_CPU_PGC_SW_PUP_REQ)) &
		BM_CPU_PGC_SW_PDN_PUP_REQ) != 0)
		;

	/*Clear the core PCR bit after sw PUP/PDN trigger */
	imx_gpc_set_m_core_pgc(GPC_ARM_PGC + cpu * 0x40, false);
}

void imx_gpc_pre_suspend(bool arm_power_off)
{
	unsigned int i;

	/* set the LPM mode */
	if (arm_power_off) {
		/* enable core 0/1/2/3 power down/up with low power mode */
		/* enable plat power down/up with low power mode */

		/* 
		 * to avoid confuse, we use slot 0~4 for power down.
		 * slot 5~9 for power up.
		 * power down slot sequence:
		 * slot 0 -> CORE0,
		 * SLOT 1 -> Mega/Fast mix,
		 * SLOT 2 -> SCU
		 *
		 * SLOT 5 -> Mega/Fast mix,
		 * SLOT 6 -> SCU
		 * SLOT 7 -> CORE0
		 */
		 /* SCU slot ack as the power down ack */
		 /* CORE0 slot ack as the power up ack */
		imx_gpc_set_lpm_mode(STOP_POWER_OFF);
		imx_gpc_set_slot_ack(0, A53_CORE0, false, false);
		imx_gpc_set_slot_ack(2, A53_SCU, false, true);

		imx_gpc_set_slot_ack(6, A53_SCU, true, false);
		imx_gpc_set_slot_ack(7, A53_CORE0, true, true);
		imx_gpc_set_m_core_pgc(GPC_ARM_PGC, true);
		imx_gpc_set_m_core_pgc(GPC_SCU_PGC, true);

		imx_gpc_set_cpu_power_gate_by_lpm(0, true);
		imx_gpc_set_plat_power_gate_by_lpm(true);
	} else {
		imx_gpc_set_lpm_mode(STOP_POWER_ON);
	}

	for (i = 0; i < 4; i++) {
		gpc_saved_imrs[i] = mmio_read_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + i * 4);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + i * 4, ~gpc_wake_irqs[i]);
	}

	/* override PLL/OSC to let ccm control them */
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL1_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL2_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL1_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_GPU_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_GPU_PLL_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VPU_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VPU_PLL_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_ARM_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_ARM_PLL_CFG0) | 0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL1_CFG0) | 0x1555540);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL2_CFG0) | 0x1555540);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL3_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL3_CFG0) | 0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL2_CFG0) | 0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_DRAM_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_DRAM_PLL_CFG0) | 0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_ANAMIX_MISC_CFG,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_ANAMIX_MISC_CFG) | 0xa);
}

void imx_gpc_post_resume(void)
{
	int i;
	/* set LPM mode WAIT CLOCKED */
	imx_gpc_set_lpm_mode(WAIT_CLOCKED);
	/* clear lpm power gate of core and plat */
	imx_gpc_set_cpu_power_gate_by_lpm(0, false);
	imx_gpc_set_plat_power_gate_by_lpm(false);
	/* clear PGC PDN bit */
	imx_gpc_set_m_core_pgc(GPC_ARM_PGC, false);
	imx_gpc_set_m_core_pgc(GPC_SCU_PGC, false);

	for (i = 0; i < 4; i++) {
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + i * 4, gpc_saved_imrs[i]);
	}

	/* skip slot m4 use , clear slots */
	for(i = 0; i < 10; i ++) {
		if (i == 1 || i == 5)
			continue;
		mmio_write_32(IMX_GPC_BASE +GPC_SLOT0_CFG + i * 0x4, 0x0);
	}
	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_ACK_SEL_A53, 1 << 31 | 1 << 15);

	/* clear override of PLL/OSC */
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL1_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_AUDIO_PLL2_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL1_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_GPU_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_GPU_PLL_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VPU_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VPU_PLL_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_ARM_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_ARM_PLL_CFG0) & ~0x140000);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL1_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL1_CFG0) & ~0x1555540);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL2_CFG0) & ~0x1555540);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL3_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_SYS_PLL3_CFG0) & ~0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL2_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_VIDEO_PLL2_CFG0) & ~0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_DRAM_PLL_CFG0,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_DRAM_PLL_CFG0) & ~0x140);
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_HW_ANAMIX_MISC_CFG,
		mmio_read_32(IMX_ANAMIX_BASE + ANAMIX_HW_ANAMIX_MISC_CFG) & ~0xa);
}

static void imx_gpc_hwirq_mask(unsigned int hwirq)
{
	uintptr_t reg;
	unsigned int val;

	reg = IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val |= 1 << hwirq % 32;
	mmio_write_32(reg, val);
}

static void imx_gpc_hwirq_unmask(unsigned int hwirq)
{
	uintptr_t reg;
	unsigned int val;

	reg = IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val &= ~(1 << hwirq % 32);
	mmio_write_32(reg, val);
}

static void imx_gpc_set_wake(uint32_t hwirq, unsigned int on)
{
	uint32_t mask, idx;

	mask = 1 << hwirq % 32;
	idx = hwirq / 32;
	gpc_wake_irqs[idx] = on ? gpc_wake_irqs[idx] | mask :
				 gpc_wake_irqs[idx] & ~mask;
}

static void imx_gpc_pm_domain_enable(uint32_t domain_id, uint32_t on)
{
	uint32_t val;
	uintptr_t reg;

	imx_gpc_set_m_core_pgc(gpc_pu_m_core_offset[domain_id], true);

	reg = IMX_GPC_BASE + (on ? 0xf8 : 0x104);
	val = 1 << (domain_id > 3 ? (domain_id + 3) : domain_id);
	mmio_write_32(reg, val);
	while(mmio_read_32(reg) & val)
		;
	imx_gpc_set_m_core_pgc(gpc_pu_m_core_offset[domain_id], false);
}

void imx_gpc_init(void)
{
	unsigned int val;
	int i;
	/* mask all the interrupt by default */
	for (i = 0; i < 4; i ++) {
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_M4 + i * 4, ~0x0);
	}

	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53, ~0x1);

	/* use external IRQs to wakeup C0~C3 from LPM */
	val = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC);
	val |= 0x70c00000;
	/* clear the MASTER0 LPM handshake */
	val &= ~(1 << 6);
	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_A53_BSC, val);

	/* mask M4 DSM trigger if M4 is NOT enabled */
	val = mmio_read_32(IMX_GPC_BASE + GPC_LPCR_M4);
	val |= 1 << 31;
	mmio_write_32(IMX_GPC_BASE + GPC_LPCR_M4, val);

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CPU_0_1_MAPPING, 0xfffd);

	/* set SCU timming */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_SCU_TIMMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set A53 core power up timming */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CORE0_TIMMING, 0x1a << 7);
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CORE1_TIMMING, 0x1a << 7);
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CORE2_TIMMING, 0x1a << 7);
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CORE3_TIMMING, 0x1a << 7);

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_ACK_SEL_A53, 1 << 31 | 1 << 15);
	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + GPC_SLPCR);
	val &= ~(1 << 31);
	/* TODO if M4 is not enabled, clear more SLPCR bits */
	mmio_write_32(IMX_GPC_BASE + GPC_SLPCR, val);
}

int imx_gpc_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch(x1) {
	case FSL_SIP_CONFIG_GPC_SET_WAKE:
		imx_gpc_set_wake(x2, x3);
		break;
	case FSL_SIP_CONFIG_GPC_MASK:
		imx_gpc_hwirq_mask(x2);
		break;
	case FSL_SIP_CONFIG_GPC_UNMASK:
		imx_gpc_hwirq_unmask(x2);
		break;
	case FSL_SIP_CONFIG_GPC_PM_DOMAIN:
		imx_gpc_pm_domain_enable(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}
