/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <common/runtime_svc.h>
#include <drivers/arm/css/scmi.h>

#include <imx_scmi_client.h>
#include <scmi_imx9.h>
#include <common/debug.h>
#include <imx_sip_svc.h>

#define IMX9_SCMI_CPU_M7P		1

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	uint32_t run, sleep;
	uint64_t vector;
	int i, ret;
	char name[16];
	char *agent_m7_name = "M7";
	uint32_t num_protocols, num_agents;
	static uint32_t agent_id_resp = -1U;

	if (agent_id_resp == -1U) {
		ret = scmi_base_protocol_attributes(imx9_scmi_handle,
						    &num_protocols,
						    &num_agents);
		if (ret)
			return ret;

		for (i = 0; i < num_agents; i++) {
			ret = scmi_base_discover_agent(imx9_scmi_handle, i,
						       &agent_id_resp,
						       name);
			if (ret)
				continue;
			if (!strcmp(name, agent_m7_name))
				break;
			else
				agent_id_resp = -1U;
		}

		if (i == num_agents)
			agent_id_resp = -1U;
	}

	switch(x1) {
	case IMX_SIP_SRC_M4_START:
		ret = scmi_core_set_reset_addr(imx9_scmi_handle, x2,
					       IMX9_SCMI_CPU_M7P,
					       SCMI_CPU_VEC_FLAGS_BOOT);
		if (ret)
			return ret;

		ret = scmi_core_start(imx9_scmi_handle, IMX9_SCMI_CPU_M7P);
		if (ret)
			return ret;
		break;

	case IMX_SIP_SRC_M4_STARTED:
		ret = scmi_core_info_get(imx9_scmi_handle, IMX9_SCMI_CPU_M7P, &run,
					 &sleep, &vector);
		/* M7 may not allow to be query, report booted up */
		if (ret)
			return 1;

		/* WAIT MODE means M7 is not running */
		if (run == 1)
			return 0;
		else
			return 1;

	case IMX_SIP_SRC_M4_STOP:
		ret = scmi_core_stop(imx9_scmi_handle, IMX9_SCMI_CPU_M7P);
		if (ret)
			return ret;
		ret = scmi_base_reset_agent_config(imx9_scmi_handle, agent_id_resp, 0);
		if (ret)
			return ret;
		SMC_SET_GP(handle, CTX_GPREG_X1, 0);
		break;
	default:
		return SMC_UNK;
	};

	return 0;
}
