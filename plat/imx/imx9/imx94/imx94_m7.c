/*
 * Copyright 2025 NXP
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

#define CPU_RUN_MODE_START		0
#define CPU_RUN_MODE_HOLD		1
#define CPU_RUN_MODE_STOP		2
#define CPU_RUN_MODE_SLEEP		3

#define MCORE_IDX(x)	(((x) < 7 ) ? ((x) - 1) : ((x) - 6))

struct mcore_agents {
	char name[16];
	uint32_t agent_id;
} agents[] = {
	{ .name = "M7", .agent_id = -1 },
	{ .name = "M71", .agent_id = -1 },
	{ .name = "M33S-S", .agent_id = -1 },
};

//static char *mcore_agent_name[] = { "M7", "M71", "M33S-S" };
int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, u_register_t x4, void *handle)
{
	uint32_t run, sleep;
	uint64_t vector;
	int i, ret = 0;
	char name[16];
	uint32_t num_protocols, num_agents;
	uint32_t agent_id_resp = -1;

	if (agents[MCORE_IDX(x3)].agent_id == -1) {
		ret = scmi_base_protocol_attributes(imx9_scmi_handle,
						    &num_protocols,
						    &num_agents);
		if (ret) {
			return ret;
		}

		for (i = 0U; i < num_agents; i++) {
			ret = scmi_base_discover_agent(imx9_scmi_handle, i,
						       &agent_id_resp,
						       name);
			if (ret) {
				continue;
			}

			if (!strcmp(name, agents[MCORE_IDX(x3)].name)) {
				agents[MCORE_IDX(x3)].agent_id = agent_id_resp;
				break;
			}
		}
	}

	switch(x1) {
	case IMX_SIP_SRC_M4_RESET_ADDR_SET:
		ret = scmi_core_set_reset_addr(imx9_scmi_handle, x2,
					       x4,
					       x3);
		if (ret)
			return ret;

		break;
	case IMX_SIP_SRC_M4_START:
		ret = scmi_core_set_reset_addr(imx9_scmi_handle, x2,
					       x3,
					       SCMI_CPU_VEC_FLAGS_BOOT);
		if (ret)
			return ret;

		ret = scmi_core_start(imx9_scmi_handle, x3);
		if (ret)
			return ret;
		break;

	case IMX_SIP_SRC_M4_STARTED:
		ret = scmi_core_info_get(imx9_scmi_handle, x3, &run,
					 &sleep, &vector);

		/* WAIT MODE means M7 is not running */
		if ((run == CPU_RUN_MODE_START) || (run == CPU_RUN_MODE_SLEEP))
			return 1;
		else
			return 0;

	case IMX_SIP_SRC_M4_STOP:
		ret = scmi_core_stop(imx9_scmi_handle, x3);
		if (ret)
			return ret;
		ret = scmi_base_reset_agent_config(imx9_scmi_handle, agents[MCORE_IDX(x3)].agent_id, 0);
		if (ret)
			return ret;
		SMC_SET_GP(handle, CTX_GPREG_X1, 0);
		break;
	default:
		NOTICE("unknown");
		return SMC_UNK;
	};

	return 0;
}
