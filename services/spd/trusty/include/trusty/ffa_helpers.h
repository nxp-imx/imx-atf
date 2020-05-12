/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <trusty/arm_ffa.h>

/**
 * trusty_ffa_mtd_get_comp_mrd - get ffa_comp_mrd from ffa_mtd
 * @mtd:    ffa_mtd structure to get the ffa_comp_mrd from
 *
 * Return:  pointer to ffa_comp_mrd
 */
inline struct ffa_comp_mrd *trusty_ffa_mtd_get_comp_mrd(struct ffa_mtd *mtd) {
	return (struct ffa_comp_mrd *)
			((uint8_t *)mtd + mtd->emad[0].comp_mrd_offset);
}

/**
 * trusty_ffa_should_be_secure - indicate whether the memory identified by
 *                               the given ffa_mtd should be secure
 *
 * @mtd:    ffa_mtd structure describing a share or lend request
 *
 * Returns %true if @mtd is a lend request, %false otherwise.
 *
 * Return:  %true if the memory should be secure, %false if not.
 */
inline bool trusty_ffa_should_be_secure(struct ffa_mtd *mtd) {
	return (mtd->flags & FFA_MTD_FLAG_TYPE_MASK) ==
	       FFA_MTD_FLAG_TYPE_LEND_MEMORY;
}
