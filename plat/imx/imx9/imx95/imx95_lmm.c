/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/css/scmi.h>
#include <imx_sip_svc.h>
#include <imx_scmi_client.h>
#include <scmi_imx9.h>
#include <common/runtime_svc.h>

int imx_lmm_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	int ret;

	switch (x1) {
	case IMX_SIP_LMM_BOOT:
		ret = scmi_lmm_boot(imx9_scmi_handle, x2);
		if (ret)
			return ret;
		break;
	case IMX_SIP_LMM_SHUTDOWN:
		ret = scmi_lmm_shutdown(imx9_scmi_handle, x2,
					IMX9_SCMI_LMM_SHUTDOWN_FLAG_FORCE);
		if (ret)
			return ret;
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}
