/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scmi.h>
#include <scmi_hal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <lib/utils_def.h>
#include <common/debug.h>

struct __attribute((packed)) scmi_perf_level {
	uint32_t performance_level;
	uint32_t power_cost;
	uint32_t attributes;
};

struct __attribute((packed)) scmi_perf_describe_levels_p2a {
	int32_t status;
	uint32_t num_levels;
	struct scmi_perf_level perf_levels[];
};

/* TODO:
 * currently only use domain_id 0 for A35 DVFS, after SCMI clk ready,
 * need check domain_id, add fine grained lock
 */
int scmi_perf_domain_handler(uint32_t msg_id, void *shmem)
{
	struct scmi_shared_mem *mem = (struct scmi_shared_mem *)SMC_SHMEM_BASE;
	struct response *response = (struct response *)&mem->payload[0];
	struct scmi_perf_describe_levels_p2a *return_values;
	uint32_t domain_id;
	uint32_t *index;
	uint32_t performance_level;
	int ret;

	switch (msg_id) {
	case PROTOCOL_VERSION:
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = 0x20000;
		mem->length = 12;
		break;
	case PROTOCOL_ATTRIBUTES:
		response->status = SCMI_RET_SUCCESS;
		/*
		 * TODO
		 * mW or Scale
		 * statics area
		 * only CPU domain
		 * */
		response->data[0]= BIT_32(16) | 1;
		response->data[1] = 0;
		response->data[2] = 0;
		response->data[3] = 0;
		mem->length = 24;
		break;
	case PERFORMANCE_DOMAIN_ATTRIBUTES:
		index = (uint32_t *)&mem->payload[0];
		domain_id = *index;

		NOTICE("domain_id: %d \n", domain_id);
		if (domain_id) {
			response->status = SCMI_RET_OUT_OF_RANGE;
			break;
		}

		response->status = SCMI_RET_SUCCESS;
		/* TODO: fast channel support */
		//response->data[0] = BIT_32(31) | BIT_32(30) | BIT_32(27);
		response->data[0] = BIT_32(31) | BIT_32(30);
		response->data[1] = 0;
		response->data[2] = 1008000; 
		response->data[3] = 1008000;
		memcpy(&response->data[4], "CPU-DVFS", strlen("CPU-DVFS") + 1);
		mem->length = 40;
		break;
	case PERFORMANCE_DESCRIBE_LEVELS:
		index = (uint32_t *)&mem->payload[0];
		domain_id = *index;
		/* TODO:  we could return in one command, refine to use level_index */
		//level_index = *(index + 1);

		return_values = (void *)&mem->payload[0];

		return_values->status = SCMI_RET_SUCCESS;
		/* TODO: Three performance level, re check this part */
		return_values->num_levels = 3;
		return_values->perf_levels[0].performance_level = 504000;
		return_values->perf_levels[0].power_cost = 800;
		return_values->perf_levels[0].attributes = 15;
		return_values->perf_levels[1].performance_level = 744000;
		return_values->perf_levels[1].power_cost = 900;
		return_values->perf_levels[1].attributes = 15;
		return_values->perf_levels[2].performance_level = 1008000;
		return_values->perf_levels[2].power_cost = 1000;
		return_values->perf_levels[2].attributes = 15;
		mem->length = 48;
		break;
	case PERFORMANCE_LEVEL_SET:
		index = (uint32_t *)&mem->payload[0];
		domain_id = *index;
		/* TODO: Add DVFS scaling, upower voltage */
		/* uint64_t ? */
		performance_level = *(index + 1);
		ret = scmi_set_a35_clk(performance_level);
		if (ret)
			response->status = SCMI_RET_GENERIC_ERROR;
		else
			response->status = SCMI_RET_SUCCESS;
		mem->length = 8;
		break;
	case PERFORMANCE_LEVEL_GET:
		index = (uint32_t *)&mem->payload[0];
		domain_id = *index;
		/* uint64_t ? */
		/* TODO: Add DVFS scaling */
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = scmi_get_a35_clk();
		mem->length = 12;
		break;
	/* TODO */
	case PERFORMANCE_LIMITS_SET:
	case PERFORMANCE_LIMITS_GET:
	default:
		response->status = SCMI_RET_NOT_SUPPORTED;
		break;
	}

	mem->status = 1;
	return 0;
}
