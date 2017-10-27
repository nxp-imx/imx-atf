/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_SOC_H
#define __IMX_SOC_H

enum imx_cpu_pwr_mode {
	WAIT_CLOCKED,			/* wfi only */
	WAIT_UNCLOCKED,			/* WAIT */
	WAIT_UNCLOCKED_POWER_OFF,	/* WAIT + SRPG */
	STOP_POWER_ON,			/* just STOP */
	STOP_POWER_OFF,			/* STOP + SRPG */
};

enum imx_gpc_slot {
	A53_CORE0,
	A53_CORE1,
	A53_CORE2,
	A53_CORE3,
	A53_SCU,
};

enum imx_gpc_pu_slot {
	FAST_MEGA_MIX,
	MIPI_PHY,
	PCIE1_PHY,
	OTG1_PHY,
	OTG2_PHY,
	RESERVED,
	CORE1_M4,
	DDR1_PHY,
	DDR2_PHY,
	GPU,
	VPU,
	HDMI_PHY,
	DSIP,
	MIPI_CSI1,
	MIPI_CSI2,
	PCIE2_PHY,
};

void imx_gpc_set_m_core_pgc(unsigned int cpu, bool pdn);
void imx_gpc_set_lpm_mode(enum imx_cpu_pwr_mode mode);
void imx_gpc_set_cpu_power_gate_by_lpm(unsigned int cpu, bool pdn);
void imx_gpc_set_plat_power_gate_by_lpm(bool pdn);
void imx_gpc_set_core_pdn_pup_by_software(unsigned int cpu, bool pdn);
void imx_gpc_set_cpu_ppower_gate_by_wfi(unsigned int cpu, bool pdn);
void imx_gpc_pre_suspend(bool arm_power_off);
void imx_gpc_post_resume(void);
void imx_gpc_init(void);

void ddrc_enter_retention(void);
void ddrc_exit_retention(void);

void imx_enable_cpu(unsigned int cpu, bool enable);
int imx_is_m4_enabled(void);
void imx_set_cpu_jump_addr(unsigned int cpu, void *jump_addr);
#endif /* __IMX_SOC_H */
