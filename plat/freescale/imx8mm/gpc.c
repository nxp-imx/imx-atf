/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <gicv3.h>
#include <stdlib.h>
#include <stdint.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <types.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <platform_def.h>
#include <psci.h>
#include <fsl_sip.h>
#include <soc.h>

#define GPC_PGC_ACK_SEL_A53	0x24
#define GPC_IMR1_CORE0_A53	0x30
#define GPC_IMR1_CORE1_A53	0x40
#define GPC_IMR1_CORE0_M4	0x50
#define GPC_IMR1_CORE2_A53	0x1c0
#define GPC_IMR1_CORE3_A53	0x1d0
#define GPC_PGC_CPU_0_1_MAPPING	0xec
#define GPC_PGC_SCU_TIMMING	0x910

#define CPU_PGC_SW_PUP_TRG		0xf0
#define CPU_PGC_SW_PDN_TRG		0xfc
#define BM_CPU_PGC_SW_PDN_PUP_REQ 	0x1

#define PGC_PCR			0

/* BSC */
#define LPCR_A53_BSC			0x0
#define LPCR_A53_BSC2			0x108
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
#define A53_CLK_ON_LPM			(1 << 14)

#define MST_CPU_MAPPING			0x18

#define SRC_GPR1_OFFSET			0x74

/* AD */
#define LPCR_A53_AD			0x4 
#define	L2PGE				(1 << 31)
#define EN_L2_WFI_PDN			(1 << 5)
#define EN_PLAT_PDN			(1 << 4)

#define PU_PGC_UP_TRG			0xf8
#define PU_PGC_DN_TRG			0x104
#define GPC_PU_PWRHSK			0x1fc

/* SLOT */
#define PGC_ACK_SEL_A53			0x24
#define SLT0_CFG			0xb0
#define SLT1_CFG			0xb4
#define SLT2_CFG			0xb8
#define SLT3_CFG			0xbc

/* ack for slot pup/pdn */
#define A53_DUMMY_PGC_PUP_ACK	(1 << 31)
#define NOC_PGC_PUP_ACK		(1 << 19)
#define PLAT_PGC_PUP_ACK	(1 << 18)
#define A53_DUMMY_PGC_PDN_ACK	(1 << 15)
#define NOC_PGC_PDN_ACK		(1 << 3)
#define PLAT_PGC_PDN_ACK	(1 << 2)

/* pdn/pup bit define for slot control */
#define NOC_PUP_SLT_CTRL	(1 << 11)
#define NOC_PDN_SLT_CTRL	(1 << 10)
#define PLAT_PUP_SLT_CTRL	(1 << 9)
#define PLAT_PDN_SLT_CTRL	(1 << 8)

#define PLAT_PGC_PCR		0x900
#define NOC_PGC_PCR		0xa40

#define MIPI_PGC               0xc00
#define PCIE_PGC               0xc40
#define OTG1_PGC               0xc80
#define OTG2_PGC               0xcc0
#define GPU2D_PGC              0xd80
#define GPUMIX_PGC             0xdc0
#define VPUMIX_PGC             0xe00
#define GPU3D_PGC              0xe40
#define DISPMIX_PGC            0xe80
#define VPU_G1_PGC             0xec0
#define VPU_G2_PGC             0xf00
#define VPU_H1_PGC             0xf40

#define MIPI_PWR_REQ		(1 << 0)
#define PCIE_PWR_REQ		(1 << 1)
#define OTG1_PWR_REQ		(1 << 2)
#define OTG2_PWR_REQ		(1 << 3)
#define GPU2D_PWR_REQ		(1 << 6)
#define GPUMIX_PWR_REQ		(1 << 7)
#define VPUMIX_PWR_REQ		(1 << 8)
#define GPU3D_PWR_REQ		(1 << 9)
#define DISPMIX_PWR_REQ		(1 << 10)
#define VPU_G1_PWR_REQ		(1 << 11)
#define VPU_G2_PWR_REQ		(1 << 12)
#define VPU_H1_PWR_REQ		(1 << 13)

#define DISPMIX_ADB400_SYNC	(1 << 7)
#define VPUMIX_ADB400_SYNC	(1 << 8)
#define GPU3D_ADB400_SYNC	(1 << 9)
#define GPU2D_ADB400_SYNC	(1 << 10)
#define GPUMIX_ADB400_SYNC	(1 << 11)
#define DISPMIX_ADB400_ACK	(1 << 25)
#define VPUMIX_ADB400_ACK	(1 << 26)
#define GPU3D_ADB400_ACK	(1 << 27)
#define GPU2D_ADB400_ACK	(1 << 28)
#define GPUMIX_ADB400_ACK	(1 << 29)

#define MIPI_PGC		0xc00
#define PCIE_PGC		0xc40
#define OTG1_PGC		0xc80
#define OTG2_PGC		0xcc0
#define GPU2D_PGC		0xd80
#define GPUMIX_PGC		0xdc0
#define VPUMIX_PGC		0xe00
#define GPU3D_PGC		0xe40
#define DISPMIX_PGC		0xe80
#define VPU_G1_PGC		0xec0
#define VPU_G2_PGC		0xf00
#define VPU_H1_PGC		0xf40

#define IMX_PD_DOMAIN(name)				\
	{						\
		.pwr_req = name##_PWR_REQ,		\
		.pgc_offset = name##_PGC,		\
		.need_sync = false,			\
		.init_on = true,			\
	}

#define IMX_MIX_DOMAIN(name)				\
	{						\
		.pwr_req = name##_PWR_REQ,		\
		.pgc_offset = name##_PGC,		\
		.adb400_sync = name##_ADB400_SYNC,	\
		.adb400_ack = name##_ADB400_ACK,	\
		.need_sync = true,			\
		.init_on = true,			\
	}

#define COREx_PGC_PCR(core_id)	(0x800 + core_id * 0x40)
#define COREx_WFI_PDN(core_id)	(1 << (core_id < 2 ? core_id * 2 : (core_id - 2) * 2 + 16))
#define COREx_IRQ_WUP(core_id)	(core_id < 2 ? (1 << (core_id * 2 + 8)) : (1 << (core_id * 2 + 20)));
#define LPM_MODE(local_state)	(local_state == PLAT_WAIT_RET_STATE ? A53_LPM_WAIT : A53_LPM_STOP)
#define A53_CORE_WUP_SRC(core_id) (1 << (core_id < 2 ? 28 + core_id : 22 + core_id - 2))

#define IMR_MASK_ALL		0xffffffff
#define IRQ_SRC_A53_WUP		30

struct imx_pwr_domain {
	uint32_t pwr_req;
	uint32_t adb400_sync;
	uint32_t adb400_ack;
	uint32_t pgc_offset;
	bool need_sync;
	bool init_on;
};

/* PU domain */
static struct imx_pwr_domain pu_domains[] = {
	IMX_PD_DOMAIN(MIPI),
	IMX_PD_DOMAIN(PCIE),
	IMX_PD_DOMAIN(OTG1),
	IMX_PD_DOMAIN(OTG2),
	IMX_MIX_DOMAIN(GPU2D),
	IMX_MIX_DOMAIN(GPUMIX),
	IMX_MIX_DOMAIN(VPUMIX),
	IMX_MIX_DOMAIN(GPU3D),
	IMX_MIX_DOMAIN(DISPMIX),
	IMX_PD_DOMAIN(VPU_G1),
	IMX_PD_DOMAIN(VPU_G2),
};

static uint32_t gpc_imr_offset[] = { 0x30, 0x40, 0x1c0, 0x1d0, };
/* save gic dist&redist context when NOC wrapper is power down */
static struct plat_gic_ctx imx_gicv3_ctx;

void imx_set_cpu_secure_entry(int core_id, uint64_t sec_entrypoint)
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
	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
	val |= COREx_WFI_PDN(core_id);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

	/* assert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id));
	val |= (1 << 0);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), val);
};

/* use the sw method to power up the core */
void imx_set_cpu_pwr_on(int core_id)
{
	uint32_t val;

	/* clear the wfi power down bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
	val &= ~COREx_WFI_PDN(core_id);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

	/* assert the ncpuporeset */
	val = mmio_read_32(IMX_SRC_BASE + 0x8);
	val &= ~(1 << core_id);
	mmio_write_32(IMX_SRC_BASE + 0x8, val);

	/* assert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id));
	val |= (1 << 0);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), val);

	/* sw power up the core */
	val = mmio_read_32(IMX_GPC_BASE + CPU_PGC_SW_PUP_TRG);
	val |= (1 << core_id);
	mmio_write_32(IMX_GPC_BASE + CPU_PGC_SW_PUP_TRG, val);

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + CPU_PGC_SW_PUP_TRG) & (1 << core_id)) != 0)
		;
	/* deassert the pcg pcr bit of the core */
	val = mmio_read_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id));
	val &= ~(1 << 0);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), val);

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
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		/* enable the core WFI power down */
		val |= COREx_WFI_PDN(core_id);
		/* enable the core IRQ wakeup */
		val |= COREx_IRQ_WUP(core_id);
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

		/* assert the pcg pcr bit of the core */
		val = mmio_read_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id));
		val |= (1 << 0);
		mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), val);
	} else {
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		/* disable the core WFI power down */
		val &= ~COREx_WFI_PDN(core_id);
		/* disable the core IRQ wakeup */
		val &= ~COREx_IRQ_WUP(core_id);
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);
		/* deassert the pcg pcr bit of the core */
		val = mmio_read_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id));
		val &= ~(1 << 0);
		mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), val);
	}
}

/*
 * SLOT 0 is used for A53 PLAT poewr down,
 * SLOT 1 is used for A53 NOC power down,
 * SLOT 2 is used for A53 NOC power up,
 * SLOT 3 is used for A53 PLAT power up,
 * when enter LPM power down, NOC's ACK is used,
 * when out of LPM, SCU's ACK is used
 */
void imx_a53_plat_slot_config(bool pdn)
{
	uint32_t pgc_pcr, slot_ack, pdn_slot_cfg, pup_slot_cfg;

	pdn_slot_cfg = mmio_read_32(IMX_GPC_BASE + SLT0_CFG);
	pup_slot_cfg = mmio_read_32(IMX_GPC_BASE + SLT3_CFG);
	pgc_pcr = mmio_read_32(IMX_GPC_BASE + PLAT_PGC_PCR);

		/* enable PLAT PGC PCR */
		pgc_pcr |= 0x1;
		mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR, pgc_pcr);


	if (pdn) {
		/* config a53 plat pdn/pup slot */
		pdn_slot_cfg |= PLAT_PDN_SLT_CTRL;
		pup_slot_cfg |= PLAT_PUP_SLT_CTRL;
		/* config a53 plat pdn/pup ack */
		slot_ack = PLAT_PGC_PDN_ACK | PLAT_PGC_PUP_ACK;
		/* enable PLAT PGC PCR */
		pgc_pcr |= 0x1;

	} else {
		/* clear slot/ack config */
		pdn_slot_cfg &= ~PLAT_PDN_SLT_CTRL;
		pup_slot_cfg &= ~PLAT_PUP_SLT_CTRL;
		slot_ack = A53_DUMMY_PGC_PDN_ACK | A53_DUMMY_PGC_PUP_ACK;
		/* enable PLAT PGC PCR */
		pgc_pcr &= ~0x1;
	}

	mmio_write_32(IMX_GPC_BASE + SLT0_CFG, pdn_slot_cfg);
	mmio_write_32(IMX_GPC_BASE + SLT3_CFG, pup_slot_cfg);
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, slot_ack);
	mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR, pgc_pcr);
}

void imx_noc_slot_config(bool pdn)
{
	uint32_t pgc_pcr, slot_ack, pdn_slot_cfg, pup_slot_cfg;

	pdn_slot_cfg = mmio_read_32(IMX_GPC_BASE + SLT1_CFG);
	pup_slot_cfg = mmio_read_32(IMX_GPC_BASE + SLT2_CFG);
	slot_ack = mmio_read_32(IMX_GPC_BASE + PGC_ACK_SEL_A53);
	pgc_pcr = mmio_read_32(IMX_GPC_BASE + NOC_PGC_PCR);

	if (pdn) {
		pdn_slot_cfg |= NOC_PDN_SLT_CTRL;
		pup_slot_cfg |= NOC_PUP_SLT_CTRL;

		/* clear a53's PDN ack, use NOC's PDN ack */
		slot_ack &= ~0xffff;
		slot_ack |= NOC_PGC_PDN_ACK;
		/* enable NOC PGC PCR */
		pgc_pcr |= 0x1;
	 } else {
		pdn_slot_cfg &= ~NOC_PDN_SLT_CTRL;
		pup_slot_cfg &= ~NOC_PUP_SLT_CTRL;
		slot_ack = A53_DUMMY_PGC_PUP_ACK | A53_DUMMY_PGC_PDN_ACK;
		/* disable NOC PGC PCR */
		pgc_pcr&= ~0x1;
	}

	mmio_write_32(IMX_GPC_BASE + SLT1_CFG, pdn_slot_cfg);
	mmio_write_32(IMX_GPC_BASE + SLT2_CFG, pup_slot_cfg);
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53, slot_ack);
	mmio_write_32(IMX_GPC_BASE + NOC_PGC_PCR, pgc_pcr);
}

/* TODO for cpu clock gate off wait mode */
void imx_set_cluster_standby(bool enter)
{

}

/* set the BSC and BSC2 LPM bit, and other bit in AD */
void imx_set_cluster_powerdown(int last_core, uint8_t power_state)
{
	uint32_t val;

	if (!is_local_state_run(power_state)) {

		/* config A53 cluster LPM mode */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val |= LPM_MODE(power_state); /* enable the C0~1 LPM */
		val &= ~A53_CLK_ON_LPM;
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		/* enable C2-3's LPM */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC2);
		val |= LPM_MODE(power_state);
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC2, val);

		/* enable PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val &= ~EN_L2_WFI_PDN;

		/* L2 cache memory is on in WAIT mode */
		if (is_local_state_off(power_state))
			val |= (L2PGE | EN_PLAT_PDN);
		else
			val |= EN_PLAT_PDN;

		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);

		/* config SLOT for PLAT power up/down */
		imx_a53_plat_slot_config(true);
	} else {
		/* clear the slot and ack for cluster power down */
		imx_a53_plat_slot_config(false);

		/* reverse the cluster level setting */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
		val &= ~0xf; /* clear the C0~1 LPM */
		val |= A53_CLK_ON_LPM;
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC2);
		val &= ~0xf;
		mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC2, val);

		/* clear PLAT/SCU power down */
		val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_AD);
		val |= EN_L2_WFI_PDN;
		val &= ~(L2PGE | EN_PLAT_PDN);
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
			SLPCR_BYPASS_PMIC_READY |
			SLPCR_RBC_EN | SLPCR_A53_FASTWUP_STOP);
		/* TODO DDR retention */
	} else {
		/* TODO DDR retention */
	}
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);
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
	/* TODO */
}

void imx_anamix_post_resume(void)
{
	/* TODO */
}

void noc_wrapper_pre_suspend(unsigned int proc_num)
{
	/* FIXME enable NOC power down on real silicon */
#if 0
	imx_noc_slot_config(true);
#endif
	/*
	 * gic redistributor context save must be called when
	 * the GIC CPU interface is disabled and before distributor save.
	 */
	plat_gic_save(proc_num, &imx_gicv3_ctx);
}

void noc_wrapper_post_resume(unsigned int proc_num)
{
	/* FIXME enable NOC power down on real silicon */
#if 0
	imx_noc_slot_config(false);
#endif
	/* restore gic context */
	plat_gic_restore(proc_num, &imx_gicv3_ctx);
}

/* use external IRQ wakeup source for LPM if NOC power down */
void imx_set_sys_wakeup(int last_core, bool pdn)
{
	uint32_t irq_mask, val;
	gicv3_dist_ctx_t *dist_ctx = &imx_gicv3_ctx.dist_ctx;

	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	if (pdn) {
		/* select the external IRQ as the LPM wakeup source */
		val |= (1 << IRQ_SRC_A53_WUP);
		/* select the external IRQ as last core's wakeup source */
		val &= ~A53_CORE_WUP_SRC(last_core); 
	} else {
		val &= ~(1 << IRQ_SRC_A53_WUP);
		val |= A53_CORE_WUP_SRC(last_core);
	}
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);
	
	/* clear last core's IMR based on GIC's mask setting */
	for (int i = 0; i < 4; i++) {
		if (pdn)
			irq_mask = ~dist_ctx->gicd_isenabler[i];
		else
			irq_mask = IMR_MASK_ALL;

		mmio_write_32(IMX_GPC_BASE + gpc_imr_offset[last_core] + i * 4,
			      irq_mask);
	}
}

static void imx_gpc_pm_domain_enable(uint32_t domain_id, uint32_t on)
{
	uint32_t val;

	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];

	/* FIXME, will remove after runtime PM is ok */
	return;

	if (on) {
		/* clear the PGC bit */
		val = mmio_read_32(IMX_GPC_BASE + pwr_domain->pgc_offset);
		val &= ~(1 << 0);
		mmio_write_32(IMX_GPC_BASE + pwr_domain->pgc_offset, val);

		/* power up the domain */
		val = mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG);
		val |= pwr_domain->pwr_req;
		mmio_write_32(IMX_GPC_BASE + PU_PGC_UP_TRG, val);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* clear adb power down request */
			val = mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK);
			val |= pwr_domain->adb400_sync;
			mmio_write_32(IMX_GPC_BASE + GPC_PU_PWRHSK, val);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		/* special fixup for dispmix */
		if (pwr_domain->pwr_req == DISPMIX_PWR_REQ) {
			mmio_write_32(0x303845d0, 0x3);
			mmio_write_32(0x32e28000, 0x7f);
			mmio_write_32(0x32e28004, 0x1fff);
			mmio_write_32(0x32e28008, 0x30000);
		}
	} else {
		/* TODO  for bringup purpose */
		return;
		/* set the PGC bit */
		val = mmio_read_32(IMX_GPC_BASE + pwr_domain->pgc_offset);
		val |= (1 << 0);
		mmio_write_32(IMX_GPC_BASE + pwr_domain->pgc_offset, val);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* set adb power down request */
			val = mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK);
			val &= ~(pwr_domain->adb400_sync);
			mmio_write_32(IMX_GPC_BASE + GPC_PU_PWRHSK, val);

			/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		/* power down the domain */
		val = mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG);
		val |= pwr_domain->pwr_req;
		mmio_write_32(IMX_GPC_BASE + PU_PGC_DN_TRG, val);
	}

	pwr_domain->init_on = false;
}

void imx_gpc_init(void)
{
	unsigned int val;
	int i;

	/* mask all the wakeup irq by default */
	for (i = 0; i < 4; i++) {
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_M4 + i * 4, ~0x0);
	}

	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	/* use GIC wake_request to wakeup C0~C3 from LPM */
	val |= 0x30c00000;
	/* clear the MASTER0 LPM handshake */
	val &= ~(1 << 6);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* clear MASTER1&MASTER2 mapping in CPU0(A53) */
	val = mmio_read_32(IMX_GPC_BASE + MST_CPU_MAPPING);
	val &= ~(0x3 << 1);
	mmio_write_32(IMX_GPC_BASE + MST_CPU_MAPPING, val);

	/* mask M4 DSM trigger if M4 is NOT enabled */
	val = mmio_read_32(IMX_GPC_BASE + LPCR_M4);
	val |= 1 << 31;
	mmio_write_32(IMX_GPC_BASE + LPCR_M4, val);

	/*set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CPU_0_1_MAPPING, 0xffff);

	/* TODO release dispmix sft reset */
	/* enable all the PU for bringup up purpose */
	mmio_write_32(0x30384450, 0x3);
	mmio_write_32(0x303844d0, 0x3);
	mmio_write_32(0x303844f0, 0x3);
	mmio_write_32(0x30384560, 0x3);
	mmio_write_32(0x30384570, 0x3);
	mmio_write_32(0x30384590, 0x3);
	mmio_write_32(0x303845a0, 0x3);
	mmio_write_32(0x303845d0, 0x3);
	mmio_write_32(0x30384630, 0x3);
	mmio_write_32(0x30384660, 0x3);
	mmio_write_32(IMX_GPC_BASE + 0xf8, 0x3fcf);
	mmio_write_32(0x32e28000, 0x7f);
	mmio_write_32(0x32e28004, 0x1fff);
	mmio_write_32(0x32e28008, 0x30000);

	/* set SCU timming */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_SCU_TIMMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53,
		      A53_DUMMY_PGC_PUP_ACK | A53_DUMMY_PGC_PDN_ACK);
	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~SLPCR_EN_DSM;
	/* enable the fast wakeup wait mode */
	val |= SLPCR_A53_FASTWUP_WAIT;
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
}

int imx_gpc_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
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
