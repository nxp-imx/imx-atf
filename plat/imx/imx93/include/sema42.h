/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SEMA42_H
#define SEMA42_H

#include <lib/mmio.h>

#include <platform_def.h>

static inline void sema42_lock(uint8_t lock_id)
{
	assert(lock_id < 64);

	do {
		mmio_write_8(SEMA42_1_BASE + lock_id, 0x1);
	} while (mmio_read_8(0x44260000) != 0x1);
}

static inline void sema42_unlock(uint8_t lock_id)
{
	mmio_write_8(SEMA42_1_BASE + lock_id, 0x0);
}

#endif
