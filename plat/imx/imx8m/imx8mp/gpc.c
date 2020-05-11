/*
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/arm/tzc380.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/smccc.h>
#include <platform_def.h>
#include <services/std_svc.h>

#include <gpc.h>
#include <imx_sip_svc.h>
#include <plat_imx8.h>

#define FSL_SIP_CONFIG_GPC_MASK		0x00
#define FSL_SIP_CONFIG_GPC_UNMASK	0x01
#define FSL_SIP_CONFIG_GPC_SET_WAKE	0x02
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x03
#define FSL_SIP_CONFIG_GPC_SET_AFF	0x04
#define FSL_SIP_CONFIG_GPC_CORE_WAKE	0x05

#define IMR_NUM		U(5)
#define CCGR(x)		(0x4000 + (x) * 16)

struct imx_noc_setting {
       uint32_t domain_id;
       uint32_t start;
       uint32_t end;
       uint32_t prioriy;
       uint32_t mode;
       uint32_t socket_qos_en;
};

enum clk_type {
	CCM_ROOT_SLICE,
	CCM_CCGR,
};

struct clk_setting {
	uint32_t offset;
	uint32_t val;
	enum clk_type type;
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
	{ MLMIX, 0x180, 0x180, 0x80000303, 0x0, 0x0},
	{ AUDIOMIX, 0x200, 0x200, 0x80000303, 0x0, 0x0},
	{ AUDIOMIX, 0x280, 0x480, 0x80000404, 0x0, 0x0},
	{ GPUMIX, 0x500, 0x580, 0x80000303, 0x0, 0x0},
	{ HDMIMIX, 0x600, 0x680, 0x80000202, 0x0, 0x1},
	{ HDMIMIX, 0x700, 0x700, 0x80000505, 0x0, 0x0},
	{ HSIOMIX, 0x780, 0x900, 0x80000303, 0x0, 0x0},
	{ MEDIAMIX, 0x980, 0xb80, 0x80000202, 0x0, 0x1},
	{ MEDIAMIX_ISPDWP, 0xc00, 0xd00, 0x80000505, 0x0, 0x0},
	//can't access VPU NoC if only power up VPUMIX,
	//need to power up both NoC and IP
	{ VPU_G1, 0xd80, 0xd80, 0x80000303, 0x0, 0x0},
	{ VPU_G2, 0xe00, 0xe00, 0x80000303, 0x0, 0x0},
	{ VPU_H1, 0xe80, 0xe80, 0x80000303, 0x0, 0x0}
};

static struct clk_setting hsiomix_clk[] = {
	{ 0x8380, 0x0, CCM_ROOT_SLICE },
	{ 0x44d0, 0x0, CCM_CCGR },
	{ 0x45c0, 0x0, CCM_CCGR },
};

static uint32_t gpc_saved_imrs[20];
static uint32_t gpc_wake_irqs[5];
static uint32_t gpc_imr_offset[] = {
	IMX_GPC_BASE + IMR1_CORE0_A53,
	IMX_GPC_BASE + IMR1_CORE1_A53,
	IMX_GPC_BASE + IMR1_CORE2_A53,
	IMX_GPC_BASE + IMR1_CORE3_A53,
	IMX_GPC_BASE + IMR1_CORE0_M4,
};

static unsigned int pu_domain_status;
spinlock_t gpc_imr_lock[4];

static void gpc_imr_core_spin_lock(unsigned int core_id)
{
	spin_lock(&gpc_imr_lock[core_id]);
}

static void gpc_imr_core_spin_unlock(unsigned int core_id)
{
	spin_unlock(&gpc_imr_lock[core_id]);
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

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int imr, core;

	if (pdn)
		for (imr = 0; imr < 5; imr++)
			for (core = 0; core < 4; core++)
				gpc_save_imr_lpm(core, imr);
	else
		for (imr = 0; imr < 5; imr++)
			for (core = 0; core < 4; core++)
				gpc_restore_imr_lpm(core, imr);

	/* enable the MU wakeup */
	if (imx_m4_lpa_active())
		mmio_clrbits_32(gpc_imr_offset[last_core] + 0x8, BIT(24));
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

static void imx_config_noc(uint32_t domain_id)
{
	int i;
	uint32_t hurry;

	if (domain_id == HDMIMIX) {
		mmio_write_32(0x32fc0220, 0x22018);
		mmio_write_32(0x32fc0220, 0x22010);

		//set GPR to make lcdif read hurry level 0x7
		hurry = mmio_read_32 (0x32fc0200);
		hurry |= 0x00077000;
		mmio_write_32 (0x32fc0200, hurry);
	}

	if (domain_id == MEDIAMIX) {
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
				mmio_write_32 (0x32700000 + offset + 0x8, noc_setting[i].prioriy);
				mmio_write_32 (0x32700000 + offset + 0xc, noc_setting[i].mode);
				mmio_write_32 (0x32700000 + offset + 0x18, noc_setting[i].socket_qos_en);
				offset += 0x80;
			}
		}
	}
}

void imx_aips5_init(void)
{
	/* config the AIPSTZ5, since it depends on power up audio mix */
	mmio_write_32(0x30df0000, 0x77777777);
	mmio_write_32(0x30df0004, 0x77777777);
	mmio_write_32(0x30df0040, 0x0);
	mmio_write_32(0x30df0044, 0x0);
	mmio_write_32(0x30df0048, 0x0);
	mmio_write_32(0x30df004c, 0x0);
	mmio_write_32(0x30df0050, 0x0);
}

void imx_gpc_pm_domain_enable(uint32_t domain_id, bool on)
{
	struct imx_pwr_domain *pwr_domain = &pu_domains[domain_id];
	int i;

	if (domain_id == HSIOMIX) {
		for (i = 0; i < ARRAY_SIZE(hsiomix_clk); i++) {
			hsiomix_clk[i].val = mmio_read_32(IMX_CCM_BASE + hsiomix_clk[i].offset);
				mmio_setbits_32(IMX_CCM_BASE + hsiomix_clk[i].offset,
					hsiomix_clk[i].type == CCM_ROOT_SLICE ? BIT(28) : 0x3);
			}
	}

	if (on) {
		if (pwr_domain->need_sync)
			pu_domain_status |= (1 << domain_id);

		if (domain_id == HDMIMIX) {
			/* assert the reset */
			mmio_write_32(0x32fc0020, 0x0);
			/* enable all th function clock */
			mmio_write_32(0x32fc0040, 0xFFFFFFFF);
			mmio_write_32(0x32fc0050, 0x7ffff87e);
		}

		/* clear the PGC bit */
		mmio_clrbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/* power up the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & pwr_domain->pwr_req);

		if (domain_id == HDMIMIX) {
			/* wait for memory repair done for HDMIMIX */
			while(!(mmio_read_32(IMX_SRC_BASE + 0x94) & BIT(8)))
				;
			/* disable all the function clock */
			mmio_write_32(0x32fc0040, 0x0);
			mmio_write_32(0x32fc0050, 0x0);
			/* deassert the reset */
			mmio_write_32(0x32fc0020, 0xffffffff);
			/* enable all the clock again */
			mmio_write_32(0x32fc0040, 0xFFFFFFFF);
			mmio_write_32(0x32fc0050, 0x7ffff87e);
		}

		if (domain_id == HSIOMIX)
			/* enable HSIOMIX clock */
			mmio_write_32 (0x32f10000, 0x2);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* clear adb power down request */
			mmio_setbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while (!(mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		imx_config_noc(domain_id);

		/* AIPS5 config is lost when audiomix is off, so need to re-init it */
		if (domain_id == AUDIOMIX)
			imx_aips5_init();
	} else {
		if (imx_m4_lpa_active() && domain_id == AUDIOMIX)
			return;
 
		/* keep the USB PHY always on currently */
		if (domain_id == USB1_PHY || domain_id == USB2_PHY)
			return;

		if (pwr_domain->need_sync)
			pu_domain_status &= ~(1 << domain_id);

		/* handle the ADB400 sync */
		if (!pwr_domain->init_on && pwr_domain->need_sync) {
			/* set adb power down request */
			mmio_clrbits_32(IMX_GPC_BASE + GPC_PU_PWRHSK, pwr_domain->adb400_sync);

			/* wait for adb power request ack */
			while ((mmio_read_32(IMX_GPC_BASE + GPC_PU_PWRHSK) & pwr_domain->adb400_ack))
				;
		}

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + pwr_domain->pgc_offset, 0x1);

		/*
		 * leave the G1, G2, H1 power domain on until VPUMIX power off,
		 * otherwise system will hang due to VPUMIX ACK
		 */
		if (domain_id == VPU_H1 || domain_id == VPU_G1 || domain_id == VPU_G2)
			return;

		if (domain_id == VPUMIX)
			mmio_write_32(IMX_GPC_BASE + PU_PGC_DN_TRG, VPU_G1_PWR_REQ |
				 VPU_G2_PWR_REQ | VPU_H1_PWR_REQ);

		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, pwr_domain->pwr_req);

		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & pwr_domain->pwr_req);

		if (domain_id == HDMIMIX) {
			/* disable all the clocks of HDMIMIX */
			mmio_write_32(0x32fc0040, 0x0);
			mmio_write_32(0x32fc0050, 0x0);
		}
	}

	if (domain_id == HSIOMIX) {
		for (i = 0; i < ARRAY_SIZE(hsiomix_clk); i++)
			mmio_write_32(IMX_CCM_BASE + hsiomix_clk[i].offset, hsiomix_clk[i].val);
	}

	pwr_domain->init_on = false;
}

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

void imx_noc_wrapper_pre_suspend(unsigned int proc_num)
{
	/* enable MASTER1 & MASTER2 power down in A53 LPM mode */
	mmio_clrbits_32(IMX_GPC_BASE + LPCR_A53_BSC, MASTER1_LPM_HSK | MASTER2_LPM_HSK);
	mmio_setbits_32(IMX_GPC_BASE+ MST_CPU_MAPPING, MASTER1_MAPPING | MASTER2_MAPPING);

	/* noc can only be power down when all the pu domain is off */
	if (!pu_domain_status) {
		/* enable noc power down */
		imx_noc_slot_config(true);
	}
	/*
	 * gic redistributor context save must be called when
	 * the GIC CPU interface is disabled and before distributor save.
	 */
	plat_gic_save(proc_num, &imx_gicv3_ctx);
}

void imx_noc_wrapper_post_resume(unsigned int proc_num)
{
	/* disable MASTER1 & MASTER2 power down in A53 LPM mode */
	mmio_setbits_32(IMX_GPC_BASE + LPCR_A53_BSC, MASTER1_LPM_HSK | MASTER2_LPM_HSK);
	mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, MASTER1_MAPPING | MASTER2_MAPPING);

	/* noc can only be power down when all the pu domain is off */
	if (!pu_domain_status) {
		/* re-init the tz380 if resume from noc power down */
		imx8mm_tz380_init();
		/* disable noc power down */
		imx_noc_slot_config(false);

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

	/* restore gic context */
	plat_gic_restore(proc_num, &imx_gicv3_ctx);
}

uint32_t pd_init_on[] = {
	/* hsio ss */
	HSIOMIX,
	USB1_PHY,
	USB2_PHY,
};

void imx_gpc_init(void)
{
	unsigned int val;
	int i;

	/* mask all the wakeup irq by default */
	for (i = 0; i < IMR_NUM; i++) {
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE1_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE2_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE3_A53 + i * 4, ~0x0);
		mmio_write_32(IMX_GPC_BASE + IMR1_CORE0_M4 + i * 4, ~0x0);
	}

	/* Due to the hardware design requirement, need to make
	 * sure GPR interrupt(#32) is unmasked during RUN mode to
	 * avoid entering DSM mode by mistake.
	 */
	for (i = 0; i < 4; i++)
		mmio_write_32(gpc_imr_offset[i], ~0x1);

	/* leave the IOMUX_GPC bit 12 on for core wakeup */
	mmio_setbits_32(IMX_IOMUX_GPR_BASE + 0x4, 1 << 12);


	val = mmio_read_32(IMX_GPC_BASE + LPCR_A53_BSC);
	/* use GIC wake_request to wakeup C0~C3 from LPM */
	val |= 0x40000000;
	/* clear the MASTER0 LPM handshake */
	val &= ~(1 << 6);
	mmio_write_32(IMX_GPC_BASE + LPCR_A53_BSC, val);

	/* clear MASTER1 & MASTER2 mapping in CPU0(A53) */
	mmio_clrbits_32(IMX_GPC_BASE + MST_CPU_MAPPING, (MASTER1_MAPPING |
		MASTER2_MAPPING));

	/* set all mix/PU in A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_CPU_0_1_MAPPING, 0x3fffff);

	/*
	 * Set the CORE & SCU power up timing:
	 * SW = 0x1, SW2ISO = 0x1;
	 * the CPU CORE and SCU power up timming counter
	 * is drived  by 32K OSC, each domain's power up
	 * latency is (SW + SW2ISO) / 32768
	 */
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(0) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(1) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(2) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + COREx_PGC_PCR(3) + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + PLAT_PGC_PCR + 0x4, 0x401);
	mmio_write_32(IMX_GPC_BASE + PGC_SCU_TIMING,
		      (0x59 << 10) | 0x5B | (0x2 << 20));

	/* set DUMMY PDN/PUP ACK by default for A53 domain */
	mmio_write_32(IMX_GPC_BASE + PGC_ACK_SEL_A53,
		      A53_DUMMY_PUP_ACK | A53_DUMMY_PDN_ACK);

	/* clear DSM by default */
	val = mmio_read_32(IMX_GPC_BASE + SLPCR);
	val &= ~SLPCR_EN_DSM;
	/* enable the fast wakeup wait/stop mode */
	val |= SLPCR_A53_FASTWUP_WAIT_MODE;
	val |= SLPCR_A53_FASTWUP_STOP_MODE;
	/* clear the RBC */
	val &= ~(0x3f << SLPCR_RBC_COUNT_SHIFT);
	/* set the STBY_COUNT to 0x5, (128 * 30)us */
	val &= ~(0x7 << SLPCR_STBY_COUNT_SHFT);
	val |= (0x7 << SLPCR_STBY_COUNT_SHFT);
	mmio_write_32(IMX_GPC_BASE + SLPCR, val);

	/*
	 * USB PHY power up needs to make sure RESET bit in SRC is clear,
	 * otherwise, the PU power up bit in GPC will NOT self-cleared.
	 * only need to do it once.
	 */
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG1PHY_SCR, 0x1);
	mmio_clrbits_32(IMX_SRC_BASE + SRC_OTG2PHY_SCR, 0x1);

	/* enable all power domain by default for bringup purpose */
	mmio_write_32(0x303844f0, 0x3);
	mmio_write_32(0x30384570, 0x3);

	for (i = 0; i < 102; i++)
		mmio_write_32(0x30384000 + i * 16, 0x3);

	for (i = 0; i < ARRAY_SIZE(pd_init_on); i++)
		imx_gpc_pm_domain_enable(pd_init_on[i], true);

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
	case FSL_SIP_CONFIG_GPC_SET_AFF:
		imx_gpc_set_affinity(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}
