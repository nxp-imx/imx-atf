/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef POLICY_H
#define	POLICY_H

/*
 * Set this to 0x0 to leave the default SMMU page size in sACR
 * Set this to 0x1 to change the SMMU page size to 64K
 */
#define POLICY_SMMU_PAGESZ_64K 0x1

/*
 * POLICY_PERF_WRIOP = 0 : No Performance enhancement for WRIOP RN-I
 * POLICY_PERF_WRIOP = 1 : No Performance enhancement for WRIOP RN-I = 7
 * POLICY_PERF_WRIOP = 2 : No Performance enhancement for WRIOP RN-I = 23
 */
#define POLICY_PERF_WRIOP 0

/*
 * Set this to '1' if the debug clocks need to remain enabled during
 * system entry to low-power (LPM20) - this should only be necessary
 * for testing and NEVER set for normal production
 */
#define POLICY_DEBUG_ENABLE 0

#endif
