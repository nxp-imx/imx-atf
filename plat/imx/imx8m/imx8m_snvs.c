/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lib/mmio.h>
#include <platform_def.h>

#define SNVS_HPCOMR             0x04
#define SNVS_NPSWA_EN           (1U << 31)

void enable_snvs_privileged_access(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_SNVS_BASE + SNVS_HPCOMR);
	mmio_write_32(IMX_SNVS_BASE + SNVS_HPCOMR, val | SNVS_NPSWA_EN);
}
