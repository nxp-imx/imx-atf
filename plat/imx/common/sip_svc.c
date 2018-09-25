/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <fsl_sip.h>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <stdint.h>
#include <stdbool.h>
#include <uuid.h>
#include <string.h>
#include <bl_common.h>

extern int imx_gpc_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_cpufreq_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_otp_handler(uint32_t smc_fid, void *handle, u_register_t x1, u_register_t x2);
extern int imx_srtc_handler(uint32_t smc_fid, void *handle, u_register_t x1,
	u_register_t x2, u_register_t x3, u_register_t x4);
extern int lpddr4_dvfs_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_src_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_soc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_hab_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3, u_register_t x4);
extern int imx_noc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int dram_dvfs_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_misc_set_temp_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
				     u_register_t x3, u_register_t x4);

bool wakeup_src_irqsteer = false;

/* Setup i.MX platform specific services Services */
static int32_t plat_svc_setup(void)
{
	/* gpc init ?*/
	NOTICE("sip svc init\n");
	return 0;
}

uint64_t imx_get_commit_hash(u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	/* Parse the version_string */
	char* parse = (char*)version_string;
	uint64_t hash = 0;

	do {
		parse = strchr(parse, '-');
		if (parse) {
			parse += 1;
			if (*(parse) == 'g') {
				/* Default is 7 hexadecimal digits */
				memcpy((void *)&hash, (void *)(parse + 1), 7);
				break;
			}
		}

	} while (parse != NULL);

	return hash;
}

uint64_t imx_buildinfo_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	uint64_t ret;

	switch(x1) {
	case FSL_SIP_BUILDINFO_GET_COMMITHASH:
		ret = imx_get_commit_hash(x2, x3, x4);
		break;
	default:
		return SMC_UNK;
	}

	return ret;
}

int imx_wakeup_src_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch(x1) {
	case FSL_SIP_WAKEUP_SRC_IRQSTEER:
		wakeup_src_irqsteer = true;
		break;
	case FSL_SIP_WAKEUP_SRC_SCU:
		wakeup_src_irqsteer = false;
		break;
	default:
		return SMC_UNK;
	}

	return SMC_OK;
}

/* i.MX platform specific service SMC handler */
uintptr_t imx_svc_smc_handler(uint32_t smc_fid,
			      u_register_t x1,
			      u_register_t x2,
			      u_register_t x3,
			      u_register_t x4,
			      void *cookie,
			      void *handle,
			      uint64_t flags)
{
	switch (smc_fid) {
#if defined(PLAT_IMX8M) || defined(PLAT_IMX8MM)
	case FSL_SIP_DDR_DVFS:
		SMC_RET1(handle, dram_dvfs_handler(smc_fid, x1, x2, x3));
		break;
	case  FSL_SIP_GPC:
		SMC_RET1(handle, imx_gpc_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_SRC:
		SMC_RET1(handle, imx_src_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_GET_SOC_INFO:
		SMC_RET1(handle, imx_soc_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_HAB:
		SMC_RET1(handle, imx_hab_handler(smc_fid, x1, x2, x3, x4));
	case FSL_SIP_NOC:
		SMC_RET1(handle, imx_noc_handler(smc_fid, x1, x2, x3));
		break;
#endif
#if (defined(PLAT_IMX8QM) || defined(PLAT_IMX8QXP))
	case  FSL_SIP_CPUFREQ:
		SMC_RET1(handle, imx_cpufreq_handler(smc_fid, x1, x2, x3));
		break;
	case  FSL_SIP_SRTC:
		return imx_srtc_handler(smc_fid, handle, x1, x2, x3, x4);
	case  FSL_SIP_WAKEUP_SRC:
		SMC_RET1(handle, imx_wakeup_src_handler(smc_fid, x1, x2, x3));
		break;
        case FSL_SIP_OTP_READ:
        case FSL_SIP_OTP_WRITE:
                return imx_otp_handler(smc_fid, handle, x1, x2);
        case FSL_SIP_MISC_SET_TEMP:
		SMC_RET1(handle, imx_misc_set_temp_handler(smc_fid, x1, x2, x3, x4));
		break;
#endif
	case  FSL_SIP_BUILDINFO:
		SMC_RET1(handle, imx_buildinfo_handler(smc_fid, x1, x2, x3, x4));
		break;
	default:
		WARN("Unimplemented SIP Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}
}

/* Rigister SIP Service Calls as runtime service */
DECLARE_RT_SVC(
		imx_svc,
		OEN_SIP_START,
		OEN_SIP_END,
		SMC_TYPE_FAST,
		plat_svc_setup,
		imx_svc_smc_handler
);
