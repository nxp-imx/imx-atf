/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "plat_tzc380.h"

#if !defined(TZC380_DEF_REGIONS_H) && defined(IMAGE_BL2)
#define TZC380_DEF_REGIONS_H

/* Default TZC380 Regions Configuration and Regions List */

/*
 * Four Regions:
 *	Region 0: Default region marked as Non-Secure.
 *	Region 1: Secure Region on DRAM 1 for  2MB out of  2MB,
 *			excluding 0 sub-region(=256KB).
 *	Region 2: Secure Region on DRAM 1 for 54MB out of 64MB,
 *			excluding 1 sub-rgion(=8MB) of 8MB
 *	Region 3: Secure Region on DRAM 1 for  6MB out of  8MB,
 *			excluding 2 sub-rgion(=1MB) of 2MB
 *
 *      For TZC-380 MAX_NUM_TZC_REGION will remain = 4.
 *
 * Note: No need to confifure Non-Secure region as it falls in region-0.
 */

#define MAX_NUM_TZC_REGION	4

#define TZC380_REGION_GAURD_SIZE	(66 * 1024 * 1024)

/* Default TZC region list */
static struct tzc380_reg tzc380_default_reg_list[] = {
	{
		TZC_ATTR_SP_NS_RW,		/* .secure attr */
		TZC_ATTR_REGION_DISABLE,	/* disabled */
		UL(0x0),			/* .addr */
		0x0,				/* .size */
		0x0,				/* .submask, all enabled */
	},
	{
		TZC_ATTR_SP_S_RW,		/* .secure attr */
		TZC_ATTR_REGION_ENABLE,		/* .enabled */
		UL(0x0),			/* .addr */
		TZC_REGION_SIZE_2M,		/* .size */
		0x0,				/* .submask, all enabled */
	},
	{
		TZC_ATTR_SP_S_RW,		/* .secure attr */
		TZC_ATTR_REGION_ENABLE,		/* .enabled */
		UL(0x0),			/* .addr */
		TZC_REGION_SIZE_64M,		/* .size */
		0x80,				/* Disable sub-region 7 */
	},
	/* reserve 2M non-scure memory for OPTEE public memory */
	{
		TZC_ATTR_SP_S_RW,		/* .secure attr */
		TZC_ATTR_REGION_ENABLE,		/* .enabled */
		UL(0x0),			/* .addr */
		TZC_REGION_SIZE_8M,		/* .size */
		0xC0,				/* Disable sub-region 6 & 7 */
	},

	{}
};

#endif /* TZC380_DEF_REGIONS_H */
