/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <common/runtime_svc.h>
#include <drivers/arm/css/scmi.h>

#include <imx_scmi_client.h>
#include <common/debug.h>
#include <imx_sip_svc.h>
#include <scmi_imx9.h>

#define IMX95_PD_M7			17
#define LM_ID_M7			1

#define IMX9_SCMI_CPU_M7P		1
#define CPU_RUN_MODE_START		0
#define CPU_RUN_MODE_HOLD		1
#define CPU_RUN_MODE_STOP		2
#define CPU_RUN_MODE_SLEEP		3

int imx_src_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, void *handle)
{
	uint32_t run, sleep;
	uint64_t vector;
	int i, ret;
	char name[SCMI_BASE_DISCOVER_AGENT_RESP_LEN - 8];
	char *agent_m7_name = "M7";
	uint32_t num_protocols, num_agents;
	static uint32_t agent_id_resp = -1U;
	/* num_lm: 2 means mx95alt, 3 means mx95evkrpmsg */
	static uint32_t num_lm = 0U;
	uint32_t state = 1;

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

	if (num_lm == 0U) {
		ret = scmi_lmm_protocol_attributes(imx9_scmi_handle, &num_lm);
		if (ret)
			return ret;
		INFO("num_lm %x\n", num_lm);
	}

	switch(x1) {
	case IMX_SIP_SRC_M4_RESET_ADDR_SET:
		ret = scmi_core_set_reset_addr(imx9_scmi_handle, x2,
					       IMX9_SCMI_CPU_M7P,
					       x3);
		if (ret)
			return ret;

		break;
	case IMX_SIP_SRC_M4_START:
		if (num_lm != 2) {
			INFO("start reset vector to %lx\n", x2);
			ret = scmi_lmm_set_reset_vector(imx9_scmi_handle, LM_ID_M7,
							IMX9_SCMI_CPU_M7P, x2);
			if (ret)
				return ret;

			INFO("reset vector %lx\n", x2);

			ret = scmi_lmm_boot(imx9_scmi_handle, LM_ID_M7);
			if (ret)
				return ret;
			INFO("LM Booted: %d\n", LM_ID_M7);
			break;
		}
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

		/* WAIT MODE means M7 is not running */
		if ((run == CPU_RUN_MODE_START) || (run == CPU_RUN_MODE_SLEEP))
			return 1;
		else
			return 0;

	case IMX_SIP_SRC_M4_STOP:
		if (num_lm != 2) {
			ret = scmi_lmm_shutdown(imx9_scmi_handle, LM_ID_M7,
						IMX9_SCMI_LMM_SHUTDOWN_FLAG_FORCE);
			if (ret)
				return ret;
			INFO("stopped\n");
			break;
		}
		ret = scmi_core_stop(imx9_scmi_handle, IMX9_SCMI_CPU_M7P);
		if (ret)
			return ret;
		ret = scmi_base_reset_agent_config(imx9_scmi_handle, agent_id_resp, 0);
		if (ret)
			return ret;
		SMC_SET_GP(handle, CTX_GPREG_X1, 0);
		break;
	case IMX_SIP_SRC_M4_PREP:
		if (num_lm != 2) {
			ret = scmi_lmm_power_on(imx9_scmi_handle, IMX9_SCMI_CPU_M7P);
			if (ret)
				return ret;
			INFO("prep ready\n");
		};
		break;
	case IMX_SIP_SRC_M4_PREPED:
		ret = scmi_pwr_state_get(imx9_scmi_handle, IMX95_PD_M7,
					 &state);
		if (ret)
			return ret;

		INFO("prepared? state=%d, 0 means on\n", state);
		/* state 0 means on, prepared */
		if (state == 0)
			return 1;
		else
			return 0;
	default:
		return SMC_UNK;
	};

	return 0;
}
