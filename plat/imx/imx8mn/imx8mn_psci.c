/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <stdbool.h>
#include <dram.h>
#include <plat_imx8.h>
#include <psci.h>
#include <mmio.h>
#include <soc.h>
#include <delay_timer.h>

#define SNVS_LPCR	0x38

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

int imx_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core_id;
	uint64_t base_addr = BL31_BASE;

	core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* set the secure entrypoint */
	imx_set_cpu_secure_entry(core_id, base_addr);
	/* power up the core */
	imx_set_cpu_pwr_on(core_id);

	return PSCI_E_SUCCESS;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	/* program the GIC per cpu dist and rdist interface */
	plat_gic_pcpu_init();
	/* enable the GICv3 cpu interface */
	plat_gic_cpuif_enable();
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* disable the GIC cpu interface first */
	plat_gic_cpuif_disable();
	/* config the core for power down */
	imx_set_cpu_pwr_off(core_id);
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* The non-secure entrypoint should be in RAM space */
	if (ns_entrypoint < 0x40000000)
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
		CLUSTER_PWR_STATE(req_state) = PLAT_WAIT_RET_STATE;
	}

	return PSCI_E_SUCCESS;
}

void imx_cpu_standby(plat_local_state_t cpu_state)
{
	dsb();
	write_scr_el3(read_scr_el3() | 0x4);
	isb();

	wfi();

	write_scr_el3(read_scr_el3() & (~0x4));
	isb();
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t base_addr = BL31_BASE;
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* disable the cpu interface */
		plat_gic_cpuif_disable();
		/* set the resume entry */
		imx_set_cpu_secure_entry(core_id, base_addr);
		imx_set_cpu_lpm(core_id, true);
	} else {
		/* TODO cluster level clock gate off ? */
		dsb();
		write_scr_el3(read_scr_el3() | 0x4);
		isb();
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, CLUSTER_PWR_STATE(target_state));

	/* do system level power mode setting */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		if (!imx_is_m4_enabled() || !imx_m4_lpa_active())
			dram_enter_retention();
		imx_set_sys_lpm(true);
		imx_anamix_pre_suspend();
		noc_wrapper_pre_suspend(core_id);
		/* set the wakeup source based on GIC SPI config */
		imx_set_sys_wakeup(core_id, true);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* check the system level status */
	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		imx_set_sys_lpm(false);
		/* clear the system wakeup setting */
		imx_set_sys_wakeup(core_id, false);
		imx_anamix_post_resume();
		if (!imx_is_m4_enabled() || !imx_m4_lpa_active())
			dram_exit_retention();
		noc_wrapper_post_resume(core_id);
	}

	/* check the cluster level power status */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		imx_clear_rbc_count();
		imx_set_cluster_powerdown(core_id, PSCI_LOCAL_STATE_RUN);
	}

	/* check the core level power status */
	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* clear the core lpm setting */
		imx_set_cpu_lpm(core_id, false);
		/* enable the gic cpu interface */
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~0x4));
		isb();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i < PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_STOP_OFF_STATE;

	req_state->pwr_domain_state[PLAT_MAX_PWR_LVL] = PLAT_MAX_RET_STATE;
}

void __dead2 imx_system_reset(void)
{
	uintptr_t wdog_base = IMX_WDOG_BASE;
	unsigned int val;

	/* WDOG_B reset */
	val = mmio_read_16(wdog_base);
#ifdef IMX_WDOG_B_RESET
	val = (val & 0x00FF) | (7 << 2) | (1 << 0);
#else
	val = (val & 0x00FF) | (4 << 2) | (1 << 0);
#endif
	mmio_write_16(wdog_base, val);

	mmio_write_16(wdog_base + 0x2, 0x5555);
	mmio_write_16(wdog_base + 0x2, 0xaaaa);
	while (1)
		;
}

void __dead2 imx_system_off(void)
{
	mmio_write_32(IMX_SNVS_BASE + SNVS_LPCR, 0x61);

	INFO("Unable to poweroff system\n");

	while (1)
		;
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	/*
	 * before enter WAIT or STOP mode with PLAT(SCU) power down,
	 * rbc count need to be enabled to make sure PLAT is
	 * power down successfully even if the the wakeup IRQ is pending
	 * early before the power down sequence. the RBC counter is
	 * drived by the 32K OSC, so delay 30us to make sure the counter
	 * is really running.
	 */
	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		imx_set_rbc_count();
		udelay(30);
	}

	while (1)
		wfi();
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
