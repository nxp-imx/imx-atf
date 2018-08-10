/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_SOC_H
#define __IMX_SOC_H

#include <stdbool.h>

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
void imx_set_cluster_powerdown(int last_core, uint8_t power_state);
void imx_set_sys_lpm(bool retention);
void imx_set_sys_wakeup(int last_core, bool pdn);
void imx_set_rbc_count(void);
void imx_clear_rbc_count(void);

void noc_wrapper_pre_suspend(unsigned int proc_num);
void noc_wrapper_post_resume(unsigned int proc_num);

void ddrc_enter_retention(void);
void ddrc_exit_retention(void);

bool imx_is_m4_enabled(void);
bool imx_m4_lpa_active(void);

#endif /* __IMX_SOC_H */
