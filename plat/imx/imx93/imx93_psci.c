/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <plat_imx8.h>

#define BLK_CTRL_S_BASE                0x444F0000
#define CA55_CPUWAIT           0x118
#define CA55_RVBADDR0_L                0x11c
#define CA55_RVBADDR0_H                0x120
#define HW_LP_HANDHSK		0x110
#define HW_LP_HANDHSK2		0x114
#define  HW_S401_RESET_REQ_MASK 0x130

#define IMX_SRC_BASE		0x44460000
#define IMX_GPC_BASE		0x44470000

/* SRC */
#define MIX_AUTHEN_CTRL 	0x4
#define LPM_SETTING_1		0x14
#define LPM_SETTING_2		0x18
#define A55C0_MEM		0x5c00

#define MEM_CTRL		0x4
#define MEM_LP_EN		BIT(2)
#define MEM_LP_RETENTION	BIT(1)

/* GPC */
#define A55C0_CMC_OFFSET	0x800
#define CM_MISC		0xc
#define IRQ_MUX		BIT(5)
#define SW_WAKEUP	BIT(6)

#define CM_MODE_CTRL	0x10
#define CM_IRQ_WAKEUP_MASK0	0x100
#define CM_SYS_SLEEP_CTRL	0x380

#define IMX_SRC_A55C0_OFFSET   0x2c00
#define SLICE_SW_CTRL          0x20
#define SLICE_SW_CTRL_PDN_SOFT BIT(31)

#define CM_MODE_RUN	0x0
#define CM_MODE_WAIT	0x1
#define CM_MODE_STOP	0x2
#define CM_MODE_SUSPEND	0x3

#define GPC_DOMAIN	0x10
#define GPC_DOMAIN_SEL	0x18
#define GPC_MASTER	0x1c
#define GPC_SYS_SLEEP	0x40
#define PMIC_CTRL	0x100
#define GPC_RCOSC_CTRL	0x200

#define GPC_GLOBAL_OFFSET	0x4000

#define BBNSM_BASE	U(0x44440000)
#define BBNSM_CTRL	U(0x8)
#define BBNSM_DP_EN	BIT(24)
#define BBNSM_TOSP	BIT(25)

#define LPM_DOMAINx(n)	(1 << ((n) * 4))

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

/* for GIC context save/restore if NIC lost power */
struct plat_gic_ctx imx_gicv3_ctx;
/* platfrom secure warm boot entry */
static uintptr_t secure_entrypoint;

static bool boot_stage = true;

void gpc_src_init(void)
{
	int i;

	/* config some of the GPC global config */
	/* use GPC assigned domain control */
	mmio_write_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_DOMAIN_SEL, 0x0);

	/*
	 * assigned A55 cluster to domain3, m33 to domain2, CORE0 & CORE1 to domain0/1
	 * domain0/1 only used for trigger LPM of themselves
	 */
	mmio_write_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_DOMAIN, 0x3102);

	for (i = 0; i < 3; i++) {
		/*
		 * disable the system sleep control trigger for A55 core0/core1 & cluster by default.
		 */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_SYS_SLEEP_CTRL, 0x0);

		/* TODO, config A55 core & cluster's wakeup source to GIC by default */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_MISC, IRQ_MUX);

		/* clear the cpu sleep hold */
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * i + CM_MISC, BIT(1));
	}
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

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

	if (boot_stage) {
		/* assert CPU core SW reset */
		mmio_clrbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + core_id * 0x400 + 0x24, BIT(2) | BIT(0));

		/* deassert CPU core SW reset */
		mmio_setbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + core_id * 0x400 + 0x24, BIT(2) | BIT(0));

		/* release the cpuwait to kick the cpu */
		mmio_clrbits_32(BLK_CTRL_S_BASE + CA55_CPUWAIT, BIT(core_id));

		boot_stage = false;
	} else {
		/* config the CMC MISC SW WAKEUP BIT to kick the cpu core */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + core_id * 0x800 + CM_MISC, SW_WAKEUP);
	}

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();

	/* below config is harmless either first time boot or hotplug */
	/* clear the CPU power mode */
	mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_RUN);
	/* clear the SW wakeup */
	mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + core_id * 0x800 + CM_MISC, SW_WAKEUP);
	/* switch to GIC wakeup source */
	mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MISC, IRQ_MUX);
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	int i;

	plat_gic_cpuif_disable();

	/* switch to GPC wakeup source */
	mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MISC, IRQ_MUX);

	/* mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core as we need to use SW_WAKEUP for hotplug purpose */
	for (i = 0; i < 8; i++)
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800  * core_id + CM_IRQ_WAKEUP_MASK0 + i * 4, 0xffffffff);

	/* config the target mode to suspend */
	mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_SUSPEND);

	/* SRC config */
	/* config the MEM LPM */
	mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * core_id + 0x4, MEM_LP_EN);

	/* LPM config to only ON in run mode to its domain */
	mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x14, 0x1 << (core_id * 4));

	/* white list config, only enable its domain? */
	mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x4, 0xffff0000, (1 << (core_id + 16)) | BIT(2));

	/* as this cpu core is hotpluged, so config its GPC_SYS_SLEEP FORCE_COREx_DISABLE  to 1'b1 */
	mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, 1 << (17 + core_id));
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t val;

       /* enable the HW LP handshake between S401 & A55 cluster */
       mmio_setbits_32(BLK_CTRL_S_BASE + HW_LP_HANDHSK, BIT(5));
 
	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint);

		plat_gic_cpuif_disable();
		/* switch to GIC wakeup source */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MISC, IRQ_MUX);

		/* config the target mode to WAIT */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_WAIT);

		/* ignore core's LPM trigger for system sleep, may need to be moved to system level */
		mmio_setbits_32(IMX_GPC_BASE + GPC_GLOBAL_OFFSET + GPC_SYS_SLEEP, 1 << (17 + core_id));

		/* MEM LPM */
		mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * core_id + 0x4, MEM_LP_EN);

		/* LPM config to only ON in run mode, LPM control only by core itself; domain2/3 */
		mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x14, 0x1 << (core_id * 4));
		/* config SRC to enable LPM control(HW flow) */
		mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * core_id + 0x4, 0xffff0000, (1 << (core_id + 16)) | BIT(2));
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config A55 cluster's wakeup source to GIC */
		mmio_setbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, IRQ_MUX);

		/* config the A55 cluster target mode to WAIT */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MODE_CTRL, CM_MODE_WAIT);

		/* SCU Snoop MEM must be power down, can NOT be put into retention */
		mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 2 + 0x4, MEM_LP_EN);
		/* L3 MEM can be put into retention or power down as needed */
		if (is_local_state_off(CLUSTER_PWR_STATE(target_state))) {
			/* ALL off mode */
			mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 3 + 0x4, MEM_LP_EN);
		} else {
			/* L3 retention */
			mmio_setbits_32(IMX_SRC_BASE + A55C0_MEM + 0x400 * 3 + 0x4, MEM_LP_EN | MEM_LP_RETENTION);
		}

		/* FIXME LPM config to only ON in RUN mode: domain1 only, set before whitelist config */
		mmio_write_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 2 + 0x14, 0x1 << 12);
		/* config the SRC for cluster slice LPM control */
		mmio_clrsetbits_32(IMX_SRC_BASE + IMX_SRC_A55C0_OFFSET + 0x400 * 2 + 0x4, 0xffff0000, (1 << 19) | BIT(2));

		/* clear the cluster sleep hold */
		mmio_clrbits_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MISC, BIT(1));

		/* config DSU for cluster power down */
		val = read_clusterpwrdn();
		val &= ~DSU_CLUSTER_PWR_MASK;
		val |= DSU_CLUSTER_PWR_OFF;
		/* L3 retention */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			val |= BIT(1);
		}
			
		write_clusterpwrdn(val);
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t val;

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* clear DSU cluster power down if cluster power off is aborted by wakeup */
		val = read_clusterpwrdn();
		val &= ~(DSU_CLUSTER_PWR_MASK | BIT(1));
		val |= DSU_CLUSTER_PWR_ON;
		write_clusterpwrdn(val);

		/* set the cluster's target mode to RUN */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * 2 + CM_MODE_CTRL, CM_MODE_RUN);
	}
	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* set A55 CORE's power mode to RUN */
		mmio_write_32(IMX_GPC_BASE + A55C0_CMC_OFFSET + 0x800 * core_id + CM_MODE_CTRL, CM_MODE_RUN);
		plat_gic_cpuif_enable();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1)
		wfi();
}

#define IMX_WDOG3_BASE	U(0x42490000)

void __dead2 imx_system_reset(void)
{
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0xd928c520);
	while ((mmio_read_32(IMX_WDOG3_BASE) & 0x800) == 0)
		;
	mmio_write_32(IMX_WDOG3_BASE + 0x8, 0x10);
	mmio_write_32(IMX_WDOG3_BASE, 0x21e3);

	while (true)
		;
}

void __dead2 imx_system_off(void)
{
	mmio_setbits_32(BBNSM_BASE + BBNSM_CTRL, BBNSM_DP_EN | BBNSM_TOSP);

	while (1)
		;
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
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
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

	gpc_src_init();

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
