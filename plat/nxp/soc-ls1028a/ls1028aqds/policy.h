/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef POLICY_H
#define	POLICY_H

/*
 * Set this to 0x0 to leave the default SMMU page size in sACR
 * Set this to 0x1 to change the SMMU page size to 64K
 */
#define POLICY_SMMU_PAGESZ_64K	0x1

/*
 * POLICY_PERF_WRIOP = 0 : No Performance enhancement for WRIOP RN-I
 * POLICY_PERF_WRIOP = 1 : No Performance enhancement for WRIOP RN-I = 7
 * POLICY_PERF_WRIOP = 2 : No Performance enhancement for WRIOP RN-I = 23
 */
#define POLICY_PERF_WRIOP	0
#endif
