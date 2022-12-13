/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <drivers/arm/gicv3.h>
#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <plat_imx8.h>
#include <pwr_ctrl.h>

#define ARM_PLL		U(0x44481000)
#define SYS_PLL		U(0x44481100)
#define SYS_PLL_DFS_0	U(SYS_PLL + 0x70)
#define SYS_PLL_DFS_1	U(SYS_PLL + 0x90)
#define SYS_PLL_DFS_2	U(SYS_PLL + 0xb0)
#define OSCPLL_CHAN(x)	(0x44455000 + (x) * 0x40)
#define OSCPLL_NUM	U(12)
#define OSCPLL_LPM0	U(0x10)
#define OSCPLL_LPM_DOMAIN_MODE(x, d) ((x) << (d * 4))
#define OSCPLL_LPM_AUTH	U(0x30)
#define PLL_HW_CTRL_EN	BIT(16)
#define CCM_ROOT_SLICE(x)	(0x44450000 + (x) * 0x80)
#define ROOT_MUX_MASK	GENMASK_32(9, 8)

#define S400_MU_RSR	(S400_MU_BASE + 0x12c)
#define S400_MU_TRx(i)	(S400_MU_BASE + 0x200 + (i) * 4)
#define S400_MU_RRx(i)	(S400_MU_BASE + 0x280 + (i) * 4)
#define ELE_POWER_DOWN_REQ	U(0x17d10306)

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define GPIO_CTRL_REG_NUM		U(8)
#define GPIO_PIN_MAX_NUM		U(32)
#define GPIO_CTX(addr, num)	\
	{.base = (addr), .pin_num = (num), }

enum ccm_clock_root {
	M33_ROOT = 3,
	WAKEUP_AXI_ROOT = 7,
	HSIO_CLK_ROOT = 61,
	NIC_CLK_ROOT = 65,
};

extern void dram_enter_retention(void);
extern void dram_exit_retention(void);
extern void s401_request_pwrdown(void);
extern void trdc_n_reinit(void);
extern void trdc_w_reinit(void);

struct gpio_ctx {
	/* gpio base */
	uintptr_t base;
	/* port control */
	uint32_t port_ctrl[GPIO_CTRL_REG_NUM];
	/* GPIO ICR, Max 32 */
	uint32_t pin_num;
	uint32_t gpio_icr[GPIO_PIN_MAX_NUM];
};

/* for GIC context save/restore if NIC lost power */
struct plat_gic_ctx imx_gicv3_ctx;
/* platform secure warm boot entry */
static uintptr_t secure_entrypoint;

static bool boot_stage = true;
static bool no_wakeup_enabled = true;

static uint32_t gpio_ctrl_offset[GPIO_CTRL_REG_NUM] = { 0xc, 0x10, 0x14, 0x18, 0x1c, 0x40, 0x54, 0x58 };
static struct gpio_ctx wakeupmix_gpio_ctx[3] = {
	GPIO_CTX(GPIO2_BASE | BIT(28), 30),
	GPIO_CTX(GPIO3_BASE | BIT(28), 32),
	GPIO_CTX(GPIO4_BASE | BIT(28), 28),
};

static uint32_t clock_root[4];
/*
 * Empty implementation of these hooks avoid setting the GICR_WAKER.Sleep bit
 * on ARM GICv3 implementations without LPI support.
 */
void arm_gicv3_distif_pre_save(unsigned int rdist_proc_num)
{}

void arm_gicv3_distif_post_restore(unsigned int rdist_proc_num)
{}

void pll_pwr_down(bool enter)
{
	if(enter) {
		/* Switch the ARM/SYS PLLs to hw_ctrl(bit 16) in PLL CTRL reg */
		mmio_setbits_32(ARM_PLL, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_setbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		/* LPM setting for PLL */
		for (unsigned int i = 1; i <= OSCPLL_NUM; i++) {
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM0, OSCPLL_LPM_DOMAIN_MODE(0x1, 0x3));
			mmio_setbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	} else {
		mmio_clrbits_32(ARM_PLL, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_0, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_1, PLL_HW_CTRL_EN);
		mmio_clrbits_32(SYS_PLL_DFS_2, PLL_HW_CTRL_EN);

		for (unsigned int i = 1; i <= OSCPLL_NUM; i++) {
			mmio_clrbits_32(OSCPLL_CHAN(i) + OSCPLL_LPM_AUTH, BIT(2));
		}
	}
}

void s401_request_pwrdown(void)
{
	uint32_t msg, resp;

	mmio_write_32(S400_MU_TRx(0), ELE_POWER_DOWN_REQ);
	mmio_write_32(S400_MU_TRx(1), 0x4000);
	mmio_write_32(S400_MU_TRx(2), BL31_BASE);

	do {
		resp = mmio_read_32(S400_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(S400_MU_RRx(0));
	resp = mmio_read_32(S400_MU_RRx(1));

	VERBOSE("resp %x; %x", msg, resp);
}

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int i;
	uint32_t irq_mask;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn) {
		/*
		 * If NICMIX power down, need to switch the primary core & cluster wakeup
		 * source to GPC as GIC will be power down.
		 */
		gpc_select_wakeup_raw_irq(CPU_A55C0);
		gpc_select_wakeup_raw_irq(CPU_A55_PLAT);
	} else {
		/* switch to GIC wakeup source for last_core and cluster */
		gpc_select_wakeup_gic(CPU_A55C0);
		gpc_select_wakeup_gic(CPU_A55_PLAT);
	}

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (i = 0; i < IMR_NUM; i++) {
		if (pdn) {
			/* set the wakeup irq base GIC */
			irq_mask = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		} else {
			irq_mask = 0xFFFFFFFF;
		}

		/* set the mask into core & cluster GPC IMR */
		gpc_set_irq_mask(CPU_A55C0, i, irq_mask);
		gpc_set_irq_mask(CPU_A55_PLAT, i, irq_mask);
	}
}

void nicmix_qos_init(void)
{
	mmio_write_32(0x49000010, 0x44);
	mmio_write_32(0x49000014, 0x330033);
	mmio_write_32(0x49000018, 0x330033);
	mmio_write_32(0x4900001c, 0x330033);
	mmio_write_32(0x49000020, 0x440044);
}

void wakeupmix_qos_init(void)
{
	mmio_write_32(0x42846100, 0x4);
	mmio_write_32(0x42846104, 0x4);

	mmio_write_32(0x42847100, 0x4);
	mmio_write_32(0x42847104, 0x4);

	mmio_write_32(0x42842100, 0x4);
	mmio_write_32(0x42842104, 0x4);

	mmio_write_32(0x42843100, 0x4);
	mmio_write_32(0x42843104, 0x4);

	mmio_write_32(0x42845100, 0x4);
	mmio_write_32(0x42845104, 0x4);

	mmio_write_32(0x43947100, 0x4);
	mmio_write_32(0x43947104, 0x4);

	mmio_write_32(0x43945100, 0x4);
	mmio_write_32(0x43945104, 0x4);
}

void nicmix_pwr_down(unsigned int core_id)
{
	/* enable the handshake between sentinel & NICMIX */
	mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(11));

	/* swith wakeup axi, hsio & nic to 24M when NICMIX power down */
	clock_root[1] = mmio_read_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT));
	clock_root[2] = mmio_read_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT));
	clock_root[3] = mmio_read_32(CCM_ROOT_SLICE(NIC_CLK_ROOT));
	mmio_clrbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), ROOT_MUX_MASK);
	mmio_clrbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), ROOT_MUX_MASK);

	/* NICMIX */
	src_mix_set_lpm(SRC_NIC, 0x3, CM_MODE_WAIT);
	src_authen_config(SRC_NIC, 0x8, 0x1);

	/* NICMIX OCRAM memory retention */
	src_mem_lpm_en(SRC_NIC_MEM, MEM_OFF);
	/* OCRAM MEM */
	src_mem_lpm_en(SRC_NIC_OCRAM, MEM_RETN);

	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, true);
}

void nicmix_pwr_up(unsigned int core_id)
{
	mmio_setbits_32(CCM_ROOT_SLICE(WAKEUP_AXI_ROOT), clock_root[1] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(HSIO_CLK_ROOT), clock_root[2] & ROOT_MUX_MASK);
	mmio_setbits_32(CCM_ROOT_SLICE(NIC_CLK_ROOT), clock_root[3] & ROOT_MUX_MASK);

	/* keep nicmix on when exit from system suspend */
	src_mix_set_lpm(SRC_NIC, 0x3, CM_MODE_SUSPEND);
	trdc_n_reinit();
	nicmix_qos_init();
	plat_gic_restore(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, false);
}

extern int trdc_mbc_blk_config(unsigned long trdc_reg, uint32_t mbc_x,
	 uint32_t dom_x, uint32_t mem_x, uint32_t blk_x,
	 bool sec_access, uint32_t glbac_id);

void set_gpio_secure(bool ns)
{
	/* GPIO2-4 */
	/* GPIO2 TRDC_W MBC0 MEM 1 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 1, 0x0, ns, 0);
	/* GPIO3 TRDC_W MBC0 MEM 2 */
	trdc_mbc_blk_config(0x42460000, 0, 3, 2, 0x0, ns, 0);
	/* GPIO4 TRDC_W MBC1 MEM 3 */
	trdc_mbc_blk_config(0x42460000, 1, 3, 3, 0x0, ns, 0);
}

void gpio_save(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	/* Enable GPIO secure access */
	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		/* save the port control setting */
		for (j = 0; j < GPIO_CTRL_REG_NUM; j++) {
			if (j < 4) {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
				/*
				 * clear the permission setting to read the GPIO non-secure world setting.
				*/
				mmio_write_32(ctx->base + gpio_ctrl_offset[j], 0x0);
			} else {
				ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
			}
		}
		/* save the gpio icr setting */
		for (j = 0; j < ctx->pin_num; j++) {
			ctx->gpio_icr[j] = mmio_read_32(ctx->base + 0x80 + j * 4);

			/* check if any gpio irq is enabled as wakeup source */
			if (ctx->gpio_icr[j]) {
				no_wakeup_enabled = false;
			}
		}

		/* permission config retore back */
		for (j = 0; j < 4; j++) {
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}
	/* Disable GPIO secure access */
	set_gpio_secure(false);
}

void gpio_restore(struct gpio_ctx *ctx, int port_num)
{
	unsigned int i, j;

	set_gpio_secure(true);

	for (i = 0; i < port_num; i++) {
		for (j = 0; j < ctx->pin_num; j++)
			mmio_write_32(ctx->base + 0x80 + j * 4, ctx->gpio_icr[j]);

		for (j = 4; j < GPIO_CTRL_REG_NUM; j++)
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);

		/* permission config retore last */
		for (j = 0; j < 4; j++) {
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
		}

		ctx++;
	}

	set_gpio_secure(false);
}

void wakeupmix_pwr_down(void)
{
	gpio_save(wakeupmix_gpio_ctx, 3);
	if (no_wakeup_enabled) {
		/* m33 root need to switch to 24M OSC when wakeupmix power down */
		clock_root[0] = mmio_read_32(CCM_ROOT_SLICE(M33_ROOT));
		mmio_clrbits_32(CCM_ROOT_SLICE(M33_ROOT), ROOT_MUX_MASK);

		/* wakeup mix controlled by A55 cluster power down: domain3 only */
		src_mix_set_lpm(SRC_WKUP, 0x3, CM_MODE_WAIT);
		src_authen_config(SRC_WKUP, 0x8, 0x1);
		/* wakeupmix mem off */
		src_mem_lpm_en(SRC_WKUP_MEM, MEM_OFF);
		/* enable the handshake between sentinel & wakeupmix */
		mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(9));
	}
}

void wakeupmix_pwr_up(void)
{
	if (no_wakeup_enabled) {
		mmio_setbits_32(CCM_ROOT_SLICE(M33_ROOT), clock_root[0] & ROOT_MUX_MASK);
		/* keep wakeupmix on when exit from system suspend */
		src_mix_set_lpm(SRC_WKUP, 0x3, CM_MODE_SUSPEND);
		trdc_w_reinit();
		wakeupmix_qos_init();
		gpio_restore(wakeupmix_gpio_ctx, 3);
	}
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(unsigned int core_id, uint64_t boot_entry)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	mmio_write_32(BLK_CTRL_S_BASE + CA55_RVBADDR0_L + core_id * 8, boot_entry >> 2);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;

	core_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_set_cpu_boot_entry(core_id, secure_entrypoint);

	/*
	 * When the core is first time boot up, the core is already ON after SoC POR,
	 * So 'SW_WAKEUP' can not work, so need to toggle core's reset then release
	 * the core from cpu_wait.
	 */
	if (boot_stage) {
		/* assert CPU core SW reset */
		mmio_clrbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* deassert CPU core SW reset */
		mmio_setbits_32(SRC_SLICE(SRC_A55C0 + core_id) + 0x24, BIT(2) | BIT(0));
		/* release the cpuwait to kick the cpu */
		mmio_clrbits_32(BLK_CTRL_S_BASE + CA55_CPUWAIT, BIT(core_id));
	} else {
		/* assert the CMC MISC SW WAKEUP BIT to kick the offline core */
		gpc_assert_sw_wakeup(CPU_A55C0 + core_id);
	}

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();

	/* below config is ok both for boot & hotplug */
	/* clear the CPU power mode */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
	/* clear the SW wakeup */
	gpc_deassert_sw_wakeup(CPU_A55C0 + core_id);
	/* switch to GIC wakeup source */
	gpc_select_wakeup_gic(CPU_A55C0 + core_id);

	if (boot_stage) {
		/* SRC config */
		/* config the MEM LPM */
		src_mem_lpm_en(SRC_A55P0_MEM + core_id, MEM_OFF);
		/* LPM config to only ON in run mode to its domain */
		src_mix_set_lpm(SRC_A55C0 + core_id, core_id, CM_MODE_WAIT);
		/* Set CNT_MODE =0 to reduce unnecessary latency */
		src_ack_cnt_mode(SRC_A55C0 + core_id, 0x0);
		/* white list config, only enable its own domain */
		src_authen_config(SRC_A55C0 + core_id, 1 << core_id, 0x1);

		boot_stage = false;
	}
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int i;

	plat_gic_cpuif_disable();
	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * as we need to use SW_WAKEUP for hotplug purpose
	 */
	for (i = 0U; i < IMR_NUM; i++) {
		gpc_set_irq_mask(CPU_A55C0 + core_id, i, 0xffffffff);
	}
	/* switch to GPC wakeup source */
	gpc_select_wakeup_raw_irq(CPU_A55C0 + core_id);
	/* config the target mode to suspend */
	gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_set_cpu_boot_entry(core_id, secure_entrypoint);
		/* config the target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_WAIT);
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config the A55 cluster target mode to WAIT */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_WAIT);

		/* config DSU for cluster power down with L3 MEM RET */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
		}
	}

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/*
		 * for the A55 cluster, the cache disable/flushing is controlled by HW,
		 * so flush cache explictly before put DDR into retention to make sure
		 * no cache maintenance to DDR memory happens afte DDR retention.
		 */
		dcsw_op_all(DCCISW);
		dram_enter_retention();

		/*
		 * if NICMIX or WAKEUPMIX power down, the TRDC_N/W config will lost,
		 * so need to request Sentinel to set the correct permission at the early
		 * begining.
		 */
		s401_request_pwrdown();

		nicmix_pwr_down(core_id);
		wakeupmix_pwr_down();

		/* config the A55 cluster target mode to SUSPEND */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_SUSPEND);

		/* Enable system suspend when A55 cluster is in SUSPEND MODE */
		gpc_set_cpu_ss_mode(CPU_A55_PLAT, SS_SUSPEND);

		/*
		 * FIXME: Only use A55 cluster to trigger system sleep, force M33 into system sleep
		 * this should be removed after M33 low power suspport is ready
		 */
		gpc_force_cpu_suspend(CPU_M33);
		/* put OSC into power down */
		gpc_rosc_off(true);
		/* put PMIC into standby mode */
		gpc_pmic_stby_en(true);

		/* power down PLL */
		pll_pwr_down(true);
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* system level */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		/* Disable system suspend when A55 cluster is in SUSPEND MODE */
		gpc_set_cpu_ss_mode(CPU_A55_PLAT, 0x0);
		/* FIXME: Only use A55 cluster to trigger system sleep, force CM33 into system sleep  */
		gpc_unforce_cpu_suspend(CPU_M33);
		/* Disable PMIC standby */
		gpc_pmic_stby_en(false);
		/* Disable OSC power down */
		gpc_rosc_off(false);
		/* power down PLL */
		pll_pwr_down(false);

		nicmix_pwr_up(core_id);
		wakeupmix_pwr_up();
		dram_exit_retention();
	}

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* set the cluster's target mode to RUN */
		gpc_set_cpu_mode(CPU_A55_PLAT, CM_MODE_RUN);
	}

	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* set A55 CORE's power mode to RUN */
		gpc_set_cpu_mode(CPU_A55C0 + core_id, CM_MODE_RUN);
		plat_gic_cpuif_enable();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}

	SYSTEM_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
}

void __dead2 imx_system_reset(void)
{
	mmio_write_32(WDOG3_BASE + WDOG_CNT, 0xd928c520);
	while ((mmio_read_32(WDOG3_BASE + WDOG_CS) & WDOG_CS_ULK) == 0U) {
		;
	}

	mmio_write_32(WDOG3_BASE + WDOG_TOVAL, 0x10);
	mmio_write_32(WDOG3_BASE + WDOG_CS, 0x21e3);

	while (1) {
		wfi();
	}
}

void __dead2 imx_system_off(void)
{
	mmio_setbits_32(BBNSM_BASE + BBNSM_CTRL, BBNSM_DP_EN | BBNSM_TOSP);

	while (1) {
		wfi();
	}
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_suspend = imx_pwr_domain_suspend,
	.pwr_domain_suspend_finish = imx_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, sec_entrypoint);

	pwr_sys_init();
	nicmix_qos_init();
	wakeupmix_qos_init();

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
