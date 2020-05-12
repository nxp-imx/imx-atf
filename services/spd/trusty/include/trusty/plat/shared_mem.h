/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <trusty/arm_ffa.h>

/**
 * plat_mem_set_shared - Share or reclaim memory.
 * @mtd:    Structure describing the memory being shared or reclaimed
 * @shared: If %true, the memory is being shared.
 *          If %false, the memory is being reclaimed.
 *
 * Share or reclaim memory. This function is called with a lock held.
 * This function must update the memory_region_attributes field of the
 * struct ffa_mtd if the memory was secured.
 *
 * Return: 0 on success, other values indicates failure.
 */
int plat_mem_set_shared(struct ffa_mtd *mtd, bool shared);

