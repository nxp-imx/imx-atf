/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scmi.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <lib/utils_def.h>
#include <common/debug.h>
#include <upower_soc_defs.h>
#include <upower_api.h>

#define POWER_STATE_ON	(0 << 30)
#define POWER_STATE_OFF	(1 << 30)

extern int upower_pwm(int domain_id, int power_state);

struct power_domain {
	char *name;
	uint32_t reg;
	int power_state;
};

enum {
	PS0 = 0,
	PS1 = 1,
	PS2 = 2,
	PS3 = 3,
	PS4 = 4,
	PS5 = 5,
	PS6 = 6,
	PS7 = 7,
	PS8 = 8,
	PS9 = 9,
	PS10 = 10,
	PS11 = 11,
	PS12 = 12,
	PS13 = 13,
	PS14 = 14,
	PS15 = 15,
	PS16 = 16,
	PS17 = 17,
	PS18 = 18,
	PS19 = 19,
};

static struct power_domain scmi_power_domains[] = {
	{ .name = "PS0", .reg = PS0, .power_state = POWER_STATE_OFF },
	{ .name = "PS1", .reg = PS1, .power_state = POWER_STATE_OFF },
	{ .name = "PS2", .reg = PS2, .power_state = POWER_STATE_OFF },
	{ .name = "PS3", .reg = PS3, .power_state = POWER_STATE_OFF },
	{ .name = "PS4", .reg = PS4, .power_state = POWER_STATE_OFF },
	{ .name = "PS5", .reg = PS5, .power_state = POWER_STATE_OFF },
	{ .name = "PS6", .reg = PS6, .power_state = POWER_STATE_OFF },
	{ .name = "PS7", .reg = PS7, .power_state = POWER_STATE_OFF },
	{ .name = "PS8", .reg = PS8, .power_state = POWER_STATE_OFF },
	{ .name = "PS9", .reg = PS9, .power_state = POWER_STATE_OFF },
	{ .name = "PS10", .reg = PS10, .power_state = POWER_STATE_OFF },
	{ .name = "PS11", .reg = PS11, .power_state = POWER_STATE_OFF },
	{ .name = "PS12", .reg = PS12, .power_state = POWER_STATE_OFF },
	{ .name = "PS13", .reg = PS13, .power_state = POWER_STATE_OFF },
	{ .name = "PS14", .reg = PS14, .power_state = POWER_STATE_OFF },
	{ .name = "PS15", .reg = PS15, .power_state = POWER_STATE_OFF },
	{ .name = "PS16", .reg = PS16, .power_state = POWER_STATE_OFF },
	{ .name = "PS17", .reg = PS17, .power_state = POWER_STATE_OFF },
	{ .name = "PS18", .reg = PS18, .power_state = POWER_STATE_OFF },
	{ .name = "PS19", .reg = PS19, .power_state = POWER_STATE_OFF }
};

int scmi_power_domain_handler(uint32_t msg_id, void *shmem)
{
	struct scmi_shared_mem *mem = (struct scmi_shared_mem *)SMC_SHMEM_BASE;
	struct response *response = (struct response *)&mem->payload[0];
	uint32_t domain_id;
	uint32_t *index;
	uint32_t flags;
	uint32_t power_state = 0;
	bool pwr_on;
	int i;
	int ret;

	switch (msg_id) {
	case PROTOCOL_VERSION:
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = 0x20000;
		mem->length = 12;
		break;
	case PROTOCOL_ATTRIBUTES:
		/* TODO */
		response->status = SCMI_RET_SUCCESS;
		response->data[0]= ARRAY_SIZE(scmi_power_domains);
		response->data[1] = -1;
		response->data[2] = -1;
		response->data[3] = 0;
		mem->length = 24;
		break;
	case POWER_DOMAIN_ATTRIBUTES:
		domain_id =  *(uint16_t *)&mem->payload[0];

		for (i = 0; i < ARRAY_SIZE(scmi_power_domains); i++) {
			if (scmi_power_domains[i].reg == domain_id)
				break;
		}

		if (i == ARRAY_SIZE(scmi_power_domains)) {
			response->status = SCMI_RET_NOT_FOUND;
		} else {
			response->status = SCMI_RET_SUCCESS;
			response->data[0] = POWER_DOMAIN_SUPPORT_SYNCHRONOUS;
			memcpy(&response->data[1], scmi_power_domains[domain_id].name, strlen(scmi_power_domains[domain_id].name) + 1);
		}
		mem->length = 28;
		break;
	case POWER_STATE_SET:
		index = (uint32_t *)&mem->payload[0];
		flags =  *index;
		domain_id = *(index + 1);
		power_state = *(index + 2);

		NOTICE("get flags %d domain_id %d power_state %d\n", flags, domain_id, power_state);

		/* TODO: add more details */
		if (power_state & POWER_STATE_OFF) {
			/* OFF */
			pwr_on = false;
		} else {
			/* ON */
			pwr_on = true;
		}
		ret = upower_pwm(domain_id, pwr_on);
		if (ret)
			response->status = ret;

		response->status = SCMI_RET_SUCCESS;
		mem->length = 8;
		break;
	case POWER_STATE_GET:
		domain_id =  *(uint32_t *)&mem->payload[0];

		NOTICE("get domain_id %d power_state %d\n", domain_id, power_state);
		/* TODO: add more details */
		response->status = SCMI_RET_SUCCESS;
		/* TODO: Add lock to protect power_state */
		response->data[0] = scmi_power_domains[domain_id].power_state;
		mem->length = 12;
		break;
	default:
		response->status = SCMI_RET_NOT_SUPPORTED;
		break;
	}

	mem->status = 1;
	return 0;
}
