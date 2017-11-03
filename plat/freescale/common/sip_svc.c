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

#include <debug.h>
#include <fsl_sip.h>
#include <runtime_svc.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <stdint.h>
#include <uuid.h>
#include <string.h>
#include <bl_common.h>

extern int imx_gpc_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_cpufreq_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_srtc_handler(uint32_t smc_fid, u_register_t x1,
	u_register_t x2, u_register_t x3, u_register_t x4);
extern int lpddr4_dvfs_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_src_handler(uint32_t  smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);
extern int imx_soc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3);

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
	NOTICE("smc_fid is %x\n", smc_fid);
	switch (smc_fid) {
#ifdef PLAT_IMX8M
	case  FSL_SIP_GPC:
		SMC_RET1(handle, imx_gpc_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_DDR_DVFS:
		SMC_RET1(handle, lpddr4_dvfs_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_SRC:
		SMC_RET1(handle, imx_src_handler(smc_fid, x1, x2, x3));
		break;
	case FSL_SIP_GET_SOC_INFO:
		SMC_RET1(handle, imx_soc_handler(smc_fid, x1, x2, x3));
		break;
#endif
#if (defined(PLAT_IMX8QM) || defined(PLAT_IMX8QXP))
	case  FSL_SIP_CPUFREQ:
		SMC_RET1(handle, imx_cpufreq_handler(smc_fid, x1, x2, x3));
		break;
	case  FSL_SIP_SRTC:
		SMC_RET1(handle, imx_srtc_handler(smc_fid, x1, x2, x3, x4));
		break;
	case  FSL_SIP_BUILDINFO:
		SMC_RET1(handle, imx_buildinfo_handler(smc_fid, x1, x2, x3, x4));
		break;
#endif
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
