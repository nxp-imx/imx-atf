/*
 * Copyright 2019-2020 NXP
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

#include <dram.h>
#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	imx_set_cpu_pwr_off(core_id);

	/* TODO: Find out why this is still
	 * needed in order not to break suspend */
	udelay(50);
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t base_addr = BL31_BASE;
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_set_cpu_secure_entry(core_id, base_addr);
		imx_set_cpu_lpm(core_id, true);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, CLUSTER_PWR_STATE(target_state));

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		if (!imx_m4_lpa_active()) {
			imx_set_sys_lpm(core_id, true);
			dram_enter_retention();
			imx_anamix_override(true);
			imx_noc_wrapper_pre_suspend(core_id);
		} else {
			/*
			 * when A53 don't enter DSM, only need to
			 * set the system wakeup option.
			 */
			imx_set_sys_wakeup(core_id, true);
		}
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
		if (!imx_m4_lpa_active()) {
			imx_noc_wrapper_post_resume(core_id);
			imx_anamix_override(false);
			dram_exit_retention();
			imx_set_sys_lpm(core_id, false);
		} else {
			imx_set_sys_wakeup(core_id, false);
		}
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		imx_clear_rbc_count();
		imx_set_cluster_powerdown(core_id, PSCI_LOCAL_STATE_RUN);
	}

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* mark this core as awake by masking IRQ0 */
		imx_gpc_set_a53_core_awake(core_id);
		imx_set_cpu_lpm(core_id, false);
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
		isb();
	}
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	imx_mailbox_init(sec_entrypoint);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
