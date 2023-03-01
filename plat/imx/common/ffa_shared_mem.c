/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <inttypes.h>
#include <platform_def.h>
#include <stddef.h>
#include <trusty/arm_ffa.h>
#include <trusty/ffa_helpers.h>

int plat_mem_set_shared(struct ffa_mtd *mtd, bool shared) {
	if (trusty_ffa_should_be_secure(mtd)) {
		mtd->memory_region_attributes &= ~FFA_MEM_ATTR_NONSECURE;
	}
	return 0;
}
