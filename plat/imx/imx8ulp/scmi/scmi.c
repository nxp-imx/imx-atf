/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <scmi.h>
#include <stdint.h>
#include <string.h>
#include <lib/utils_def.h>
#include <common/debug.h>

/*
 * TODO:
 *  Fine grained lock
 *  Asyn notification
 *  multiple request
 */

uint8_t scmi_protocol[] = {SCMI_PROTOCOL_POWER_DOMAIN, SCMI_PROTOCOL_SENSOR, SCMI_PROTOCOL_PERF_DOMAIN };

const char *vendor = "NXP";
const char *sub_vendor = "IMX";

int scmi_base_protocol_handler(uint32_t msg_id, void *shmem)
{
	struct scmi_shared_mem *mem = (struct scmi_shared_mem *)SMC_SHMEM_BASE;
	struct response *response = (struct response *)&mem->payload[0];
	uint32_t num_skip;
	uint32_t num_protocols;
	uint8_t *index;
	int i;

	switch (msg_id) {
	case PROTOCOL_VERSION:
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = 0x20000;
		mem->length = 12;
		break;
	case PROTOCOL_ATTRIBUTES:
		response->status = SCMI_RET_SUCCESS;
		/* power domain and sensor */
		response->data[0]= ARRAY_SIZE(scmi_protocol);
		mem->length = 12;
		break;
	case BASE_DISCOVER_VENDOR:
		response->status = SCMI_RET_SUCCESS;
		memcpy(&response->data[0], vendor, strlen(vendor) + 1);
		mem->length = 12;
		break;
	case BASE_DISCOVER_SUB_VENDOR:
		response->status = SCMI_RET_SUCCESS;
		memcpy(&response->data[0], sub_vendor, strlen(sub_vendor) + 1);
		mem->length = 12;
		break;
	case BASE_DISCOVER_IMPLEMENTATION_VERSION:
		response->status = SCMI_RET_SUCCESS;
		/* VERSION 1.00 */
		response->data[0]= 0x100;
		mem->length = 12;
		break;
	case BASE_DISCOVER_LIST_PROTOCOLS:
		num_skip = ((uint32_t *)&mem->payload[0])[0];
		num_protocols = ARRAY_SIZE(scmi_protocol);
		if (num_skip > 0)
			response->status = SCMI_RET_NOT_SUPPORTED;
		else
			response->status = SCMI_RET_SUCCESS;
		response->data[0]= num_protocols;
		index = (uint8_t *)&response->data[1];
		for (i = 0; i < num_protocols; i++)
			*index++ = scmi_protocol[i];

		mem->length = 12 + (1 + (num_protocols - 1) / 4) * sizeof(uint32_t);
		break;
	default:
		response->status = SCMI_RET_NOT_SUPPORTED;
		mem->length = 8;
		break;
	}

	mem->status = 1;

	return 0;
}

int scmi_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2, u_register_t x3)
{
	NOTICE("%s %x\n", __func__, smc_fid);

	unsigned int *shmem = (unsigned int *)SMC_SHMEM_BASE;
	struct scmi_shared_mem *mem = (struct scmi_shared_mem *)SMC_SHMEM_BASE;
	struct response *response = (struct response *)&mem->payload[0];
	uint32_t msg_header;
	uint32_t msg_id, msg_pro_id;

	msg_header = mem->header;
	msg_id = MSG_ID(msg_header);
	msg_pro_id = MSG_PRO_ID(msg_header);

	switch(msg_pro_id) {
	case SCMI_PROTOCOL_BASE:
		return scmi_base_protocol_handler(msg_id, shmem);
	case SCMI_PROTOCOL_POWER_DOMAIN:
		return scmi_power_domain_handler(msg_id, shmem);
	case SCMI_PROTOCOL_PERF_DOMAIN:
		return scmi_perf_domain_handler(msg_id, shmem);
	default:
		mem->status = 1;
		response->status = SCMI_RET_NOT_SUPPORTED;
		break;
	}

	return 0;
}
