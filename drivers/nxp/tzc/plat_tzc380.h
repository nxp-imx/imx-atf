/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if !defined(PLAT_TZC380_H) && defined(IMAGE_BL2)
#define PLAT_TZC380_H

#include <tzc380.h>

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	uint64_t addr;
	uint64_t size;
	unsigned int sub_mask;
};

void mem_access_setup(uintptr_t base, uint32_t total_regions,
			struct tzc380_reg *tzc380_reg_list);

#endif /* PLAT_TZC380_H */
