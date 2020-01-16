/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <gicv3.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <runtime_svc.h>
#include <std_svc.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <platform_def.h>
#include <psci.h>
#include <imx_sip.h>
#include <tzc380.h>
#include <soc.h>
#include <delay_timer.h>
#include <bakery_lock.h>

#define GPC_PGC_ACK_SEL_A53	0x24
#define GPC_IMR1_CORE0_A53	0x30
#define GPC_IMR1_CORE1_A53	0x44
#define GPC_IMR1_CORE0_M4	0x58
#define GPC_IMR1_CORE2_A53	0x194
#define GPC_IMR1_CORE3_A53	0x1a8
#define GPC_PGC_CPU_0_1_MAPPING	0x1cc
#define GPC_PGC_SCU_TIMMING	0x910

#define CPU_PGC_SW_PUP_TRG		0xd0
#define CPU_PGC_SW_PDN_TRG		0xdc
#define BM_CPU_PGC_SW_PDN_PUP_REQ 	0x1

#define PGC_PCR			0

/* BSC */
#define LPCR_A53_BSC			0x0
#define LPCR_A53_BSC2			0x180
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
#define SLPCR_RBC_SHIFT			24
#define SLPCR_STBY_COUNT_SHFT		3

#define MST_CPU_MAPPING			0x18

#define SRC_GPR1_OFFSET			0x74

/* AD */
#define LPCR_A53_AD			0x4 
#define	L2PGE				(1 << 31)
#define EN_L2_WFI_PDN			(1 << 5)
#define EN_PLAT_PDN			(1 << 4)

#define PU_PGC_UP_TRG			0xd8
#define PU_PGC_DN_TRG			0xe4

/* SLOT */
#define PGC_ACK_SEL_A53			0x24
#define SLT0_CFG			0x200
#define SLT1_CFG			0x204
#define SLT2_CFG			0x208
#define SLT3_CFG			0x20c

/* ack for slot pup/pdn */
#define A53_DUMMY_PGC_PUP_ACK	(1 << 31)
#define NOC_PGC_PUP_ACK		(1 << 13)
#define PLAT_PGC_PUP_ACK	(1 << 9)
#define A53_DUMMY_PGC_PDN_ACK	(1 << 30)
#define NOC_PGC_PDN_ACK		(1 << 12)
#define PLAT_PGC_PDN_ACK	(1 << 8)

/* pdn/pup bit define for slot control */
#define NOC_PUP_SLT_CTRL	(1 << 13)
#define NOC_PDN_SLT_CTRL	(1 << 12)
#define PLAT_PUP_SLT_CTRL	(1 << 9)
#define PLAT_PDN_SLT_CTRL	(1 << 8)

#define PLAT_PGC_PCR		0x900
#define NOC_PGC_PCR		0xa40

#define MIPI_PHY1_PWR_REQ	BIT(0)
#define PCIE_PHY_PWR_REQ	BIT(1)
#define USB1_PHY_PWR_REQ	BIT(2)
#define USB2_PHY_PWR_REQ	BIT(3)
#define MLMIX_PWR_REQ		BIT(4)
#define AUDIOMIX_PWR_REQ	BIT(5)
#define GPU2D_PWR_REQ		BIT(6)
#define GPUMIX_PWR_REQ		BIT(7)
#define VPUMIX_PWR_REQ		BIT(8)
#define GPU3D_PWR_REQ		BIT(9)
#define MEDIAMIX_PWR_REQ	BIT(10)
#define VPU_G1_PWR_REQ		BIT(11)
#define VPU_G2_PWR_REQ		BIT(12)
#define VPU_H1_PWR_REQ		BIT(13)
#define HDMIMIX_PWR_REQ		BIT(14)
#define HDMI_PHY_PWR_REQ	BIT(15)
#define MIPI_PHY2_PWR_REQ	BIT(16)
#define HSIOMIX_PWR_REQ		BIT(17)
#define DDRMIX_PWR_REQ		BIT(18)
#define MEDIAMIX_ISPDWP_PWR_REQ	BIT(19)

#define AUDIOMIX_ADB400_SYNC	(1 << 4 | 1 << 15)
#define MLMIX_ADB400_SYNC	(1 << 7 | 1 << 8)
#define GPUMIX_ADB400_SYNC	BIT(9)
#define VPUMIX_ADB400_SYNC	BIT(10)
#define HSIOMIX_ADB400_SYNC	BIT(12)
#define HDMIMIX_ADB400_SYNC	BIT(13)
#define MEDIAMIX_ADB400_SYNC	BIT(14)

#define AUDIOMIX_ADB400_ACK	(1 << 20 | 1 << 31)
#define MLMIX_ADB400_ACK	(1 << 23 | 1 << 30)
#define GPUMIX_ADB400_ACK	BIT(25)
#define VPUMIX_ADB400_ACK	BIT(26)
#define HSIOMIX_ADB400_ACK	BIT(28)
#define HDMIMIX_ADB400_ACK	BIT(29)
#define MEDIAMIX_ADB400_ACK	BIT(30)

#define MIPI_PHY1_PGC		0xb00
#define PCIE_PHY_PGC		0xb40
#define USB1_PHY_PGC		0xb80
#define USB2_PHY_PGC		0xbc0
#define MLMIX_PGC		0xc00
#define AUDIOMIX_PGC		0xc40
#define GPU2D_PGC		0xc80
#define GPUMIX_PGC		0xcc0
#define VPUMIX_PGC		0xd00
#define GPU3D_PGC		0xd40
#define MEDIAMIX_PGC		0xd80
#define VPU_G1_PGC		0xdc0
#define VPU_G2_PGC		0xe00
#define VPU_H1_PGC		0xe40
#define HDMIMIX_PGC		0xe80
#define HDMI_PHY_PGC		0xec0
#define MIPI_PHY2_PGC		0xf00
#define HSIOMIX_PGC		0xf40
#define DDRMIX_PGC		0xf80
#define MEDIAMIX_ISPDWP_PGC	0xfc0


#define M4RCR 	0xC
#define SRC_SCR_M4C_NON_SCLR_RST_MASK		(1 << 0)

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
#define COREx_IRQ_WUP(core_id)	(core_id < 2 ? (1 << (core_id * 2 + 8)) : (1 << (core_id * 2 + 20)))
#define LPM_MODE(local_state)	(local_state == PLAT_WAIT_RET_STATE ? A53_LPM_WAIT : A53_LPM_STOP)
#define A53_CORE_WUP_SRC(core_id) (1 << (core_id < 2 ? 28 + core_id : 22 + core_id - 2))

#define IMR_MASK_ALL		0xffffffff
#define IRQ_SRC_A53_WUP		30

/*
 * 0:mipi_phy1; 1: pcie_phy; 2: usb1_phy; 3: usb2_phy; 4: mlmix; 5: audiomix; 6: gpu2d;
 * 7: gpu_share; 8: vpu_share; 9: gpu3d; 10: mediamix; 11:vpu_g1; 12:vpu_g2;
 * 13: vpu_h1; 14: hdmimix; 15: hdmi_phy; 16: mipi_phy2; 17: hsiomix; 18: ddrmix; 19: mediamix_ispdwp
 */
struct imx_pwr_domain {
	uint32_t pwr_req;
	uint32_t adb400_sync;
	uint32_t adb400_ack;
	uint32_t pgc_offset;
	bool need_sync;
	bool init_on;
};

struct imx_noc_setting {
       uint32_t domain_id;
       uint32_t start;
       uint32_t end;
       uint32_t prioriy;
       uint32_t mode;
       uint32_t socket_qos_en;
};

enum pu_domain_id {
	/* hsio ss */
	HSIOMIX,
	PCIE_PHY,
	USB1_PHY,
	USB2_PHY,
	MLMIX,
	AUDIOMIX,
	/* gpu ss */
	GPUMIX,
	GPU2D,
	GPU3D,
	/* vpu ss */
	VPUMIX,
	VPU_G1,
	VPU_G2,
	VPU_H1,
	/* media ss */
	MEDIAMIX,
	MEDIAMIX_ISPDWP,
	MIPI_PHY1,
	MIPI_PHY2,
	/* HDMI ss */
	HDMIMIX,
	HDMI_PHY,
	DDRMIX,
};

/* PU domain, add some hole to minimize the uboot change */
static struct imx_pwr_domain pu_domains[20] = {
	[MIPI_PHY1] = IMX_PD_DOMAIN(MIPI_PHY1),
	[PCIE_PHY] = IMX_PD_DOMAIN(PCIE_PHY),
	[USB1_PHY] = IMX_PD_DOMAIN(USB1_PHY),
	[USB2_PHY] = IMX_PD_DOMAIN(USB2_PHY),
	[MLMIX] = IMX_MIX_DOMAIN(MLMIX),
	[AUDIOMIX] = IMX_MIX_DOMAIN(AUDIOMIX),
	[GPU2D] = IMX_PD_DOMAIN(GPU2D),
	[GPUMIX] = IMX_MIX_DOMAIN(GPUMIX),
	[VPUMIX] = IMX_MIX_DOMAIN(VPUMIX),
	[GPU3D] = IMX_PD_DOMAIN(GPU3D),
	[MEDIAMIX] = IMX_MIX_DOMAIN(MEDIAMIX),
	[VPU_G1] = IMX_PD_DOMAIN(VPU_G1),
	[VPU_G2] = IMX_PD_DOMAIN(VPU_G2),
	[VPU_H1] = IMX_PD_DOMAIN(VPU_H1),
	[HDMIMIX] = IMX_MIX_DOMAIN(HDMIMIX),
	[HDMI_PHY] = IMX_PD_DOMAIN(HDMI_PHY),
	[MIPI_PHY2] = IMX_PD_DOMAIN(MIPI_PHY2),
	[HSIOMIX] = IMX_MIX_DOMAIN(HSIOMIX),
	[MEDIAMIX_ISPDWP] = IMX_PD_DOMAIN(MEDIAMIX_ISPDWP),
};

static struct imx_noc_setting noc_setting[] = {
       {MLMIX, 0x180, 0x180, 0x80000303, 0x0, 0x0},
       {AUDIOMIX, 0x200, 0x480, 0x80000404, 0x0, 0x0},
       {GPUMIX, 0x500, 0x580, 0x80000303, 0x0, 0x0},
       {HDMIMIX, 0x600, 0x680, 0x80000202, 0x0, 0x1},
       {HDMIMIX, 0x700, 0x700, 0x80000505, 0x0, 0x0},
       {HSIOMIX, 0x780, 0x900, 0x80000303, 0x0, 0x0},
       {MEDIAMIX, 0x980, 0xb80, 0x80000202, 0x0, 0x1},
       {MEDIAMIX_ISPDWP, 0xc00, 0xd00, 0x80000505, 0x0, 0x0},
       //can't access VPU NoC if only power up VPUMIX,
       //need to power up both NoC and IP
       {VPU_G1, 0xd80, 0xd80, 0x80000303, 0x0, 0x0},
       {VPU_G2, 0xe00, 0xe00, 0x80000303, 0x0, 0x0},
       {VPU_H1, 0xe80, 0xe80, 0x80000303, 0x0, 0x0}
};

static uint32_t gpc_imr_offset[] = { 0x30, 0x40, 0x1c0, 0x1d0, };
/* save gic dist&redist context when NOC wrapper is power down */
static struct plat_gic_ctx imx_gicv3_ctx;

static unsigned int pu_domain_status;

DEFINE_BAKERY_LOCK(gpc_lock);

bool imx_is_m4_enabled(void)
{
	return !( mmio_read_32(IMX_IOMUX_GPR_BASE + 0x58) & 0x1);
}

#define M4_LPA_ACTIVE	0x5555
#define M4_LPA_IDLE	0x0
#define SRC_GPR10	0x98

bool imx_m4_lpa_active(void)
{
	return mmio_read_32(IMX_SRC_BASE + SRC_GPR10) & M4_LPA_ACTIVE;
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

	bakery_lock_get(&gpc_lock);

	/* enable the wfi power down of the core */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));

	bakery_lock_release(&gpc_lock);

	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
};

/* use the sw method to power up the core */
void imx_set_cpu_pwr_on(int core_id)
{

	bakery_lock_get(&gpc_lock);

	/* clear the wfi power down bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id));

	bakery_lock_release(&gpc_lock);

	/* assert the ncpuporeset */
	mmio_clrbits_32(IMX_SRC_BASE + 0x8, 0x1 << core_id);

	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);

	/* sw power up the core */
	mmio_setbits_32(IMX_GPC_BASE + CPU_PGC_SW_PUP_TRG, 0x1 << core_id);

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + CPU_PGC_SW_PUP_TRG) & (0x1 << core_id)) != 0)
		;
	/* deassert the pcg pcr bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);

	/* deassert the ncpuporeset */
	mmio_setbits_32(IMX_SRC_BASE + 0x8, 0x1 << core_id);
}

/* if out of lpm, we need to do reverse steps */
void imx_set_cpu_lpm(int core_id, bool pdn)
{

	bakery_lock_get(&gpc_lock);

	if (pdn) {
		/* enable the core WFI PDN & IRQ WUP */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));

		/* assert the pcg pcr bit of the core */
		mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	} else {
		/* disable the core WFI PDN & IRQ WUP */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_AD, COREx_WFI_PDN(core_id) |
				COREx_IRQ_WUP(core_id));

		/* deassert the pcg pcr bit of the core */
		mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	}

	bakery_lock_release(&gpc_lock);
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
		if (is_local_state_off(power_state)) {
			val |= (L2PGE | EN_PLAT_PDN);

			/* config SLOT for PLAT power up/down */
			imx_a53_plat_slot_config(true);
		}

		mmio_write_32(IMX_GPC_BASE + LPCR_A53_AD, val);
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
		 SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN |
		 SLPCR_A53_FASTWUP_STOP);

	if (retention)
		val |= (SLPCR_EN_DSM | SLPCR_VSTBY | SLPCR_SBYOS |
			SLPCR_BYPASS_PMIC_READY | SLPCR_RBC_EN);

	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

#if 0
	if (imx_is_m4_enabled() && imx_m4_lpa_active()) {
		val = mmio_read_32(IMX_GPC_BASE + SLPCR);
		val |= SLPCR_A53_FASTWUP_STOP;
		mmio_write_32(IMX_GPC_BASE + 0x14, val);
			return;
	}
#endif

	if (!imx_is_m4_enabled()) {
		/* mask M4 DSM trigger if M4 is NOT enabled */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_M4, (0x1 << 31));
	}
}

void imx_set_rbc_count(void)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val |= SLPCR_RBC_EN;
	val |= (0xa << SLPCR_RBC_SHIFT);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);
}

void imx_clear_rbc_count(void)
{
	uint32_t val;

	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~(SLPCR_RBC_EN);
	val &= ~(0x3f << SLPCR_RBC_SHIFT);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

}

static int pll_ctrl_offset[] = {0x0, 0x14, 0x28, 0x50, 0x64, 0x74, 0x84, 0x94, 0x104, 0x114, };
#define PLL_BYPASS	BIT(4)

void imx_anamix_pre_suspend()
{
	int i;
	uint32_t pll_ctrl;
	if (imx_is_m4_enabled() && imx_m4_lpa_active())
		return;
	/* bypass all the plls before enter DSM mode */
	for (i = 0; i < ARRAY_SIZE(pll_ctrl_offset); i++) {
		pll_ctrl = mmio_read_32(IMX_ANAMIX_BASE + pll_ctrl_offset[i]);
		pll_ctrl |= PLL_BYPASS;
		mmio_write_32(IMX_ANAMIX_BASE + pll_ctrl_offset[i], pll_ctrl);
	}

	/* enable plls override bit to power down in dsm */
	mmio_write_32(IMX_ANAMIX_BASE + 0x0, mmio_read_32(IMX_ANAMIX_BASE + 0x0) | ((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x14, mmio_read_32(IMX_ANAMIX_BASE + 0x14) | ((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x28, mmio_read_32(IMX_ANAMIX_BASE + 0x28) | ((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x50, mmio_read_32(IMX_ANAMIX_BASE + 0x50) | ((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x64, mmio_read_32(IMX_ANAMIX_BASE + 0x64) | ((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x74, mmio_read_32(IMX_ANAMIX_BASE + 0x74) | ((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x84, mmio_read_32(IMX_ANAMIX_BASE + 0x84) | ((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x94, mmio_read_32(IMX_ANAMIX_BASE + 0x94) | 0x5555500);
	mmio_write_32(IMX_ANAMIX_BASE + 0x104, mmio_read_32(IMX_ANAMIX_BASE + 0x104) | 0x5555500);
	mmio_write_32(IMX_ANAMIX_BASE + 0x114, mmio_read_32(IMX_ANAMIX_BASE + 0x114) | 0x500);
}

void imx_anamix_post_resume(void)
{
	int i;
	uint32_t pll_ctrl;
	if (imx_is_m4_enabled() && imx_m4_lpa_active())
		return;
	/* unbypass all the plls after exit from DSM mode */
	for (i = 0; i < ARRAY_SIZE(pll_ctrl_offset); i++) {
		pll_ctrl = mmio_read_32(IMX_ANAMIX_BASE + pll_ctrl_offset[i]);
		pll_ctrl &= ~PLL_BYPASS;
		mmio_write_32(IMX_ANAMIX_BASE + pll_ctrl_offset[i], pll_ctrl);
	}

	/* clear plls override bit */
	mmio_write_32(IMX_ANAMIX_BASE + 0x0, mmio_read_32(IMX_ANAMIX_BASE + 0x0) & ~((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x14, mmio_read_32(IMX_ANAMIX_BASE + 0x14) & ~((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x28, mmio_read_32(IMX_ANAMIX_BASE + 0x28) & ~((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x50, mmio_read_32(IMX_ANAMIX_BASE + 0x50) & ~((1 << 12) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x64, mmio_read_32(IMX_ANAMIX_BASE + 0x64) & ~((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x74, mmio_read_32(IMX_ANAMIX_BASE + 0x74) & ~((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x84, mmio_read_32(IMX_ANAMIX_BASE + 0x84) & ~((1 << 10) | (1 << 8)));
	mmio_write_32(IMX_ANAMIX_BASE + 0x94, mmio_read_32(IMX_ANAMIX_BASE + 0x94) & ~0x5555500);
	mmio_write_32(IMX_ANAMIX_BASE + 0x104, mmio_read_32(IMX_ANAMIX_BASE + 0x104) & ~0x5555500);
	mmio_write_32(IMX_ANAMIX_BASE + 0x114, mmio_read_32(IMX_ANAMIX_BASE + 0x114) & ~0x500);
}

#define GPR_TZASC_EN		(1 << 0)
#define GPR_TZASC_EN_LOCK	(1 << 16)

#if 0
static void imx8mm_tz380_init(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x28);
	if ((val & GPR_TZASC_EN) != GPR_TZASC_EN)
		return;

	tzc380_init(IMX_TZASC_BASE);

	/* Enable 1G-5G S/NS RW */
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G)
		| TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_ALL);
}
#endif

void noc_wrapper_pre_suspend(unsigned int proc_num)
{
#if 0
	if (!imx_is_m4_enabled() || !imx_m4_lpa_active()) {
		/* enable MASTER1 & MASTER2 power down in A53 LPM mode */
		mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_BSC, (0x1 << 8) | (0x1 << 7));

		mmio_setbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (0x3 << 1));

		/* noc can only be power down when all the pu domain is off */
		if (!pu_domain_status)
			/* enable noc power down */
			imx_noc_slot_config(true);
	}
#endif
	/*
	 * gic redistributor context save must be called when
	 * the GIC CPU interface is disabled and before distributor save.
	 */
	plat_gic_save(proc_num, &imx_gicv3_ctx);
}

void noc_wrapper_post_resume(unsigned int proc_num)
{
#if 0

	if (!imx_is_m4_enabled() || !imx_m4_lpa_active()) {
		/* disable MASTER1 & MASTER2 power down in A53 LPM mode */
		mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_BSC, (0x1 << 8) | (0x1 << 7));

		mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (0x3 << 1));

		/* noc can only be power down when all the pu domain is off */
		if (!pu_domain_status) {
			/* re-init the tz380 if resume from noc power down */
			imx8mm_tz380_init();
			/* disable noc power down */
			imx_noc_slot_config(false);
		}
	}
#endif

    /* config main NoC */
    //A53
    mmio_write_32 (0x32700008, 0x80000303);
    mmio_write_32 (0x3270000c, 0x0);
    //SUPERMIX
    mmio_write_32 (0x32700088, 0x80000303);
    mmio_write_32 (0x3270008c, 0x0);
    //GIC
    mmio_write_32 (0x32700108, 0x80000303);
    mmio_write_32 (0x3270010c, 0x0);

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
	for (int i = 0; i < 5; i++) {
		if (pdn)
			irq_mask = ~dist_ctx->gicd_isenabler[i];
		else
			irq_mask = IMR_MASK_ALL;

		mmio_write_32(IMX_GPC_BASE + gpc_imr_offset[last_core] + i * 4,
			      irq_mask);
	}

	/* enable the MU wakeup */
	if (imx_is_m4_enabled()) {
		val = mmio_read_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + 0x8);
		val &= ~(1 << 24);
		mmio_write_32(IMX_GPC_BASE + GPC_IMR1_CORE0_A53 + 0x8, val);
	}
}

static void imx_config_noc(uint32_t domain_id)
{
       int i;

       if (domain_id == HSIOMIX) {
              //enable HSIOMIX clock
              mmio_write_32 (0x32f10000, 0x2);
       }

       if (domain_id == HDMIMIX) {
              uint32_t hurry;

              /* reset HDMI AXI and APB clock */
              mmio_write_32(0x30388b00, 0x0);  //Disable HDMI APB
              mmio_write_32(0x30388b80, 0x0);  //Dosable HDMI AXI

              mmio_write_32(0x30388b00, 0x12010001);
              mmio_write_32(0x30388b80, 0x17000000);

              /* enable lcdif3 in hdmi_blk GPC/Reset */
              mmio_write_32(0x32fc0020, 0xffffffff);
              mmio_write_32(0x32fc0040, 0xFFFFFFFF);
              mmio_write_32(0x32fc0050, 0x7ffff87e);
              mmio_write_32(0x32fc0220, 0x22018);
              mmio_write_32(0x32fc0220, 0x22010);

              //set GPR to make lcdif read hurry level 0x7
              hurry = mmio_read_32 (0x32fc0200);
              hurry |= 0x00077000;
              mmio_write_32 (0x32fc0200, hurry);
       }

       if (domain_id == MEDIAMIX) {
              uint32_t hurry;

              /* handle mediamix special */
              mmio_write_32(0x32ec0000, 0x1FFFFFF);
              mmio_write_32(0x32ec0004, 0x1FFFFFF);
              mmio_write_32(0x32ec0008, 0x40030000);

              //set GPR to make lcdif read hurry level 0x7
              hurry = mmio_read_32 (0x32ec004c);
              hurry |= 0xfc00;
              mmio_write_32 (0x32ec004c, hurry);
              //set GPR to make isi write hurry level 0x7
              hurry = mmio_read_32 (0x32ec0050);
              hurry |= 0x1ff00000;
              mmio_write_32 (0x32ec0050, hurry);
       }

       /* set MIX NoC */
       for (i=0; i<sizeof(noc_setting)/sizeof(struct imx_noc_setting); i++) {
              if (noc_setting[i].domain_id == domain_id) {
                     udelay(50);
                     uint32_t offset = noc_setting[i].start;
                     while (offset <= noc_setting[i].end) {
                            //printf ("config noc, offset: 0x%x\n", offset);
                            mmio_write_32 (0x32700000 + offset + 0x8, noc_setting[i].prioriy);
                            mmio_write_32 (0x32700000 + offset + 0xc, noc_setting[i].mode);
                            mmio_write_32 (0x32700000 + offset + 0x18, noc_setting[i].socket_qos_en);
                            offset += 0x80;
                     }
              }
       }

       return;
}

static void imx_gpc_pm_domain_enable(uint32_t domain_id, uint32_t on)
{
	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];

	if (on) {
		pu_domain_status |= (1 << domain_id);

		/* clear the PGC bit */
		mmio_clrbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/* power up the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & pwr_domain->pwr_req);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* clear adb power down request */
			mmio_setbits_32(GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while (!(mmio_read_32(GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

              imx_config_noc (domain_id);

	} else {
		pu_domain_status &= ~(1 << domain_id);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* set adb power down request */
			mmio_clrbits_32(GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while ((mmio_read_32(GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & pwr_domain->pwr_req);
	}

	pwr_domain->init_on = false;
}

void imx_gpc_init(void)
{
	unsigned int val;
	int i;

	/* mask all the wakeup irq by default */
	for (i = 0; i < 5; i++) {
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
	mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (0x3 << 1));

	/*set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + GPC_PGC_CPU_0_1_MAPPING, 0x3fffff);

	/*
	 * Set the CORE & SCU power up timing:
	 * SW = 0x1, SW2ISO = 0x1;
	 * the CPU CORE and SCU power up timming counter
	 * is drived  by 32K OSC, each domain's power up
	 * latency is (SW + SW2ISO) / 32768
	 */
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(0) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(1) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(2) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(3) + 0x4, 0x81);
	mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR + 0x4, 0x81);
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
	/* clear the RBC */
	val &= ~(0x3f << SLPCR_RBC_SHIFT);
	/* set the STBY_COUNT to max, (512 * 30)us*/
	val &= ~(0x7 << SLPCR_STBY_COUNT_SHFT);
	val |= (0x7 << SLPCR_STBY_COUNT_SHFT);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

	/*
	 * USB PHY power up needs to make sure RESET bit in SRC is clear,
	 * otherwise, the PU power up bit in GPC will NOT self-cleared.
	 * only need to do it once.
	 */
	mmio_clrbits_32(IMX_SRC_BASE + 0x20, 0x1);
	mmio_clrbits_32(IMX_SRC_BASE + 0x24, 0x1);

	/* enable all power domain by default for bringup purpose */
	mmio_write_32(0x303844f0, 0x3);
	mmio_write_32(0x30384570, 0x3);

	for (i = 0; i < 102; i++)
		mmio_write_32(0x30384000 + i * 16, 0x3);

	for (i = 0; i < 19; i++)
		imx_gpc_pm_domain_enable(i, true);

	/* 0:mipi_phy1; 1: pcie_phy; 2: usb1_phy; 3: usb2_phy; 4: mlmix; 5: audiomix; 6: gpu2d;
	 * 7: gpu_share; 8: vpu_share; 9: gpu3d; 10: mediamix; 11:vpu_g1; 12:vpu_g2;
	 * 13: vpu_h1; 14: hdmimix; 15: hdmi_phy; 16: mipi_phy2; 17: hsiomix; 18: ddrmix; 19: mediamix_ispdwp
	 */
	mmio_write_32(IMX_GPC_BASE + PU_PGC_UP_TRG, (0x1 | (0x1 << 1) | (0x1 <<2) | (0x1 << 3) |
		(0x1 << 4) | (0x1 << 5) | (0x1 << 6) | (0x1 << 7) | (0x1 << 8) | (0x1 << 9) |
		(0x1 << 10) | (0x1 << 11) | (0x1 << 12) | (0x1 << 13) | (0x1 << 14) | (0x1 << 15) |
		(0x1 << 16) | (0x1 << 17) | (0x1 << 18) | (0x1 << 19)));

	/* handle mediamix special */
	mmio_write_32(0x32ec0000, 0x1FFFFFF);
	mmio_write_32(0x32ec0004, 0x1FFFFFF);
	mmio_write_32(0x32ec0008, 0x20000);

    /* config main NoC */
    //A53
    mmio_write_32 (0x32700008, 0x80000303);
    mmio_write_32 (0x3270000c, 0x0);
    //SUPERMIX
    mmio_write_32 (0x32700088, 0x80000303);
    mmio_write_32 (0x3270008c, 0x0);
    //GIC
    mmio_write_32 (0x32700108, 0x80000303);
    mmio_write_32 (0x3270010c, 0x0);
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
