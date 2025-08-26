/*
 * Copyright 2026 NXP
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

#include <imx_scmi_client.h>
#include <plat_imx8.h>
#include <scmi_imx9.h>

#define IMR_NUM		U(13)
#define IMX9_A55P_IDX			4

#define IMX9_SCMI_CPU_A55C0		2
#define IMX9_SCMI_CPU_A55C1		3
#define IMX9_SCMI_CPU_A55C2		4
#define IMX9_SCMI_CPU_A55C3		5
#define IMX9_SCMI_CPU_A55P		6

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
#define SCMI_PWR_MIX_SLICE_IDX_A55P          9U
#define SCMI_PWR_MIX_SLICE_IDX_DDR           10U
#define SCMI_PWR_MIX_SLICE_IDX_DISPLAY       11U
#define SCMI_PWR_MIX_SLICE_IDX_GPU           12U
#define SCMI_PWR_MIX_SLICE_IDX_HSIO_TOP      13U
#define SCMI_PWR_MIX_SLICE_IDX_HSIO_WAON     14U
#define SCMI_PWR_MIX_SLICE_IDX_M7            15U
#define SCMI_PWR_MIX_SLICE_IDX_NETC          16U
#define SCMI_PWR_MIX_SLICE_IDX_NOC           17U
#define SCMI_PWR_MIX_SLICE_IDX_NPU           18U
#define SCMI_PWR_MIX_SLICE_IDX_VPU           19U
#define SCMI_PWR_MIX_SLICE_IDX_WAKEUP        20U

#define SCMI_PWR_MEM_SLICE_IDX_AON           0U
#define SCMI_PWR_MEM_SLICE_IDX_CAMERA        1U
#define SCMI_PWR_MEM_SLICE_IDX_A55C0         2U
#define SCMI_PWR_MEM_SLICE_IDX_A55C1         3U
#define SCMI_PWR_MEM_SLICE_IDX_A55C2         4U
#define SCMI_PWR_MEM_SLICE_IDX_A55C3         5U
#define SCMI_PWR_MEM_SLICE_IDX_A55P          6U
#define SCMI_PWR_MEM_SLICE_IDX_A55L3         7U
#define SCMI_PWR_MEM_SLICE_IDX_DDR           8U
#define SCMI_PWR_MEM_SLICE_IDX_DISPLAY       9U
#define SCMI_PWR_MEM_SLICE_IDX_GPU           10U
#define SCMI_PWR_MEM_SLICE_IDX_HSIO          11U
#define SCMI_PWR_MEM_SLICE_IDX_M7            12U
#define SCMI_PWR_MEM_SLICE_IDX_NETC          13U
#define SCMI_PWR_MEM_SLICE_IDX_NOC1          14U
#define SCMI_PWR_MEM_SLICE_IDX_NOC2          15U
#define SCMI_PWR_MEM_SLICE_IDX_NPU           16U
#define SCMI_PWR_MEM_SLICE_IDX_VPU           17U
#define SCMI_PWR_MEM_SLICE_IDX_WAKEUP        18U

#define DEBUG_WAKEUP_MASK BIT(1)
#define EVENT_WAKEUP_MASK BIT(0)

static bool boot_stage[6] = {false, true, true, true, true, true};
static unsigned int scmi_cpu_id[] = {
	IMX9_SCMI_CPU_A55C0, IMX9_SCMI_CPU_A55C1,
	IMX9_SCMI_CPU_A55C2, IMX9_SCMI_CPU_A55C3,
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
		.cpu_id = IMX9_SCMI_CPU_A55P,
		.cpu_pd_id = SCMI_PWR_MIX_SLICE_IDX_A55P,
		.nmem = 1,
		.cpu_mem_pd_id = (uint32_t []){SCMI_PWR_MEM_SLICE_IDX_A55L3},
	}
};

/* for GIC context save/restore if NIC lost power */
struct plat_gic_ctx imx_gicv3_ctx;
/* platfrom secure warm boot entry */
static uintptr_t secure_entrypoint;

/*
 * IRQ masks used to check if any of the below IRQ is
 * enabled as the wakeup source:
 * lpuart3-8: 64-69, flexcan2,3: 38, 40
 * usdhc1,2,3: 86, 87, 191
 * netc: 304
 */
static uint32_t wakeup_irq_mask[IMR_NUM] = {
	0x0, 0x0140, 0xc0003f, 0x0,
	0x0, 0x80000000, 0x0, 0x0,
	0x0, 0x10000
};

static bool has_wakeup_irq;

static struct qchannel_hsk_config {
	const uint32_t per_idx;
	const unsigned int wakeup_irq;
	bool active_wakeup;
} hsk_config[] = {
	{ CPU_PER_LPI_IDX_CAN1, 8 },
	{ CPU_PER_LPI_IDX_CAN2, 38 },
	{ CPU_PER_LPI_IDX_CAN3, 40 },

	{ CPU_PER_LPI_IDX_LPUART1, 19 },
	{ CPU_PER_LPI_IDX_LPUART3, 64 },
	{ CPU_PER_LPI_IDX_LPUART4, 65 },
	{ CPU_PER_LPI_IDX_LPUART5, 66 },
	{ CPU_PER_LPI_IDX_LPUART6, 67 },
	{ CPU_PER_LPI_IDX_LPUART7, 68 },
	{ CPU_PER_LPI_IDX_LPUART8, 69 },

	{ CPU_PER_LPI_IDX_GPIO2 },
	{ CPU_PER_LPI_IDX_GPIO3 },
	{ CPU_PER_LPI_IDX_GPIO4 },
	{ CPU_PER_LPI_IDX_GPIO5 },
};

static inline void is_wakeup_source(unsigned int gic_irq_mask,
				    uint32_t idx)
{
	for (uint32_t i = 0; i < ARRAY_SIZE(hsk_config); i++) {
		if (hsk_config[i].wakeup_irq &&
                    (idx == hsk_config[i].wakeup_irq / 32))
			hsk_config[i].active_wakeup =
						!(gic_irq_mask & ( 1 << hsk_config[i].wakeup_irq % 32));
	}
}

/*
 * For peripherals like CANs, GPIOs & UARTs that need to support async wakeup
 * when clock is gated, LPCGs of these IPs need to be changed to CPU LPM
 * controlled, and for CANs &UARTs, we also need to make sure its ROOT clock
 * slice is enabled.
 */
void peripheral_qchannel_hsk(bool en, uint32_t last_core)
{
	unsigned int i;
	struct scmi_per_lpm_config per_lpm[ARRAY_SIZE(hsk_config)];
	uint32_t num_hsks_enabled = 0;

	for (i = 0; i < ARRAY_SIZE(hsk_config); i++) {
		if (en) {
			/* Only enable the qchannel handshake for active wakeup used by A55 */
			if (!hsk_config[i].wakeup_irq) {
				 hsk_config[i].active_wakeup = true;
			}
			if (hsk_config[i].active_wakeup) {
				per_lpm[num_hsks_enabled].perId = hsk_config[i].per_idx;
				per_lpm[num_hsks_enabled].lpmSetting = SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP;
				num_hsks_enabled++;
			} else {
				hsk_config[i].active_wakeup = false;
			}
		} else if (hsk_config[i].active_wakeup) {
			/* restore the initial config */
			per_lpm[num_hsks_enabled].perId = hsk_config[i].per_idx;
			per_lpm[num_hsks_enabled].lpmSetting = SCMI_CPU_PD_LPM_ON_ALWAYS;
			num_hsks_enabled++;
			hsk_config[i].active_wakeup = false;
		}
	}

	scmi_per_lpm_mode_set(imx9_scmi_handle, scmi_cpu_id[last_core],
			num_hsks_enabled, per_lpm);
}

void imx_set_sys_wakeup(unsigned int last_core, bool pdn)
{
	unsigned int i;
	uint32_t irq_mask[IMR_NUM] = {
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff
	};
	uint32_t wakeup_flags;
	uint32_t mode;
	uintptr_t gicd_base = PLAT_GICD_BASE;

	if (pdn) {
		/*
		 * If NICMIX power down, need to switch the primary core & cluster wakeup
		 * source to GPC as GIC will be power down.
		 */
		wakeup_flags = SCMI_GPC_WAKEUP;
		mode = SCMI_CPU_SLEEP_SUSPEND;
	} else {
		/* switch to GIC wakeup source for last_core and cluster */
		wakeup_flags = SCMI_GIC_WAKEUP;
		mode = SCMI_CPU_SLEEP_WAIT;
		has_wakeup_irq = false;
	}

	/* Set the GPC IMRs based on GIC IRQ mask setting */
	for (i = 0; i < IMR_NUM; i++) {
		if (pdn) {
			/* set the wakeup irq base GIC */
			irq_mask[i] = ~gicd_read_isenabler(gicd_base, 32 * (i + 1));
			is_wakeup_source(irq_mask[i], i);
		}

		if ((irq_mask[i] & wakeup_irq_mask[i]) != wakeup_irq_mask[i]) {
			has_wakeup_irq = true;
		}
	}

	/* Set IRQ wakeup mask for both last core and cluster. */
	scmi_core_Irq_wake_set(imx9_scmi_handle, cpu_info[last_core].cpu_id,
			   0, IMR_NUM, irq_mask);

	/* Set IRQ wakeup mask for the cluster */
	scmi_core_Irq_wake_set(imx9_scmi_handle, cpu_info[IMX9_A55P_IDX].cpu_id,
			       0, IMR_NUM, irq_mask);

	/*
	 * switch to GPC wakeup source, config the target mode to SUSPEND
	 * for both the cluster and last core.
	 */
	scmi_core_set_sleep_mode(imx9_scmi_handle, scmi_cpu_id[last_core],
				 wakeup_flags, mode);
	scmi_core_set_sleep_mode(imx9_scmi_handle, scmi_cpu_id[IMX9_A55P_IDX],
				 wakeup_flags, mode);

	peripheral_qchannel_hsk(pdn, IMX9_A55P_IDX);
}

void nocmix_pwr_down(unsigned int core_id)
{
	/* Save the gic context */
	plat_gic_save(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, true);
}

void nocmix_pwr_up(unsigned int core_id)
{
	/* Restore GIC context. */
	plat_gic_restore(core_id, &imx_gicv3_ctx);
	imx_set_sys_wakeup(core_id, false);
}


int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < PLAT_NS_IMAGE_OFFSET)
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state, psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int pwr_type = psci_get_pstate_type(power_state);
	int state_id = psci_get_pstate_id(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (pwr_type == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	if (pwr_type == PSTATE_TYPE_POWERDOWN && state_id == 0x33) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_set_cpu_boot_entry(unsigned int core_id, uint64_t boot_entry, uint32_t flag)
{
	/* set the cpu core reset entry: BLK_CTRL_S */
	scmi_core_set_reset_addr(imx9_scmi_handle, boot_entry,
				scmi_cpu_id[core_id], flag);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	uint32_t mask = DEBUG_WAKEUP_MASK | EVENT_WAKEUP_MASK;

	if (boot_stage[core_id]) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint, SCMI_CPU_VEC_FLAGS_BOOT);

		boot_stage[core_id] = false;
	}
	scmi_core_start(imx9_scmi_handle, scmi_cpu_id[core_id]);

	/*
	 * Set NON-IRQ wakeup mask for both last core and cluster.
	 * Disable wakeup on DEBUG_WAKEUP
	 */
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, cpu_info[core_id].cpu_id, 0, 1, mask);

	/* Set the LPM state for cpuidle. */
	struct scmi_lpm_config cpu_lpm_cfg = {cpu_info[core_id].cpu_pd_id,
	   SCMI_CPU_PD_LPM_ON_RUN, 0};
	/* Set the default LPM state for cpuidle */
	scmi_core_lpm_mode_set(imx9_scmi_handle, cpu_info[core_id].cpu_id,
			        1, &cpu_lpm_cfg);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	scmi_core_set_sleep_mode(imx9_scmi_handle, cpu_info[core_id].cpu_id,
		SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

	plat_gic_pcpu_init();
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	static uint32_t mask[] = {
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		0xffffffff
	};

	plat_gic_cpuif_disable();

	/* Ensure the cluster can be powered off. */
	write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);

	/* Configure core LPM state for hotplug. */
	struct scmi_lpm_config cpu_lpm_cfg = {cpu_info[core_id].cpu_pd_id,
	   SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP, 0};
	/* Set the default LPM state for cpuidle */
	scmi_core_lpm_mode_set(imx9_scmi_handle, cpu_info[core_id].cpu_id,
			        1, &cpu_lpm_cfg);

	/*
	 * mask all the GPC IRQ wakeup to make sure no IRQ can wakeup this core
	 * as we need to use SW_WAKEUP for hotplug purpose
	 */
	scmi_core_Irq_wake_set(imx9_scmi_handle, scmi_cpu_id[core_id], 0, IMR_NUM, mask);
	scmi_core_set_sleep_mode(imx9_scmi_handle, scmi_cpu_id[core_id],
				SCMI_GPC_WAKEUP, SCMI_CPU_SLEEP_SUSPEND);
}

void imx_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);
	bool keep_wakupmix_on = false;
	uint32_t l3_retn = 0;

	/* do cpu level config */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		imx_set_cpu_boot_entry(core_id, secure_entrypoint, SCMI_CPU_VEC_FLAGS_RESUME);

		plat_gic_cpuif_disable();
	}

	/* config DSU for cluster power down */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		/* L3 retention */
		if (is_local_state_retn(CLUSTER_PWR_STATE(target_state))) {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF | BIT(1));
			l3_retn = BIT_32(SCMI_PWR_MEM_SLICE_IDX_A55L3);
		} else {
			write_clusterpwrdn(DSU_CLUSTER_PWR_OFF);
			l3_retn = 0;
		}
	}

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		nocmix_pwr_down(core_id);
		keep_wakupmix_on = has_wakeup_irq;
		/*
		 * Setup NOCMIX to power down when Linux suspends.
		 * This is needs to be updated when wakeupmix can be powered down too.
		 */
		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				cpu_info[IMX9_A55P_IDX].cpu_pd_id,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				l3_retn
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_NOC,
				SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				BIT_32(SCMI_PWR_MEM_SLICE_IDX_NOC1) | BIT(SCMI_PWR_MEM_SLICE_IDX_NOC2)
			},
			{
				SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
				keep_wakupmix_on ? SCMI_CPU_PD_LPM_ON_ALWAYS :
					SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP,
				0
			}
		};

		/* Set the default LPM state for suspend/hotplug */
		scmi_core_lpm_mode_set(imx9_scmi_handle, cpu_info[IMX9_A55P_IDX].cpu_id,
				sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
				cpu_lpm_cfg);
	}
}

void imx_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL1_VAL(mpidr);

	/* system level */
	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		nocmix_pwr_up(core_id);
		struct scmi_lpm_config cpu_lpm_cfg[] = {
			{
				cpu_info[IMX9_A55P_IDX].cpu_pd_id,
				SCMI_CPU_PD_LPM_ON_RUN,
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

		/* Set the default LPM state for RUN MODE */
		scmi_core_lpm_mode_set(imx9_scmi_handle, cpu_info[IMX9_A55P_IDX].cpu_id,
				sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
				cpu_lpm_cfg);
	}

	/* do core level */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
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
	ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
				     SCMI_SYS_PWR_FORCEFUL_REQ,
				     SCMI_SYS_PWR_COLD_RESET);
	if (ret) {
		VERBOSE("%s failed: %d\n", __func__, ret);
	}

	while (true)
		;
}

int __dead2 imx_system_reset2(int is_vendor, int reset_type, u_register_t cookie)
{
	int ret;

	/* TODO: temp workaround for GIC to let reset done */
	gicd_clr_ctlr(PLAT_GICD_BASE,
		      CTLR_ENABLE_G0_BIT |
		      CTLR_ENABLE_G1S_BIT |
		      CTLR_ENABLE_G1NS_BIT,
		      RWP_TRUE);

	switch(reset_type) {
	case PSCI_RESET2_SYSTEM_WARM_RESET:
		/* Force: work, Gracefull: not work */
		ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
					     SCMI_SYS_PWR_FORCEFUL_REQ,
					     SCMI_SYS_PWR_WARM_RESET);
		break;
	case PSCI_RESET2_SYSTEM_COLD_RESET:
		/* Force: work, Gracefull: not work */
		ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
					     SCMI_SYS_PWR_FORCEFUL_REQ,
					     SCMI_SYS_PWR_COLD_RESET);
		break;
	case PSCI_RESET2_SYSTEM_BOARD_RESET:
		/* Force: work, Gracefull: not work */
		ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
					     SCMI_SYS_PWR_FORCEFUL_REQ,
					     SCMI_SYS_STATE_FULL_RESET);
		break;
	default:
		ret = PSCI_E_INVALID_PARAMS;
	}

	if (ret) {
		VERBOSE("%s failed: %d\n", __func__, ret);
	}

	while (true)
		;
}

void __dead2 imx_system_off(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(imx9_scmi_handle,
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
	.system_reset2 = imx_system_reset2,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uint32_t mask = DEBUG_WAKEUP_MASK | EVENT_WAKEUP_MASK;

	/* sec_entrypoint is used for warm reset */
	secure_entrypoint = sec_entrypoint;
	imx_set_cpu_boot_entry(0, secure_entrypoint, SCMI_CPU_VEC_FLAGS_BOOT);

	/*
	 * Set NON-IRQ wakeup mask for both last core and cluster.
	 * Disable wakeup on DEBUG_WAKEUP
	 */
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, scmi_cpu_id[0], 0, 1, mask);
	scmi_core_nonIrq_wake_set(imx9_scmi_handle, scmi_cpu_id[IMX9_A55P_IDX], 0, 1, mask);

	/*
	 * Setup A55 Cluster state for Cpuidle.
	 */
	struct scmi_lpm_config cpu_lpm_cfg[] =
	{
		{cpu_info[IMX9_A55P_IDX].cpu_pd_id,
		SCMI_CPU_PD_LPM_ON_RUN,
		BIT_32(SCMI_PWR_MEM_SLICE_IDX_A55L3)
		},
		{
		SCMI_PWR_MIX_SLICE_IDX_NOC,
		SCMI_CPU_PD_LPM_ON_ALWAYS,
		0},
		{
		SCMI_PWR_MIX_SLICE_IDX_WAKEUP,
		SCMI_CPU_PD_LPM_ON_ALWAYS,
		0}
	};
	/* Set the default LPM state for suspend/hotplug */
	scmi_core_lpm_mode_set(imx9_scmi_handle,
			       cpu_info[IMX9_A55P_IDX].cpu_id,
			       sizeof(cpu_lpm_cfg)/sizeof(struct scmi_lpm_config),
			       cpu_lpm_cfg);

	/* Set the LPM state for cpuidle for A55C0 (boot core) */
	cpu_lpm_cfg[0].power_domain = cpu_info[0].cpu_pd_id;
	cpu_lpm_cfg[0].lpmsetting = SCMI_CPU_PD_LPM_ON_RUN;
	cpu_lpm_cfg[0].retentionmask = 0;
	scmi_core_lpm_mode_set(imx9_scmi_handle, cpu_info[0].cpu_id,
			        1, cpu_lpm_cfg);

	/*
	 * Set core/custer to GIC wakeup source since NOCMIX is not
	 * powered down, config the target mode to WAIT
	 */
	scmi_core_set_sleep_mode(imx9_scmi_handle, scmi_cpu_id[0],
			        SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

	/*
	 * Set core/custer to GIC wakeup source since NOCMIX is not
	 * powered down, config the target mode to WAIT
	 */
	scmi_core_set_sleep_mode(imx9_scmi_handle, scmi_cpu_id[IMX9_A55P_IDX],
			        SCMI_GIC_WAKEUP, SCMI_CPU_SLEEP_WAIT);

	/* Enable the wdog3 per handshake */
	struct scmi_per_lpm_config per_lpm[1] = {
		{ CPU_PER_LPI_IDX_WDOG3, SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP },
	};

	scmi_per_lpm_mode_set(imx9_scmi_handle, scmi_cpu_id[IMX9_A55P_IDX],
			      1, per_lpm);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
