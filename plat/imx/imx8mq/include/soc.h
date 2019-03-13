/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_SOC_H
#define __IMX_SOC_H

void imx_gpc_set_m_core_pgc(unsigned int cpu, bool pdn);
void imx_anamix_pre_suspend(void);
void imx_anamix_post_resume(void);
void imx_gpc_init(void);

void imx_set_cpu_secure_entry(int cpu_id, uintptr_t sec_entrypoint);
void imx_set_cpu_pwr_off(int cpu_id);
void imx_set_cpu_pwr_on(int cpu_id);
void imx_set_cpu_lpm(int cpu_id, bool pdn);
void imx_set_lpm_wakeup(bool pdn);
void imx_set_cluster_standby(bool pdn);
void imx_set_cluster_powerdown(int last_core, bool pdn);
void imx_set_sys_lpm(bool retention);
void imx_set_rbc_count(void);
void imx_clear_rbc_count(void);

void lpddr4_cfg_umctl2(void);
void lpddr4_phy_cfg(void);
void lpddr4_phy_save_trained_csr(void);
void lpddr4_switch_to_3200(void);
void ddrc_enter_retention(void);
void ddrc_exit_retention(void);

#endif /* __IMX_SOC_H */
