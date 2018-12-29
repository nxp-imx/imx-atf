/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_IMX_SIP_H
#define __SOC_IMX_SIP_H

#define IMX_SIP_SRTC			0xC2000002
#define IMX_SIP_SRTC_SET_TIME		0x00
#define IMX_SIP_SRTC_START_WDOG		0x01
#define IMX_SIP_SRTC_STOP_WDOG		0x02
#define IMX_SIP_SRTC_SET_WDOG_ACT	0x03
#define IMX_SIP_SRTC_PING_WDOG		0x04
#define IMX_SIP_SRTC_SET_TIMEOUT_WDOG	0x05
#define IMX_SIP_SRTC_GET_WDOG_STAT	0x06
#define IMX_SIP_SRTC_SET_PRETIME_WDOG	0x07

#if defined(PLAT_IMX8QM) || defined(PLAT_IMX8QX)
int imx_srtc_handler(uint32_t smc_fid, void *handle, u_register_t x1,
	u_register_t x2, u_register_t x3, u_register_t x4);
#endif

#endif
