/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPDDR4_DVFS_H
#define __LPDDR4_DVFS_H


void lpddr4_dvfs_swffc(unsigned int init_fsp, unsigned int target_freq);
void lpddr4_dvfs_hwffc(int init_vrcg, int init_fsp, int target_freq,
		int discamdrain);

#endif /* __LPDDR4_DVFS_H */
