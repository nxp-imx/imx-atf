/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_IMX_SIP_H
#define __SOC_IMX_SIP_H

#define FSL_SIP_GPC			0xC2000000
#define FSL_SIP_CONFIG_GPC_MASK		0x00
#define FSL_SIP_CONFIG_GPC_UNMASK	0x01
#define FSL_SIP_CONFIG_GPC_SET_WAKE	0x02
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x03
#define FSL_SIP_CONFIG_GPC_SET_AFF	0x04
#define FSL_SIP_CONFIG_GPC_CORE_WAKE	0x05

#define IMX_SIP_CPUFREQ			0xC2000001
#define IMX_SIP_SET_CPUFREQ		0x00

#define IMX_SIP_SRTC			0xC2000002
#define IMX_SIP_SRTC_SET_TIME		0x00
#define IMX_SIP_SRTC_START_WDOG		0x01
#define IMX_SIP_SRTC_STOP_WDOG		0x02
#define IMX_SIP_SRTC_SET_WDOG_ACT	0x03
#define IMX_SIP_SRTC_PING_WDOG		0x04
#define IMX_SIP_SRTC_SET_TIMEOUT_WDOG	0x05
#define IMX_SIP_SRTC_GET_WDOG_STAT	0x06
#define IMX_SIP_SRTC_SET_PRETIME_WDOG	0x07

#define IMX_SIP_BUILDINFO			0xC2000003
#define IMX_SIP_BUILDINFO_GET_COMMITHASH	0x00

#define IMX_SIP_DDR_DVFS		0xc2000004
#define IMX_SIP_DDR_DVFS_WAIT_CHANGE	0x0F
#define IMX_SIP_DDR_DVFS_GET_FREQ_COUNT	0x10
#define IMX_SIP_DDR_DVFS_GET_FREQ_INFO	0x11

#define FSL_SIP_SRC			0xc2000005
#define FSL_SIP_SRC_M4_START		0x00
#define FSL_SIP_SRC_M4_STARTED		0x01

#define FSL_SIP_GET_SOC_INFO            0xc2000006

#define FSL_SIP_HAB            0xc2000007
#define FSL_SIP_HAB_AUTHENTICATE	0x00
#define FSL_SIP_HAB_ENTRY			0x01
#define FSL_SIP_HAB_EXIT			0x02
#define FSL_SIP_HAB_REPORT_EVENT	0x03
#define FSL_SIP_HAB_REPORT_STATUS	0x04
#define FSL_SIP_HAB_FAILSAFE		0x05
#define FSL_SIP_HAB_CHECK_TARGET	0x06

#define FSL_SIP_NOC			0xc2000008
#define FSL_SIP_NOC_LCDIF		0x0
#define FSL_SIP_NOC_PRIORITY		0x1
#define NOC_GPU_PRIORITY		0x10
#define NOC_DCSS_PRIORITY		0x11
#define NOC_VPU_PRIORITY		0x12
#define NOC_CPU_PRIORITY		0x13
#define NOC_MIX_PRIORITY		0x14


#define IMX_SIP_WAKEUP_SRC		0xc2000009
#define IMX_SIP_WAKEUP_SRC_SCU		0x1
#define IMX_SIP_WAKEUP_SRC_IRQSTEER	0x2

#define IMX_SIP_OTP_READ		0xc200000A
#define IMX_SIP_OTP_WRITE		0xc200000B

#define IMX_SIP_MISC_SET_TEMP		0xc200000c

#if defined(PLAT_IMX8QM) || defined(PLAT_IMX8QX)
int imx_cpufreq_handler(uint32_t smc_fid, u_register_t x1,
			u_register_t x2, u_register_t x3);
int imx_srtc_handler(uint32_t smc_fid, void *handle, u_register_t x1,
	u_register_t x2, u_register_t x3, u_register_t x4);
int imx_wakeup_src_handler(uint32_t smc_fid, u_register_t x1,
			u_register_t x2, u_register_t x3);
int imx_otp_handler(uint32_t smc_fid, void *handle,
			u_register_t x1, u_register_t x2);
int imx_misc_set_temp_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3,
				u_register_t x4);
#endif

#if defined(PLAT_IMX8M) || defined(PLAT_IMX8MM) || defined(PLAT_IMX8MN)
int imx_gpc_handler(uint32_t  smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3);
int lpddr4_dvfs_handler(uint32_t  smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3);
int imx_src_handler(uint32_t  smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3);
int imx_soc_handler(uint32_t smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3);
int imx_hab_handler(uint32_t smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3, u_register_t x4);
int imx_noc_handler(uint32_t smc_fid, u_register_t x1,
 		u_register_t x2, u_register_t x3);
int dram_dvfs_handler(uint32_t smc_fid, void *handle,
	u_register_t x1, u_register_t x2, u_register_t x3);
#endif

uint64_t imx_buildinfo_handler(uint32_t smc_fid, u_register_t x1,
				u_register_t x2, u_register_t x3,
				u_register_t x4);
#endif
