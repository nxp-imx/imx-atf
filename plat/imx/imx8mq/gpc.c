/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <delay_timer.h>
#include <dram.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <spinlock.h>
#include <runtime_svc.h>
#include <std_svc.h>
#include <mmio.h>
#include <platform_def.h>
#include <imx_sip.h>
#include <soc.h>

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

#define GPC_CPU_PGC_SW_PUP_REQ		0xf0
#define GPC_CPU_PGC_SW_PDN_REQ		0xfc
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

#define LPCR_A53_BSC			0x0
#define LPCR_A53_BSC2			0x108
#define LPCR_A53_AD			0x4 
#define LPCR_M4				0x8
#define SLPCR				0x14
#define SLPCR_EN_DSM			(1 << 31)
#define SLPCR_RBC_EN			(1 << 30)
#define SLPCR_A53_FASTWUP_STOP		(1 << 17)
#define SLPCR_A53_FASTWUP_WAIT		(1 << 16)
#define SLPCR_VSTBY			(1 << 2)
#define SLPCR_SBYOS			(1 << 1)
#define SLPCR_BYPASS_PMIC_READY		0x1
#define A53_LPM_WAIT			0x5
#define A53_LPM_STOP			0xa

#define GPC_SLOT0_CFG 		0xb0
#define SRC_GPR1_OFFSET 	0x74

static bool is_pcie1_power_down = true;
static uint32_t gpc_saved_imrs[16];
static uint32_t gpc_wake_irqs[4];
static uint32_t gpc_imr_offset[] = {
	IMX_GPC_BASE + GPC_IMR1_CORE0_A53,
	IMX_GPC_BASE + GPC_IMR1_CORE1_A53,
	IMX_GPC_BASE + GPC_IMR1_CORE2_A53,
	IMX_GPC_BASE + GPC_IMR1_CORE3_A53,
	IMX_GPC_BASE + GPC_IMR1_CORE0_M4,
};

static uint32_t gpc_pu_m_core_offset[11] = {
	0xc00, 0xc40, 0xc80, 0xcc0,
	0xdc0, 0xe00, 0xe40, 0xe80,
	0xec0, 0xf00, 0xf40,
};

spinlock_t gpc_imr_lock[4];

static void gpc_imr_core_spin_lock(unsigned int core_id)
{
	spin_lock(&gpc_imr_lock[core_id]);
}

static void gpc_imr_core_spin_unlock(unsigned int core_id)
{
	spin_unlock(&gpc_imr_lock[core_id]);
}

void imx_gpc_set_m_core_pgc(unsigned int offset, bool pdn)
{
	uintptr_t val;

	val = mmio_read_32(IMX_GPC_BASE + offset);
	val &= ~(0x1 << PGC_PCR);

	if(pdn)
		val |= 0x1 << PGC_PCR;
	mmio_write_32(IMX_GPC_BASE + offset, val);
}

void imx_set_cpu_secure_entry(int core_id, uintptr_t sec_entrypoint)
{
	uint64_t temp_base;

	temp_base = (uint64_t) sec_entrypoint;
	temp_base >>= 2;

	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3),
		((uint32_t)(temp_base >> 22) & 0xffff));
	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3) + 4,
		((uint32_t)temp_base & 0x003fffff));
}

/* use wfi power down the core */
void imx_set_cpu_pwr_off(int core_id)
{
	uint32_t val;

	/* enable the wfi power down of the core */
	val = mmio_read_32(IMX_GPC_BASE + 0x4);
	val |= (1 << (core_id < 2 ? core_id * 2 : (core_id - 2) * 2 + 16));
	val |= 1 << (core_id + 20);
	mmio_write_32(IMX_GPC_BASE + 0x4, val);

	/* assert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id);
	val |= (1 << 0);
	mmio_write_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id, val);
};

/* use the sw method to power up the core */
void imx_set_cpu_pwr_on(int core_id)
{
	uint32_t val;

	/* clear the wfi power down bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + 0x4);
	val &= ~(1 << (core_id < 2 ? core_id * 2 : (core_id - 2) * 2 + 16));
	mmio_write_32(IMX_GPC_BASE + 0x4, val);

	/* assert the ncpuporeset */
	val = mmio_read_32(IMX_SRC_BASE + 0x8);
	val &= ~(1 << core_id);
	mmio_write_32(IMX_SRC_BASE + 0x8, val);

	/* assert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id);
	val |= (1 << 0);
	mmio_write_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id, val);

	/* sw power up the core */
	val = mmio_read_32(IMX_GPC_BASE + 0xf0);
	val |= (1 << core_id);
	mmio_write_32(IMX_GPC_BASE + 0xf0, val);

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + 0xf0) & (1 << core_id)) != 0)
		;

	/* deassert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id);
	val &= ~(1 << 0);
	mmio_write_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id, val);

	/* deassert the ncpuporeset */
	val = mmio_read_32(IMX_SRC_BASE + 0x8);
	val |= (1 << core_id);
	mmio_write_32(IMX_SRC_BASE + 0x8, val);
}

/* if out of lpm, we need to do reverse steps */
void imx_set_cpu_lpm(int core_id, bool pdn)
{
	uint32_t val;

	if (pdn) {
		val = mmio_read_32(IMX_GPC_BASE + 0x4);
		/* enable the core WFI power down */
		val |= (1 << (core_id < 2 ? core_id * 2 : (core_id - 2) * 2 + 16));
		val |= 1 << (core_id + 20);
		/* enable the core IRQ wakeup */
		val |= (core_id < 2 ? (1 << (core_id * 2 + 8)) : (1 << (core_id * 2 + 20)));
		mmio_write_32(IMX_GPC_BASE + 0x4, val);

		/* assert the pcg pcr bit of the core */
		val = mmio_read_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id);
		val |= (1 << 0);
		mmio_write_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id, val);
	} else {
		val = mmio_read_32(IMX_GPC_BASE + 0x4);
		/* disable the core WFI power down */
		val &= ~(1 << (core_id < 2 ? core_id * 2 : (core_id - 2) * 2 + 16));
		/* disable the core IRQ wakeup */
		val &= ~(core_id < 2 ? (1 << (core_id * 2 + 8)) : (1 << (core_id * 2 + 20)));
		mmio_write_32(IMX_GPC_BASE + 0x4, val);
		/* deassert the pcg pcr bit of the core */
		val = mmio_read_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id);
		val &= ~(1 << 0);
		mmio_write_32(IMX_GPC_BASE + 0x800 + 0x40 * core_id, val);
	}
}

static void gpc_save_imr_lpm(unsigned int core_id, unsigned int imr_idx)
{
	uint32_t reg = gpc_imr_offset[core_id] + imr_idx * 4;

	gpc_imr_core_spin_lock(core_id);

	gpc_saved_imrs[core_id + imr_idx * 4] = mmio_read_32(reg);
	mmio_write_32(reg, ~gpc_wake_irqs[imr_idx]);

	gpc_imr_core_spin_unlock(core_id);
}

static void gpc_restore_imr_lpm(unsigned int core_id, unsigned int imr_idx)
{
	uint32_t reg = gpc_imr_offset[core_id] + imr_idx * 4;
	uint32_t val = gpc_saved_imrs[core_id + imr_idx * 4];

	gpc_imr_core_spin_lock(core_id);

	mmio_write_32(reg, val);

	gpc_imr_core_spin_unlock(core_id);
}

/*
 * On i.MX8MQ, only in system suspend mode, the A53 cluster can
 * enter LPM mode and shutdown the A53 PLAT power domain. So LPM
 * wakeup only used for system suspend. when system enter suspend,
 * any A53 CORE can be the last core to suspend the system, But
 * the LPM wakeup can only use the C0's IMR to wakeup A53 cluster
 * from LPM, so save C0's IMRs before suspend, restore back after
 * resume.
 */
void inline imx_set_lpm_wakeup(bool pdn)
{
	unsigned int imr, core;

	if (pdn)
		for (imr = 0; imr < 4; imr++)
			for (core = 0; core < 4; core++)
				gpc_save_imr_lpm(core, imr);
	else
		for (imr = 0; imr < 4; imr++)
			for (core = 0; core < 4; core++)
				gpc_restore_imr_lpm(core, imr);
}

/* SLOT 0 is used for A53 PLAT poewr down */
/* SLOT 1 is used for A53 PLAT power up */
/* SLOT 2 is used for A53 last core power up */
/* when enter LPM power down, SCU's ACK is used */
/* when out of LPM, last_core's ACK is used */
void imx_pup_pdn_slot_config(int last_core, bool pdn)
{
	uint32_t slot_ack;
	uint32_t slot0_cfg, slot1_cfg, slot2_cfg;

	slot0_cfg = mmio_read_32(IMX_GPC_BASE + GPC_SLOT0_CFG);
	slot1_cfg = mmio_read_32(IMX_GPC_BASE + GPC_SLOT0_CFG + 0x4);
	slot2_cfg = mmio_read_32(IMX_GPC_BASE + GPC_SLOT0_CFG + 0x8);
	slot_ack  = mmio_read_32(IMX_GPC_BASE + 0x24);

	if (pdn) {
		/* PUP and PDN SLOT config */
		slot0_cfg |= (1 << 8);
		slot1_cfg |= (1 << 9);
		slot2_cfg |= (0x2 << last_core * 2);

		/*PDN ACK config */
		slot_ack &= ~(1 << 15);
		slot_ack |= (1 << 2);

		/*PUP ACK setting */
		slot_ack &= ~(1 << 31);
		slot_ack |= (last_core < 2 ? (1 << (last_core + 16)) :
			    (1 << (last_core + 27)));

	} else {
		slot0_cfg &= ~(1 << 8);
		slot1_cfg &= ~(1 << 9);
		slot2_cfg &= ~(0x2 << last_core * 2);

		slot_ack |= (1 << 15);
		slot_ack &= ~(1 << 2);

		slot_ack |= (1 << 31);
		slot_ack &= ~(last_core < 2 ? (1 << (last_core + 16)) :
			    (1 << (last_core + 27)));
	}

	mmio_write_32(IMX_GPC_BASE + GPC_SLOT0_CFG, slot0_cfg);
	mmio_write_32(IMX_GPC_BASE + GPC_SLOT0_CFG + 0x4, slot1_cfg);
	mmio_write_32(IMX_GPC_BASE + GPC_SLOT0_CFG + 0x8, slot2_cfg);
	mmio_write_32(IMX_GPC_BASE + 0x24, slot_ack);
}

/* used for cpuidle support on imx8mq */
void imx_set_cluster_standby(bool enter)
{
	uint32_t val;
	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
	/*
	 * Enable BIT 6 of A53 AD register to make sure
	 * system don't enter LPM mode.
	 */
	if (enter)
		val |= (1 << 6);
	else
		val &= ~(1 << 6);

	mmio_write_32(IMX_GPC_BASE  + LPCR_A53_AD, val);
}

/* only support the GPC STOP mode power off */
/* set the BSC and BSC2 LPM bit, and other bit in AD */
void imx_set_cluster_powerdown(int last_core, bool pdn)
{
	uint32_t val;

	if (pdn) {
		/*
		 * config the LPM STOP mode, enable CPU clock
		 * disable in LPM mode.
		 */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val |= 0xa; /* enable the C0~1 LPM */
		val &= ~(1 << 14); /* disable cpu clock in LPM */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		/* enable C2-3's LPM */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC2);
		val |= 0xa;
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC2, val);

		/* enable PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val &= ~(1 << 5);
		val |= ((1 << 31) | (1 << 4));
		/* enable C0's LPM power down */
		/*If cluster enter LPM, the last core's IRQ wakeup must be clear */
		val &= ~(last_core < 2 ? (1 << (last_core * 2 + 8)) : (1 << (last_core * 2 + 20)));
		/* enable the C0's LPM PUP */
		val |= (last_core < 2 ? (1 << (last_core * 2 + 9)) : (1 << (last_core * 2 + 21)));
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

		imx_pup_pdn_slot_config(last_core, true);
		imx_set_lpm_wakeup(true);

		/* enable PLAT PGC */
		val = mmio_read_32(IMX_GPC_BASE + 0x900);
		val |= 0x1;
		mmio_write_32(IMX_GPC_BASE + 0x900, val);
	} else {
		/* clear PLAT PGC */
		val = mmio_read_32(IMX_GPC_BASE + 0x900);
		val &= ~0x1;
		mmio_write_32(IMX_GPC_BASE + 0x900, val);

		/* clear the slot and ack for cluster power down */
		imx_pup_pdn_slot_config(last_core, false);
		imx_set_lpm_wakeup(false);

		/* reverse the cluster level setting */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val &= ~0xa; /* clear the C0~1 LPM */
		val |= (1 << 14); /* disable cpu clock in LPM */
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC2);
		val &= ~0xa;
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC2, val);

		/* clear PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val |= (1 << 5);
		val &= ~((1 << 31) | (1 << 4));
		/* disable C0's LPM PUP */
		val &= ~(last_core < 2 ? (1 << (last_core * 2 + 9)) : (1 << (last_core * 2 + 21)));
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);
	}
}

/* only handle the SLPCR and DDR retention */
/* config the PLLs override setting */
void imx_set_sys_lpm(bool retention)
{
	uint32_t val;

	/* set system DSM mode SLPCR(0x14) */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
		 SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN);

	if (retention) {
		val |= (SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
			 SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN);

		/* DDR enter retention */
		dram_enter_retention();
	} else {
		/* DDR exit retention */
		dram_exit_retention();
	}

	mmio_write_32(IMX_GPC_BASE + 0x14, val);
}

void imx_set_rbc_count(void)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val |= (0x3f << 24);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);
}

void imx_clear_rbc_count(void)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(0x3f << 24);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

}
void imx_anamix_pre_suspend()
{
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

void imx_anamix_post_resume(void)
{
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

	gpc_imr_core_spin_lock(0);
	reg = gpc_imr_offset[0] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val |= 1 << hwirq % 32;
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(0);
}

static void imx_gpc_hwirq_unmask(unsigned int hwirq)
{
	uintptr_t reg;
	unsigned int val;

	gpc_imr_core_spin_lock(0);
	reg = gpc_imr_offset[0] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val &= ~(1 << hwirq % 32);
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(0);
}

static void imx_gpc_set_wake(uint32_t hwirq, unsigned int on)
{
	uint32_t mask, idx;

	mask = 1 << hwirq % 32;
	idx = hwirq / 32;
	gpc_wake_irqs[idx] = on ? gpc_wake_irqs[idx] | mask :
				 gpc_wake_irqs[idx] & ~mask;
}

static void imx_gpc_mask_irq0(uint32_t core_id, uint32_t mask)
{
	gpc_imr_core_spin_lock(core_id);
	if (mask)
		mmio_setbits_32(gpc_imr_offset[core_id], 1);
	else
		mmio_clrbits_32(gpc_imr_offset[core_id], 1);
	dsb();
	gpc_imr_core_spin_unlock(core_id);
}

void imx_gpc_core_wake(uint32_t cpumask)
{
	for (int i = 0; i < 4; i++)
		if (cpumask & (1 << i))
			imx_gpc_mask_irq0(i, false);
}

void imx_gpc_set_a53_core_awake(uint32_t core_id)
{
	imx_gpc_mask_irq0(core_id, true);
}

static void imx_gpc_set_affinity(uint32_t hwirq, unsigned cpu_idx)
{
	uintptr_t reg;
	unsigned int val;

	/*
	 * using the mask/unmask bit as affinity function.unmask the
	 * IMR bit to enable IRQ wakeup for this core.
	 */
	gpc_imr_core_spin_lock(cpu_idx);
	reg = gpc_imr_offset[cpu_idx] + (hwirq / 32) * 4;
	val = mmio_read_32(reg);
	val &= ~(1 << hwirq % 32);
	mmio_write_32(reg, val);
	gpc_imr_core_spin_unlock(cpu_idx);

	/* clear affinity of other core */
	for (int i = 0; i < 4; i++) {
		if (cpu_idx != i) {
			gpc_imr_core_spin_lock(i);
			reg = gpc_imr_offset[i] + (hwirq / 32) * 4;
			val = mmio_read_32(reg);
			val |= (1 << hwirq % 32);
			mmio_write_32(reg, val);
			gpc_imr_core_spin_unlock(i);
		}
	}
}

static void imx_gpc_pm_domain_enable(uint32_t domain_id, uint32_t on)
{
	uint32_t val;
	uintptr_t reg;

	/*
	 * PCIE1 and PCIE2 share the same reset signal, if we power down
	 * PCIE2, PCIE1 will be hold in reset too.
	 * 1. when we want to power up PCIE1, the PCIE2 power domain also
	 *    need to be power on;
	 * 2. when we want to power down PCIE2 power domain, we should make
	 *    sure PCIE1 is already power down.
	*/
	if (domain_id == 1 && !on) {
		is_pcie1_power_down = true;
	} else if (domain_id == 1 && on) {
		imx_gpc_pm_domain_enable(10, true);
		is_pcie1_power_down = false;
	}

	if (domain_id == 10 && !on && !is_pcie1_power_down)
		return;

	/* need to handle GPC_PU_PWRHSK */
	/* GPU */
	if (domain_id == 4 && !on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) & ~0x40);
	if (domain_id == 4 && on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) | 0x40);
	/* VPU */
	if (domain_id == 5 && !on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) & ~0x20);
	if (domain_id == 5 && on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) | 0x20);
	/* DISPLAY */
	if (domain_id == 7 && !on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) & ~0x10);
	if (domain_id == 7 && on)
		mmio_write_32(0x303a01fc, mmio_read_32(0x303a01fc) | 0x10);

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
	int i, j;

	/* mask all the interrupt by default */
	for (i = 0; i < 4; i++)
		for (j = 0; j < 5; j++)
			mmio_write_32(gpc_imr_offset[j] + i * 4, ~0x0);

	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	for (i = 0; i < 4; i++)
		mmio_write_32(gpc_imr_offset[i], ~0x1);

	/* leave the IOMUX_GPC bit 12 on for core wakeup */
	mmio_setbits_32(IMX_IOMUX_GPR_BASE + 0x4, 1 << 12);

	/* use external IRQs to wakeup C0~C3 from LPM */
	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	val |= 0x40000000;
	/* clear the MASTER0 LPM handshake */
	val &= ~(1 << 6);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* mask M4 DSM trigger if M4 is NOT enabled */
	val = mmio_read_32(IMX_GPC_BASE + LPCR_M4);
	val |= 1 << 31;
	mmio_write_32(IMX_GPC_BASE + LPCR_M4, val);

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CPU_0_1_MAPPING, 0xfffd);

	/* set SCU timming */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_SCU_TIMMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_ACK_SEL_A53, 1 << 31 | 1 << 15);
	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(1 << 31);
	/* TODO if M4 is not enabled, clear more SLPCR bits */
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

	/*
	 * USB PHY power up needs to make sure RESET bit in SRC is clear,
	 * otherwise, the PU power up bit in GPC will NOT self-cleared.
	 * only need to do it once.
	 */
	val = mmio_read_32(0x30390020);
	val &= ~0x1;
	mmio_write_32(0x30390020, val);
	val = mmio_read_32(0x30390024);
	val &= ~0x1;
	mmio_write_32(0x30390024, val);

	/* for USB OTG, the limitation are:
	 * 1. before system clock config, the IPG clock run at 12.5MHz, delay time
	 *    should be longer than 82us.
	 * 2. after system clock config, ipg clock run at 66.5MHz, delay time
	 *    be longer that 15.3 us.
	 *    Add 100us to make sure the USB OTG SRC is clear safely.
	 */
	udelay(100);
}

int imx_gpc_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch(x1) {
	case FSL_SIP_CONFIG_GPC_CORE_WAKE:
		imx_gpc_core_wake(x2);
		break;
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
	case FSL_SIP_CONFIG_GPC_SET_AFF:
		imx_gpc_set_affinity(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}
