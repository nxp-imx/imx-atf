/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <stdbool.h>
#include <plat_imx8.h>
#include <psci.h>
#include <mmio.h>
#include <soc.h>

#define SCR_A53RCR1_OFFSET 0x08
#define SRC_GPR1_OFFSET 0x74
#define ARM_PGC 0x800
#define PGC_PCR 1
#define GPC_CPU_PGC_SW_PUP_REQ 0xf0
#define GPC_CPU_PGC_SW_PDN_REQ 0xfc

extern void imx_gpc_set_core_pdn_pup_by_software(unsigned int cpu, bool pdn);
extern void imx_gpc_set_cpu_power_gate_by_wfi(unsigned int cpu, bool pdn);

const unsigned char imx_power_domain_tree_desc[] = {
	/* number of root nodes */
	PWR_DOMAIN_AT_MAX_LVL,
	/* number of child at the first node */
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
};

void imx8m_kill_cpu(unsigned int target_idx)
{
	unsigned int val1;

	/* Disable the secondary core */
	val1 = mmio_read_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET);
	val1 &= ~(1 << target_idx);
	mmio_write_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET, val1);

	imx_gpc_set_core_pdn_pup_by_software(target_idx, true);
}

int imx_pwr_domain_on(u_register_t mpidr)
{
	int ret = PSCI_E_SUCCESS;
	unsigned int cpu_id, reg;

	uint64_t base_addr = BL31_BASE;

	tf_printf("cpu on\n");
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	/* core power up sequence
	 * 1. Assert nCPUPORESET
	 * 2. power the core.
	 * 3. Deassert reset.
	 */
	reg = mmio_read_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET);
	reg &= ~(1 << cpu_id);
	mmio_write_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET, reg);

	/* Set CPU jump address */
	if (cpu_id > 0) {
		base_addr >>= 2;
		mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (cpu_id << 3),
			((uint32_t)(base_addr >> 22) & 0xFFFF));
		mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + (cpu_id << 3) + 4,
			((uint32_t)base_addr & 0x003FFFFF));
	}

	imx_gpc_set_core_pdn_pup_by_software(cpu_id, false);

	/* Kick CPU here */
	reg = mmio_read_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET);
	reg |= (1 << cpu_id);
	mmio_write_32(IMX_SRC_BASE + SCR_A53RCR1_OFFSET, reg);

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
	plat_gic_cpuif_disable();
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
	uint64_t base_addr = BL31_BASE;
	plat_gic_cpuif_disable();

	/* imx gpc pre suspend */
	imx_gpc_pre_suspend(true);

	base_addr >>= 2;
	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET ,
			((uint32_t)(base_addr >> 22) & 0xFFFF));
	mmio_write_32(IMX_SRC_BASE + SRC_GPR1_OFFSET + 4,
			((uint32_t)base_addr & 0x003FFFFF));

}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	/* imx gpc post resume */
	imx_gpc_post_resume();
	/* enable the GICv3 cpu interface */
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

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	imx_mailbox_init(sec_entrypoint);
	/* sec_entrypoint is used for warm reset */
	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
