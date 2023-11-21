/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scmi.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <common/debug.h>
#include <drivers/scmi.h>
#include <lib/utils_def.h>
#include <lib/libc/errno.h>
#include <lib/mmio.h>
#include <upower_soc_defs.h>
#include <upower_api.h>

/* Only Temperature now */
uint16_t plat_scmi_sensor_count(unsigned int agent_id __unused)
{
	return 1U;
}

uint8_t plat_scmi_sensor_max_requests(unsigned int agent_id __unused)
{
	return 1U;
}

extern int upower_read_temperature(uint32_t sensor_id, int32_t *temperature);
int plat_scmi_sensor_reading_get(uint32_t agent_id __unused, uint16_t sensor_id __unused,
				 uint32_t *val)
{
	int32_t temperature;
	int ret;

	ret = upower_read_temperature(1, &temperature);
	if (ret) {
		val[0] = 0xFFFFFFFF;
	} else
		val[0] = temperature;

	val[1] = 0;
	val[2] = 0;
	val[3] = 0;

	return 0;
}

#define SCMI_SENSOR_NAME_LENGTH_MAX	16U

struct scmi_sensor_desc {
	uint32_t id;
	uint32_t attr_low;
	uint32_t attr_high;
	uint8_t name[SCMI_SENSOR_NAME_LENGTH_MAX];
	uint32_t power;
	uint32_t resolution;
	int32_t min_range_low;
	int32_t min_range_high;
	int32_t max_range_low;
	int32_t max_range_high;
};

uint32_t plat_scmi_sensor_state(uint32_t agent_id __unused, uint16_t sensor_id __unused)
{
	return 1U;
}

uint32_t plat_scmi_sensor_description_get(uint32_t agent_id __unused, uint16_t desc_index __unused,
					  struct scmi_sensor_desc *desc __unused)
{
	desc->id = 0;
	desc->attr_low = 0;
	desc->attr_high = 2;
	strlcpy((char *)desc->name, "UPOWER-TEMP", 12);
	desc->power = 0;
	desc->resolution = 0;
	desc->min_range_low = 0;
	desc->min_range_high = 0x80000000;
	desc->max_range_low = 0xffffffff;
	desc->max_range_high = 0x7fffffff;

	return 1U;
}
