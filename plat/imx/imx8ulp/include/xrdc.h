/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX8ULP_XRDC_H__
#define __IMX8ULP_XRDC_H__

int xrdc_config_pdac(uint32_t bridge, uint32_t index, uint32_t dom, uint32_t perm);
int xrdc_config_mda(uint32_t mda_con, uint32_t dom);
int xrdc_config_mrc11_hifi_itcm(void);
int xrdc_config_mrc11_hifi_dtcm(void);
int xrdc_config_mrc6_dma2_ddr(void);
int xrdc_config_mrc7_hifi_ddr(void);
#endif
