/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <cci.h>
#include <debug.h>
#include <gicv3.h>
#include <mmio.h>
#include <plat_imx8.h>
#include <psci.h>
#include <sci/sci.h>
#include "../../common/sci/mx8_mu.h"

extern sc_ipc_t ipc_handle;
/* save gic dist/redist context when GIC is poewr down */
static struct plat_gic_ctx imx_gicv3_ctx;

const unsigned char imx_power_domain_tree_desc[] =
{
	/* number of root nodes */
	PWR_DOMAIN_AT_MAX_LVL,
	/* number of child at the first node */
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
	PLATFORM_CLUSTER1_CORE_COUNT,
};

const static int ap_core_index[PLATFORM_CORE_COUNT] = {
	SC_R_A53_0, SC_R_A53_1, SC_R_A53_2,
	SC_R_A53_3, SC_R_A72_0, SC_R_A72_1,
};

/* need to enable USE_COHERENT_MEM to avoid coherence issue */
#if USE_COHERENT_MEM
static unsigned int a53_cpu_on_number __section("tzfw_coherent_mem");
static unsigned int a72_cpu_on_number __section("tzfw_coherent_mem");
#endif

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

	/* Put IRQSTR into STBY mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_STBY);
}

static void imx_disable_irqstr_wakeup(void)
{
	/* Put IRQSTR into ON from STBY mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_ON);

	/* disable the irqsteer */
	mmio_write_32(IMX_WUP_IRQSTR, 0x0);
	for (int i = 0; i < 16; i ++)
		mmio_write_32(IMX_WUP_IRQSTR + 0x4 + 0x4 * i, 0x0);

	/* Put IRQSTR into OFF mode */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_IRQSTR_SCU2, SC_PM_PW_MODE_OFF);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cluster_id, cpu_id;

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	tf_printf("imx_pwr_domain_on cluster_id %d, cpu_id %d\n", cluster_id, cpu_id);

	if (cluster_id == 0) {
		if (a53_cpu_on_number == 0)
			sc_pm_set_resource_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_ON);

		if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
			ERROR("cluster0 core %d power on failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}

		if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id],
			true, 0x80000000) != SC_ERR_NONE) {
			ERROR("boot cluster0 core %d failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}
	} else {
		if (a72_cpu_on_number == 0)
			sc_pm_set_resource_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_ON);

		if (sc_pm_set_resource_power_mode(ipc_handle, ap_core_index[cpu_id + 4],
			SC_PM_PW_MODE_ON) != SC_ERR_NONE) {
			ERROR(" cluster1 core %d power on failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}

		if (sc_pm_cpu_start(ipc_handle, ap_core_index[cpu_id + 4],
			true, 0x80000000) != SC_ERR_NONE) {
			ERROR("boot cluster1 core %d failed!\n", cpu_id);
			ret = PSCI_E_INTERN_FAIL;
		}
	}

	return ret;
}

void imx_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (cluster_id == 0 && a53_cpu_on_number++ == 0)
		cci_enable_snoop_dvm_reqs(0);
	if (cluster_id == 1 && a72_cpu_on_number++ == 0)
		cci_enable_snoop_dvm_reqs(1);

	/* program the GIC per cpu dist and rdist interface */
	plat_gic_pcpu_init();

	/* enable the GICv3 cpu interface */
	plat_gic_cpuif_enable();

	tf_printf("cluster:%d core:%d is on\n", cluster_id, cpu_id);
}

void imx_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();
	if (cluster_id == 0) {
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id],
			SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_NONE);
		if (--a53_cpu_on_number == 0)
			cci_disable_snoop_dvm_reqs(0);
	} else {
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id + 4],
			SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_NONE);
		if (--a72_cpu_on_number == 0)
			cci_disable_snoop_dvm_reqs(1);
	}
	tf_printf("turn off cluster:%d core:%d\n", cluster_id, cpu_id);
}

void __dead2 imx_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	while (1)
		wfi();
}

int imx_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	/* TODO */
	return PSCI_E_SUCCESS;
}

int imx_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested afflvl */
	if (psci_get_pstate_type(power_state) == PSTATE_TYPE_STANDBY) {
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;
		/* power domain in standby state */
		req_state->pwr_domain_state[pwr_lvl] = PLAT_MAX_RET_STATE;

		return PSCI_E_SUCCESS;
	}

	return 0;
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
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	plat_gic_cpuif_disable();

	/* save gic context */
	plat_gic_save(cpu_id, &imx_gicv3_ctx);
	/* enable the irqsteer for wakeup */
	imx_enable_irqstr_wakeup();

	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	/* Put GIC in LP mode. */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_OFF);

	if (cluster_id == 0) {
		sc_pm_set_cpu_resume_addr(ipc_handle, ap_core_index[cpu_id], 0x080000000);
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id], SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_IRQSTEER);
	} else {
		sc_pm_set_cpu_resume_addr(ipc_handle, ap_core_index[cpu_id + 4], 0x080000000);
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id + 4], SC_PM_PW_MODE_OFF, SC_PM_WAKE_SRC_IRQSTEER);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	MU_Resume(SC_IPC_CH);

	if (cluster_id == 0)
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id], SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);
	else
		sc_pm_req_cpu_low_power_mode(ipc_handle, ap_core_index[cpu_id + 4], SC_PM_PW_MODE_ON, SC_PM_WAKE_SRC_GIC);

	/* Put GIC/IRQSTR back to high power mode. */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_GIC, SC_PM_PW_MODE_ON);

	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	/* restore gic context */
	plat_gic_restore(cpu_id, &imx_gicv3_ctx);
	/* disable the irqsteer wakeup */
	imx_disable_irqstr_wakeup();

	plat_gic_cpuif_enable();
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
	ERROR("Power off failed.\n");
	panic();
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
	uint64_t mpidr = read_mpidr_el1();
	unsigned int cluster_id = MPIDR_AFFLVL1_VAL(mpidr);

	imx_mailbox_init(sec_entrypoint);
	/* sec_entrypoint is used for warm reset */
	*psci_ops = &imx_plat_psci_ops;

	if (cluster_id == 0)
		a53_cpu_on_number++;
	else
		a72_cpu_on_number++;

	/* request low power mode for cluster/cci, only need to do once */
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A72, SC_PM_PW_MODE_OFF);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_A53, SC_PM_PW_MODE_OFF);
	sc_pm_req_low_power_mode(ipc_handle, SC_R_CCI, SC_PM_PW_MODE_OFF);

	/* Request RUN and LP modes for DDR, system interconnect etc. */
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_DDR, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_MU, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_MU, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_OFF);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A53, SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);
	sc_pm_req_sys_if_power_mode(ipc_handle, SC_R_A72, SC_PM_SYS_IF_INTERCONNECT, SC_PM_PW_MODE_ON, SC_PM_PW_MODE_STBY);

	return 0;
}
