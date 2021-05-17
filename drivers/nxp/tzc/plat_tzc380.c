/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include "plat_tzc380.h"

void mem_access_setup(uintptr_t base, uint32_t total_regions,
			struct tzc380_reg *tzc380_reg_list)
{
	uint32_t indx = 0;
	unsigned int attr_value;

	INFO("Configuring TrustZone Controller tzc380\n");

	tzc380_init(base);

	tzc380_set_action(TZC_ACTION_NONE);

	for (indx = 0; indx < total_regions; indx++) {
		attr_value = tzc380_reg_list[indx].secure |
			TZC_ATTR_SUBREG_DIS(tzc380_reg_list[indx].sub_mask) |
			TZC_ATTR_REGION_SIZE(tzc380_reg_list[indx].size) |
			tzc380_reg_list[indx].enabled;

		tzc380_configure_region(indx, tzc380_reg_list[indx].addr,
				attr_value);
	}

	/*
	 * Raise an exception if a NS device tries to access secure memory
	 * TODO: Add interrupt handling support.
	 */
	tzc380_set_action(TZC_ACTION_ERR);
}
