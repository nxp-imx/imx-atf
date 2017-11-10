/*
 * Copyright 2017 NXP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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
void imx_set_cluster_standby(bool pdn);
void imx_set_cluster_powerdown(int last_core, bool pdn);
void imx_set_sys_lpm(bool retention);
void imx_set_rbc_count(void);
void imx_clear_rbc_count(void);

void ddrc_enter_retention(void);
void ddrc_exit_retention(void);

#endif /* __IMX_SOC_H */
