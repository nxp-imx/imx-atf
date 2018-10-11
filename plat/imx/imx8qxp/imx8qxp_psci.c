/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <gicv3.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <psci.h>
#include <sci/sci.h>
#include "../../common/sci/mx8_mu.h"

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

extern sc_ipc_t ipc_handle;
extern void mdelay(uint32_t msec);
extern bool wakeup_src_irqsteer;

/* save gic dist/redist context when GIC is power down */
static struct plat_gic_ctx imx_gicv3_ctx;
static unsigned int gpt_lpcg, gpt_reg[2];

const unsigned char imx_power_domain_tree_desc[] =
{
	/* number of root nodes */
	PWR_DOMAIN_AT_MAX_LVL,
	/* number of child at the first node */
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CORE_COUNT,
};

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A35_0, SC_R_A35_1, SC_R_A35_2, SC_R_A35_3
};

static void imx_enable_irqstr_wakeup(void)
{
	uint32_t irq_mask;
	gicv3_dist_ctx_t *dist_ctx = &imx_gicv3_ctx.dist_ctx;

	/* put IRQSTR into ON mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_ON);

	/* enable the irqsteer to handle wakeup irq */
	mmio_write_32(IMX_WUP_IRQSTR, 0x1);
	for (int i = 0; i < 15; i++) {
		irq_mask = dist_ctx->gicd_isenabler[i];
		mmio_write_32(IMX_WUP_IRQSTR + 0x3c - 0x4 * i, irq_mask);
	}

	/* set IRQSTR low power mode */
	if (wakeup_src_irqsteer)
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_STBY);
	else
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_OFF);
}

static void imx_disable_irqstr_wakeup(void)
{
	/* Put IRQSTEER back to ON mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_ON);

	/* disable the irqsteer */
	mmio_write_32(IMX_WUP_IRQSTR, 0x0);
	for (int i = 0; i < 16; i ++)
		mmio_write_32(IMX_WUP_IRQSTR + 0x4 + 0x4 * i, 0x0);

	/* Put IRQSTEER into OFF mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_OFF);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id],
	    SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
		ERROR("core %d power on failed!\n", cpu_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id],
	    true, 0x80000000) != SC_ERR_NONE) {
		ERROR("boot core %d failed!\n", cpu_id);
		ret = PSCI_E_INTERN_FAIL;
	}

	return ret;
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
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id], SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_NONE);
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1)
		wfi();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/*
	 * U-Boot and ATFW are combined into one image, let ROM HAB
	 * validate the whole file.
	 */
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

	if (pwr_type == PSTATE_TYPE_POWERDOWN) {
		CORE_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
		if (!state_id)
			CLUSTER_PWR_STATE(req_state) = PLAT_MAX_RET_STATE;
		else
			CLUSTER_PWR_STATE(req_state) = PLAT_MAX_OFF_STATE;
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
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		sc_pm_set_cpu_resume(ipc_handle, ap_core_index[cpu_id], true, 0x80000000);
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_GIC);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | 0x4);
		isb();
	}

	if (is_local_state_off(CLUSTER_PWR_STATE(target_state)))
		sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_OFF);

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();

		/* save gic context */
		plat_gic_save(cpu_id, &imx_gicv3_ctx);
		/* enable the irqsteer for wakeup */
		imx_enable_irqstr_wakeup();

		/* Save GPT clock and registers, then turn off its power */
		gpt_lpcg = mmio_read_32(IMX_GPT0_LPCG_BASE);
		gpt_reg[0] = mmio_read_32(IMX_GPT0_BASE);
		gpt_reg[1] = mmio_read_32(IMX_GPT0_BASE + 0x4);
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_OFF);

		sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);

		/* Put GIC in OFF mode. */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_OFF);
		sc_pm_set_cpu_resume(ipc_handle, ap_core_index[cpu_id], true, 0x080000000);
		if (wakeup_src_irqsteer)
			sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
				SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_IRQSTEER);
		else
			sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
				SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_SCU);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_retn(SYSTEM_PWR_STATE(target_state))) {
		MU_Resume(SC_IPC_CH);

		sc_pm_req_low_power_mode(ipc_handle, ap_core_index[cpu_id], SC_PM_PW_MODE_ON);
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);

		/* Put GIC back to high power mode. */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_ON);

		/* restore gic context */
		plat_gic_restore(cpu_id, &imx_gicv3_ctx);

		/* Turn on GPT power and restore its clock and registers */
		sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_ON);
		sc_pm_clock_enable(ipc_handle, SC_R_GPT_0, SC_PM_CLK_PER, true, 0);
		mmio_write_32(IMX_GPT0_BASE, gpt_reg[0]);
		mmio_write_32(IMX_GPT0_BASE + 0x4, gpt_reg[1]);
		mmio_write_32(IMX_GPT0_LPCG_BASE, gpt_lpcg);

		sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_DDR,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_MU,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
		sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_INTERCONNECT,
			SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);

		/* disable the irqsteer wakeup */
		imx_disable_irqstr_wakeup();

		plat_gic_cpuif_enable();
	}

	if (is_local_state_off(CLUSTER_PWR_STATE(target_state)))
		sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_ON);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~0x4));
		isb();
	}
}

void imx_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	for (i = IMX_PWR_LVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_RET_STATE;
}

void  __attribute__((noreturn)) imx_system_reset(void)
{
	sc_pm_reset(ipc_handle, SC_PM_RESET_TYPE_BOARD);

	while (1)
		;
}

void __attribute__((noreturn)) imx_system_off(void)
{
	sc_pm_set_sys_power_mode(ipc_handle, SC_PM_PW_MODE_OFF);
	wfi();
	ERROR("power off failed.\n");
	panic();
}

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	/* sec_entrypoint is used for warm reset */
	*psci_ops = &imx_plat_psci_ops;

	/* make sure system sources power ON in low power mode by default */
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A35, SC_PM_PW_MODE_ON);

	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_DDR,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_MU,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A35, SC_PM_SYS_IF_INTERCONNECT,
		SC_PM_PW_MODE_ON, SC_PM_PW_MODE_ON);

	return 0;
}
