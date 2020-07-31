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

#include <scmi_sensor.h>

#define SCMI_SENSOR_NAME_LEN    16

/* TODO */

#define PLAT_IMX_SENSOR_COUNT	1

struct mod_sensor_info {
    /*! SCMI sensor type */
    uint32_t type;

    /*! Time (in seconds) between sensor updates. Set this field to 0 to
     *  indicate that the sensor does not have a minimum update interval. This
     *  field is used with \ref update_interval_multiplier to calculate the
     *  actual update_interval.
     */
    unsigned int update_interval;

    /*!
     *  Power-of-10 multiplier for \ref update_interval \n\n
     *  This is used to calculate the actual interval time:\n
     *  actual = \ref update_interval x10^(\ref update_interval_multiplier)\n
     */
    int update_interval_multiplier;

    /*!
     *  Power-of-10 multiplier applied to the unit (specified by \ref type)\n\n
     *  Used like this: unit x10^(\ref unit_multiplier)
     */
    int unit_multiplier;
};

enum sensor_type {
	SENSOR_TEMPERATURE_C = 0x2,
	SENSOR_VOLTAGE = 0x5,
	SENSOR_CURRENT = 0x6,
	SENSOR_POWER = 0x7,
	SENSOR_ENERY = 0x8,
};

int sensor_get_info(uint32_t sensor_id, struct mod_sensor_info *sensor_info)
{
	if (!sensor_id) {
		sensor_info->type = SENSOR_TEMPERATURE_C;
		sensor_info->update_interval = 0;
		sensor_info->update_interval_multiplier = 0;
		sensor_info->unit_multiplier = 0;
	}

	return 0;
}

int scmi_sensor_handler(uint32_t msg_id, void *shmem)
{
	struct scmi_shared_mem *mem = (struct scmi_shared_mem *)SMC_SHMEM_BASE;
	struct response *response = (struct response *)&mem->payload[0];
	struct scmi_sensor_protocol_description_get_p2a *return_values;
	uint32_t *index;
	struct mod_sensor_info sensor_info = {0,};
	struct scmi_sensor_desc *desc;
	int desc_index, status;
	int desc_index_max = PLAT_IMX_SENSOR_COUNT;
	uint32_t sensor_id __attribute__ ((unused));
	uint32_t sensor_event_control __attribute__ ((unused));
	uint32_t trip_point_ev_ctrl __attribute__ ((unused));
	uint32_t trip_point_val_low __attribute__ ((unused));
	uint32_t trip_point_val_high __attribute__ ((unused));
	uint32_t flags;


	switch (msg_id) {
	case PROTOCOL_VERSION:
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = 0x10000;
		mem->length = 12;
		break;
	case PROTOCOL_ATTRIBUTES:
		response->status = SCMI_RET_SUCCESS;
		response->data[0] = PLAT_IMX_SENSOR_COUNT;
		response->data[1] = 0;
		response->data[2] = 0;
		response->data[3] = 0;
		mem->length = 24;
		break;
	/*
	case PROTOCOL_MESSAGE_ATTRIBUTES:
		uint32_t message_id;
		index = (uint32_t *)&mem->payload[0];
		message_id = *index;
		response->status = SCMI_RET_NOT_FOUND;
		response->data[0] = 0;
		mem->length = 12;
		break;
	*/
	case SENSOR_DESCRIPTION_GET:
		index = (uint32_t *)&mem->payload[0];
		desc_index = *index;

		if (desc_index != 0)
			asm volatile("b .\r\n");

		return_values = (void *)&mem->payload[0];

		return_values->status = SCMI_RET_SUCCESS;
		return_values->num_sensor_flags = desc_index_max;
		desc = &return_values->sensor_desc[0];

		for (; desc_index < desc_index_max; ++desc_index) {

			desc->sensor_id = desc_index;
			desc->sensor_attributes_low = 0; /* None supported */

			status = sensor_get_info(desc_index, &sensor_info);
			if (status)
				asm volatile("b .\r\n");

			desc->sensor_attributes_high = SCMI_SENSOR_DESC_ATTRIBUTES_HIGH(sensor_info.type,
					sensor_info.unit_multiplier,
					(uint32_t)sensor_info.update_interval_multiplier,
					(uint32_t)sensor_info.update_interval);

			strlcpy(desc->sensor_name, "SOC TEMP", sizeof(desc->sensor_name) - 1);
		}
		mem->length = 12 + sizeof(struct scmi_sensor_desc) * desc_index_max;
		break;

	case SENSOR_CONFIG_SET:
		index = (uint32_t *)&mem->payload[0];
		sensor_id = *index;
		sensor_event_control = *(index + 1);
		/* TODO */

		response->status = SCMI_RET_NOT_SUPPORTED;
		mem->length = 8;
		break;
	case SENSOR_TRIP_POINT_SET:
		index = (uint32_t *)&mem->payload[0];
		sensor_id = *index;
		trip_point_ev_ctrl = *(index + 1);
		trip_point_val_low = *(index + 2);
		trip_point_val_high = *(index + 0x3);

		/* TODO */

		response->status = SCMI_RET_NOT_SUPPORTED;
		mem->length = 8;
		break;
	case SENSOR_READING_GET:
		index = (uint32_t *)&mem->payload[0];
		sensor_id = *index;
		flags = *(index + 1);

		if (flags & BIT_32(0)) {
			/* NOT Support now */
			response->status = SCMI_RET_NOT_SUPPORTED;
			mem->length = 16;
			break;
		}
		response->status = SCMI_RET_SUCCESS;
		/* TODO: Replace with Upower API */
		response->data[0] = 32;
		response->data[1] = 0;
		mem->length = 16;
		break;
	default:
		response->status = SCMI_RET_NOT_SUPPORTED;
		break;
	}

	mem->status = 1;
	return 0;
}
