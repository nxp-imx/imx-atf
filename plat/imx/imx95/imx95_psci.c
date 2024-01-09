/*
 * Copyright 2023 NXP
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

#include <drivers/arm/gicv3.h>
#include "../drivers/arm/gic/v3/gicv3_private.h"

#include <drivers/arm/css/scmi.h>

#include <plat_imx8.h>
#include <scmi_imx9.h>

#define IMR_NUM		U(12)
#define IMX95_A55P_IDX		6

#define IMX9_SCMI_CPU_A55C0				2
#define IMX9_SCMI_CPU_A55C1				3
#define IMX9_SCMI_CPU_A55C2				4
#define IMX9_SCMI_CPU_A55C3				5
#define IMX9_SCMI_CPU_A55C4				6
#define IMX9_SCMI_CPU_A55C5				7
#define IMX9_SCMI_CPU_A55P				8

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define SCMI_PWR_MIX_SLICE_IDX_ANA           0U
#define SCMI_PWR_MIX_SLICE_IDX_AON           1U
#define SCMI_PWR_MIX_SLICE_IDX_BBSM          2U
#define SCMI_PWR_MIX_SLICE_IDX_CAMERA        3U
#define SCMI_PWR_MIX_SLICE_IDX_CCMSRCGPC     4U
#define SCMI_PWR_MIX_SLICE_IDX_A55C0         5U
#define SCMI_PWR_MIX_SLICE_IDX_A55C1         6U
#define SCMI_PWR_MIX_SLICE_IDX_A55C2         7U
#define SCMI_PWR_MIX_SLICE_IDX_A55C3         8U
#define SCMI_PWR_MIX_SLICE_IDX_A55C4         9U
#define SCMI_PWR_MIX_SLICE_IDX_A55C5         10U
#define SCMI_PWR_MIX_SLICE_IDX_A55P          11U
#define SCMI_PWR_MIX_SLICE_IDX_DDR           12U
#define SCMI_PWR_MIX_SLICE_IDX_DISPLAY       13U
#define SCMI_PWR_MIX_SLICE_IDX_GPU           14U
#define SCMI_PWR_MIX_SLICE_IDX_HSIO_TOP      15U
#define SCMI_PWR_MIX_SLICE_IDX_HSIO_WAON     16U
#define SCMI_PWR_MIX_SLICE_IDX_M7            17U
#define SCMI_PWR_MIX_SLICE_IDX_NETC          18U
#define SCMI_PWR_MIX_SLICE_IDX_NOC           19U
#define SCMI_PWR_MIX_SLICE_IDX_NPU           20U
#define SCMI_PWR_MIX_SLICE_IDX_VPU           21U
#define SCMI_PWR_MIX_SLICE_IDX_WAKEUP        22U

#define SCMI_PWR_MEM_SLICE_IDX_AON           0U
#define SCMI_PWR_MEM_SLICE_IDX_CAMERA        1U
#define SCMI_PWR_MEM_SLICE_IDX_A55C0         2U
#define SCMI_PWR_MEM_SLICE_IDX_A55C1         3U
#define SCMI_PWR_MEM_SLICE_IDX_A55C2         4U
#define SCMI_PWR_MEM_SLICE_IDX_A55C3         5U
#define SCMI_PWR_MEM_SLICE_IDX_A55C4         6U
#define SCMI_PWR_MEM_SLICE_IDX_A55C5         7U
#define SCMI_PWR_MEM_SLICE_IDX_A55P          8U
#define SCMI_PWR_MEM_SLICE_IDX_A55L3         9U
#define SCMI_PWR_MEM_SLICE_IDX_DDR           10U
#define SCMI_PWR_MEM_SLICE_IDX_DISPLAY       11U
#define SCMI_PWR_MEM_SLICE_IDX_GPU           12U
#define SCMI_PWR_MEM_SLICE_IDX_HSIO          13U
#define SCMI_PWR_MEM_SLICE_IDX_M7            14U
#define SCMI_PWR_MEM_SLICE_IDX_NETC          15U
#define SCMI_PWR_MEM_SLICE_IDX_NOC_OCRAM     16U
#define SCMI_PWR_MEM_SLICE_IDX_NOC2          17U
#define SCMI_PWR_MEM_SLICE_IDX_NPU           18U
#define SCMI_PWR_MEM_SLICE_IDX_VPU           19U
#define SCMI_PWR_MEM_SLICE_IDX_WAKEUP        20U


extern void* imx95_scmi_handle;

static bool boot_stage[6] = {false, true, true, true, true, true};
static uint32_t scmi_cpu_id[] = {
	IMX9_SCMI_CPU_A55C0, IMX9_SCMI_CPU_A55C1, IMX9_SCMI_CPU_A55C2,
	IMX9_SCMI_CPU_A55C3, IMX9_SCMI_CPU_A55C4, IMX9_SCMI_CPU_A55C5,
	IMX9_SCMI_CPU_A55P
};

static struct scmi_cpu_pd_info cpu_info[] = {
	{
		.cpu_id = IMX9_SCMI_CPU_A55C0,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C0,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C0},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55C1,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C1,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C1},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55C2,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C2,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C2},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55C3,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C3,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C3},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55C4,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C4,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C4},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55C5,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55C5,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55C5},
	},
	{
		.cpu_id = IMX9_SCMI_CPU_A55P,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55P,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55L3},
	}
};

/* for GIC context save/restore if NOC lost power */
struct plat_gic_ctx imx_gicv3_ctx;
/* platfrom secure warm boot entry */
static uintptr_t secure_entrypoint;

void imx_set_sys_wakeup(uint32_t last_core, bool pdn)
{
	uint32_t i;
	uint32_t irq_mask[IMR_NUM] = {
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	};

	uint32_t wakeup_flags;
	uint32_t mode;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn) {
		/*
		 * If NOCMIX power down, need to switch the primary core &
		 * cluster wakeup source to GPC as GIC will be power down.
		 */
		wakeup_flags = SCMI_GPC_WAKEUP;
		mode = SCMI_CPU_SLEEP_SUSPEND;
	} else {
		/* switch to GIC wakeup source for last_core and cluster */
		wakeup_flags = SCMI_GIC_WAKEUP;
		mode = SCMI_CPU_SLEEP_RUN;
	}

	/*
	 * Set IRQ wakeup mask for last core. As a workaorund for HW bug all wakeup
	 * interrupts are directed to the cluster
	 */
	scmi_core_Irq_wake_set(imx95_scmi_handle, cpu_info[last_core].cpu_id,
			       0, IMR_NUM, irq_mask);

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (i = 0; i < IMR_NUM; i++) {
		if (pdn) {
			/* set the wakeup irq based on GIC */
			irq_mask[i] =
				~gicd_read_isenabler(gicd_base, 32 * (i + 1));
		}
	}

	/* Set IRQ wakeup mask for the cluster */
	scmi_core_Irq_wake_set(imx95_scmi_handle, cpu_info[IMX95_A55P_IDX].cpu_id,
			       0, IMR_NUM, irq_mask);


	/* switch to GPC wakeup source, config the target mode to SUSPEND */
	scmi_core_set_sleep_mode(imx95_scmi_handle, scmi_cpu_id[last_core],
				 wakeup_flags | SCMI_RESUME_CPU, mode);
	scmi_core_set_sleep_mode(imx95_scmi_handle, scmi_cpu_id[IMX95_A55P_IDX],
				 wakeup_flags, mode);
}

void nocmix_pwr_down(uint32_t core_id)
{
	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, true);
}

void nocmix_pwr_up(uint32_t core_id)
{
	/* Restore GIC context. */
	plat_gic_restore(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, false);
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(uint32_t power_state,
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

void imx_set_cpu_boot_entry(uint32_t core_id, uint64_t boot_entry,
			    uint32_t flag)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	scmi_core_set_reset_addr(imx95_scmi_handle, boot_entry,
				 scmi_cpu_id[core_id], flag);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);

	if (boot_stage[core_id]) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint,
				       SCMI_CPU_VEC_FLAGS_BOOT);

		boot_stage[core_id] = false;
	}

	scmi_core_start(imx95_scmi_handle, scmi_cpu_id[core_id]);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);

	scmi_core_set_sleep_mode(imx95_scmi_handle, cpu_info[core_id].cpu_id,
				 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t mask[IMR_NUM] = {
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
	};

	plat_gic_cpuif_disable();
	/* Ensure the cluster can be powered off. */
	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * so we need to use SW_WAKEUP for hotplug purpose
	 */
	scmi_core_Irq_wake_set(imx95_scmi_handle, scmi_cpu_id[core_id], 0,
			       IMR_NUM, mask);
	scmi_core_set_sleep_mode(imx95_scmi_handle, scmi_cpu_id[core_id],
				 SCMI_GPC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t val;

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint,
				       SCMI_CPU_VEC_FLAGS_RESUME);

		plat_gic_cpuif_disable();
	}

	/* do cluster level config */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* config DSU for cluster power down */
		val = read_clusterpwrdn();
		val &= ~DSU_CLUSTER_PWR_MASK;
		val = DSU_CLUSTER_PWR_OFF;
		/* L3 retention */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			val |= BIT(1);
		}

		write_clusterpwrdn(val);
	}

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		nocmix_pwr_down(core_id);
		/*
		 * Setup NOC and WAKEUP MIX to power down when Linux suspends.
		 */
		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				SCMI_PWR_MIX_SLICE_IDX_NOC,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				BIT_32(SCMI_PWR_MEM_SLICE_IDX_NOC_OCRAM)
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				0
			}
		};

		/* Set the default LPM state for suspend/hotplug */
		scmi_core_lpm_mode_set(imx95_scmi_handle,
				       cpu_info[IMX95_A55P_IDX].cpu_id,
				       sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
				       cpu_lpm_cfg);
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	uint32_t core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t val;

	/* system level */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		nocmix_pwr_up(core_id);
		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				SCMI_PWR_MIX_SLICE_IDX_NOC,
				SCMI_CPU_PD_LPM_ON_ALWAYS,
				0
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
				SCMI_CPU_PD_LPM_ON_ALWAYS,
				0
			}
		};

		/* Set the default LPM state for RUN MODE */
		scmi_core_lpm_mode_set(imx95_scmi_handle,
				       cpu_info[IMX95_A55P_IDX].cpu_id,
				       sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
				       cpu_lpm_cfg);
	}

	/* cluster level */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/*
		 * clear DSU cluster power down if cluster power off is
		 * aborted by wakeup
		 */
		val = read_clusterpwrdn();
		val &= ~(DSU_CLUSTER_PWR_MASK | BIT(1));
		val |= DSU_CLUSTER_PWR_ON;
		write_clusterpwrdn(val);
	}

	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_enable();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint32_t i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}

	SYSTEM_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1) {
		wfi();
	}
}

void __dead2 imx_system_reset(void)
{
	int ret;

	/* TODO: temp workaround for GIC to let reset done */
	gicd_clr_ctlr(PLAT_GICD_BASE,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	/* Force: work, Gracefull: not work */
	ret = scmi_sys_pwr_state_set(imx95_scmi_handle,
				     SCMI_SYS_PWR_FORCEFUL_REQ,
				     SCMI_SYS_PWR_COLD_RESET);
	if (ret) {
		VERBOSE("%s failed: %d\n", __func__, ret);
	}

	while (true)
		;
}

void __dead2 imx_system_off(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(imx95_scmi_handle,
				     SCMI_SYS_PWR_FORCEFUL_REQ,
				     SCMI_SYS_PWR_SHUTDOWN);
	if (ret) {
		NOTICE("%s failed: %d\n", __func__, ret);
	}

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

	imx_set_cpu_boot_entry(0, secure_entrypoint, SCMI_CPU_VEC_FLAGS_BOOT);

	/* Setup A55 Cluster state for Cpuidle. */
	struct scmi_lpm_config cpu_lpm_cfg[] = {
		{
			cpu_info[IMX95_A55P_IDX].cpu_pd_id,
			SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
			BIT_32(SCMI_PWR_MEM_SLICE_IDX_A55L3)
		},
		{
			SCMI_PWR_MIX_SLICE_IDX_NOC,
			SCMI_CPU_PD_LPM_ON_ALWAYS,
			0
		},
		{
			SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
			SCMI_CPU_PD_LPM_ON_ALWAYS,
			0
		}
	};

	/* Set the default LPM state for suspend/hotplug */
	scmi_core_lpm_mode_set(imx95_scmi_handle,
			       cpu_info[IMX95_A55P_IDX].cpu_id,
			       sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
			       cpu_lpm_cfg);

	/*
	 * Set core/custer to GIC wakeup source since NOCMIX is not
	 * powered down, config the target mode to SUSPEND
	 */
	scmi_core_set_sleep_mode(imx95_scmi_handle, scmi_cpu_id[0],
				 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);

	/*
	 * Set core/custer to GIC wakeup source since NOCMIX is not
	 * powered down, config the target mode to SUSPEND
	 */
	scmi_core_set_sleep_mode(imx95_scmi_handle, scmi_cpu_id[IMX95_A55P_IDX],
				 SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
