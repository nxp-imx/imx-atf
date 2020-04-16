/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <drivers/delay_timer.h>
#include <lib/libc/errno.h>

#include <gpc.h>
#include <imx8m_psci.h>
#include <imx_sip_svc.h>
#include <plat_imx8.h>

#define FSL_SIP_CONFIG_GPC_PM_DOMAIN		0x03

#define M4_LPA_ACTIVE	0x5555
#define M4_LPA_IDLE	0x0

static uint32_t gpc_imr_offset[] = {
	IMR1_CORE0_A53, IMR1_CORE1_A53,
	IMR1_CORE2_A53, IMR1_CORE3_A53,
};

DEFINE_BAKERY_LOCK(gpc_lock);

struct plat_gic_ctx imx_gicv3_ctx;

#pragma weak imx_set_cpu_pwr_off
#pragma weak imx_set_cpu_pwr_on
#pragma weak imx_set_cpu_lpm
#pragma weak imx_set_cluster_powerdown

bool imx_m4_lpa_active(void)
{
	return mmio_read_32(IMX_SRC_BASE + LPA_STATUS) == M4_LPA_ACTIVE;
}

bool imx_is_m4_enabled(void)
{
	return !(mmio_read_32(IMX_M4_STATUS) & IMX_M4_ENABLED_MASK);
}

void imx_set_cpu_secure_entry(unsigned int core_id, uintptr_t sec_entrypoint)
{
	uint64_t temp_base;

	temp_base = (uint64_t) sec_entrypoint;
	temp_base >>= 2;

	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3),
		((uint32_t)(temp_base >> 22) & 0xffff));
	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (core_id << 3) + 4,
		((uint32_t)temp_base & 0x003fffff));
}

void imx_set_cpu_pwr_off(unsigned int core_id)
{

	bakery_lock_get(&gpc_lock);

	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));

	bakery_lock_release(&gpc_lock);

	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
}

void imx_set_cpu_pwr_on(unsigned int core_id)
{
	bakery_lock_get(&gpc_lock);

	/* clear the wfi power down bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));

	bakery_lock_release(&gpc_lock);

	/* assert the ncpuporeset */
	mmio_clrbits_32(IMX_SRC_BASE + SRC_A53RCR1, (1 << core_id));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* sw power up the core */
	mmio_setbits_32(IMX_GPC_BASE + CPU_PGC_UP_TRG, (1 << core_id));

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + CPU_PGC_UP_TRG) & (1 << core_id)) != 0)
		;

	/* deassert the pcg pcr bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* deassert the ncpuporeset */
	mmio_setbits_32(IMX_SRC_BASE + SRC_A53RCR1, (1 << core_id));
}

void imx_set_cpu_lpm(unsigned int core_id, bool pdn)
{
	bakery_lock_get(&gpc_lock);

	if (pdn) {
		/* enable the core WFI PDN & IRQ PUP */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));
		/* assert the pcg pcr bit of the core */
		mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	} else {
		/* disbale CORE WFI PDN & IRQ PUP */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));
		/* deassert the pcg pcr bit of the core */
		mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	}

	bakery_lock_release(&gpc_lock);
}

/*
 * the plat and noc can only be power up & down by slot method,
 * slot0: plat power down; slot1: noc power down; slot2: noc power up;
 * slot3: plat power up. plat's pup&pdn ack is used by default. if
 * noc is config to power down, then noc's pdn ack should be used.
 */
static void imx_a53_plat_slot_config(bool pdn)
{
	if (pdn) {
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(0), PLAT_PDN_SLT_CTRL);
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(3), PLAT_PUP_SLT_CTRL);
		mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_PLAT_PDN_ACK |
			A53_PLAT_PUP_ACK);
		mmio_setbits_32(IMX_GPC_BASE + PLAT_PGC_PCR, 0x1);
	} else {
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(0), PLAT_PDN_SLT_CTRL);
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(3), PLAT_PUP_SLT_CTRL);
		mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_DUMMY_PUP_ACK |
			A53_DUMMY_PDN_ACK);
		mmio_clrbits_32(IMX_GPC_BASE + PLAT_PGC_PCR, 0x1);
	}
}

void imx_set_cluster_standby(bool enter)
{
	/*
	 * Enable BIT 6 of A53 AD register to make sure system
	 * don't enter LPM mode.
	 */
	if (enter)
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
	else
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, (1 << 6));
}

/* i.mx8mq need to override it */
void imx_set_cluster_powerdown(unsigned int last_core, uint8_t power_state)
{
	uint32_t val;

	if (!is_local_state_run(power_state)) {
		/* config C0~1's LPM, enable a53 clock off in LPM */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, A53_CLK_ON_LPM,
			LPM_MODE(power_state));
		/* config C2-3's LPM */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, LPM_MODE(power_state));

		/* enable PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val &= ~EN_L2_WFI_PDN;
		/* L2 cache memory is on in WAIT mode */
		if (is_local_state_off(power_state)) {
			val |= (L2PGE | EN_PLAT_PDN);
			imx_a53_plat_slot_config(true);
		}

		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);
	} else {
		/* clear the slot and ack for cluster power down */
		imx_a53_plat_slot_config(false);
		/* reverse the cluster level setting */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, 0xf, A53_CLK_ON_LPM);
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_BSC2, 0xf);

		/* clear PLAT/SCU power down */
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_AD, (L2PGE | EN_PLAT_PDN),
			EN_L2_WFI_PDN);
	}
}

static unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ISENABLER_SHIFT;

	return mmio_read_32(base + GICD_ISENABLER + (n << 2));
}

#pragma weak imx_set_sys_wakeup
/*
 * gic's clock will be gated in system suspend, so gic has no ability to
 * to wakeup the system, we need to config the imr based on the irq
 * enable status in gic, then gpc will monitor the wakeup irq
 */
void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	uint32_t irq_mask;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn)
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, A53_CORE_WUP_SRC(last_core),
			IRQ_SRC_A53_WUP);
	else
		mmio_clrsetbits_32(IMX_GPC_BASE + LPCR_A53_BSC, IRQ_SRC_A53_WUP,
			A53_CORE_WUP_SRC(last_core));

	/* clear last core's IMR based on GIC's mask setting */
	for (int i = 0; i < IRQ_IMR_NUM; i++) {
		if (pdn)
			/* set the wakeup irq base GIC */
			irq_mask = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		else
			irq_mask = IMR_MASK_ALL;

		mmio_write_32(IMX_GPC_BASE + gpc_imr_offset[last_core] + i * 4,
			      irq_mask);
	}

	/* enable the MU wakeup */
	if (imx_is_m4_enabled())
		mmio_clrbits_32(IMX_GPC_BASE + gpc_imr_offset[last_core] + 0x8, BIT(24));
}

void imx_noc_slot_config(bool pdn)
{
	if (pdn) {
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(1), NOC_PDN_SLT_CTRL);
		mmio_setbits_32(IMX_GPC_BASE + SLTx_CFG(2), NOC_PUP_SLT_CTRL);
		/* clear a53's PDN ack, use NOC's PDN ack */
		mmio_clrsetbits_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_PLAT_PDN_ACK, NOC_PGC_PDN_ACK);
		mmio_setbits_32(IMX_GPC_BASE + NOC_PGC_PCR, 0x1);
	} else {
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(1), NOC_PDN_SLT_CTRL);
		mmio_clrbits_32(IMX_GPC_BASE + SLTx_CFG(2), NOC_PUP_SLT_CTRL);
		mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, A53_DUMMY_PUP_ACK | A53_DUMMY_PDN_ACK);
		mmio_clrbits_32(IMX_GPC_BASE + NOC_PGC_PCR, 0x1);
	}
}

/* this is common for all imx8m soc */
void imx_set_sys_lpm(unsigned int last_core, bool retention)
{
	if (retention)
		mmio_clrsetbits_32(IMX_GPC_BASE + SLPCR, SLPCR_A53_FASTWUP_STOP_MODE,
			SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS | SLPCR_BYPASS_PMIC_READY);
	else
		mmio_clrsetbits_32(IMX_GPC_BASE + SLPCR, SLPCR_EN_DSM | SLPCR_VSTBY |
			 SLPCR_SBYOS | SLPCR_BYPASS_PMIC_READY, SLPCR_A53_FASTWUP_STOP_MODE);

	/* mask M4 DSM trigger if M4 is NOT enabled */
	if (!imx_is_m4_enabled())
		mmio_setbits_32(IMX_GPC_BASE + LPCR_M4, BIT(31));

	/* config wakeup irqs' mask in gpc */
	imx_set_sys_wakeup(last_core, retention);
}

void imx_set_rbc_count(void)
{
	mmio_setbits_32(IMX_GPC_BASE + SLPCR, SLPCR_RBC_EN |
		(0x8 << SLPCR_RBC_COUNT_SHIFT));
}

void imx_clear_rbc_count(void)
{
	mmio_clrbits_32(IMX_GPC_BASE + SLPCR, SLPCR_RBC_EN |
		(0x3f << SLPCR_RBC_COUNT_SHIFT));
}

#define MAX_PLL_NUM	10

struct pll_override pll[MAX_PLL_NUM] = {
	{.reg = 0x0, .override_mask = (1 << 12) | (1 << 8), },
	{.reg = 0x14, .override_mask = (1 << 12) | (1 << 8), },
	{.reg = 0x28, .override_mask = (1 << 12) | (1 << 8), },
	{.reg = 0x50, .override_mask = (1 << 12) | (1 << 8), },
	{.reg = 0x64, .override_mask = (1 << 10) | (1 << 8), },
	{.reg = 0x74, .override_mask = (1 << 10) | (1 << 8), },
	{.reg = 0x84, .override_mask = (1 << 10) | (1 << 8), },
	{.reg = 0x94, .override_mask = 0x5555500, },
	{.reg = 0x104, .override_mask = 0x5555500, },
	{.reg = 0x114, .override_mask = 0x500, },
};

#define PLL_BYPASS	BIT(4)

#pragma weak imx_anamix_override
void imx_anamix_override(bool enter)
{
	int i;

	/*
	 * bypass all the plls & enable the override bit before
	 * entering DSM mode.
	 */
	for (i = 0; i < MAX_PLL_NUM; i++) {
		if (enter) {
			mmio_setbits_32(IMX_ANAMIX_BASE + pll[i].reg, PLL_BYPASS);
			mmio_setbits_32(IMX_ANAMIX_BASE + pll[i].reg, pll[i].override_mask);
		} else {
			mmio_clrbits_32(IMX_ANAMIX_BASE + pll[i].reg, PLL_BYPASS);
			mmio_clrbits_32(IMX_ANAMIX_BASE + pll[i].reg, pll[i].override_mask);
		}
	}
}

#pragma weak imx_gpc_handler
int imx_gpc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3)
{
	switch(x1) {
	case FSL_SIP_CONFIG_GPC_PM_DOMAIN:
		imx_gpc_pm_domain_enable(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}

#pragma weak imx_src_handler
/* imx8mq/imx8mm need to verrride below function */
int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	uint32_t val;
	uint64_t timeout;
	int ret1 = 0, ret2 = 0;
	uint32_t offset;

	switch(x1) {
	case IMX_SIP_SRC_M4_START:
		mmio_clrbits_32(IMX_IOMUX_GPR_BASE + 0x58, 0x1);
		break;
	case IMX_SIP_SRC_M4_STARTED:
		val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x58);
		return !(val & 0x1);
	case IMX_SIP_SRC_M4_STOP:
		/*
		 * Safe stop
		 *    If M4 already in WFI,  perform below steps.
		 * a)	Set [0x303A_002C].0=0   [ request SLEEPHOLDREQn ]
		 * b)	Wait  [0x303A_00EC].1 = 0  [ wait SLEEPHOLDACKn ]
		 * c)	Set  GPR.CPUWAIT=1
		 * d)	Set [0x303A_002C].0=1  [ de-assert SLEEPHOLDREQn ]
		 * e)	Set SRC_M7_RCR[3:0] = 0xE0   [ reset M7 core/plat ]
		 * f)	Wait SRC_M7_RCR[3:0] = 0x8
		 * The following steps move to start part.
		 * g/h is actually no needed here.
		 * g)	Init TCM or DDR
		 * h)	Set GPR.INITVTOR
		 * i)	Set GPR.CPUWAIT=0,  M7 starting running
		 */
		offset = LPS_CPU1;
		val = mmio_read_32(IMX_GPC_BASE + offset);
		/* Not in stop/wait mode */
		if (!(val & (0x3 << 24))) {
			mmio_clrbits_32(IMX_GPC_BASE + 0x2C, 0x1);

			timeout = timeout_init_us(10000);
			while((mmio_read_32(IMX_GPC_BASE + offset) & 0x2)) {
				if (timeout_elapsed(timeout)) {
					ret1 = -ETIMEDOUT;
					break;
				}
			}
		}
		mmio_setbits_32(IMX_IOMUX_GPR_BASE + 0x58, 0x1);
		mmio_setbits_32(IMX_GPC_BASE + 0x2C, 0x1);
		mmio_setbits_32(IMX_SRC_BASE + 0xC, 0xE);
		timeout = timeout_init_us(10000);
		while ((mmio_read_32(IMX_SRC_BASE + 0xC) & 0xF) != 8) {
			if (timeout_elapsed(timeout)) {
				ret2 = -ETIMEDOUT;
				break;
			}
		}
		SMC_SET_GP(handle, CTX_GPREG_X1, ret1);
		SMC_SET_GP(handle, CTX_GPREG_X2, ret2);
		break;
	default:
		return SMC_UNK;

	};

	return 0;
}
