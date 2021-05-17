/*
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef CSU_H
#define CSU_H

#define CSU_SEC_ACCESS_REG_OFFSET	(0x0021C)
/* Bit mask */
#define TZASC_BYPASS_MUX_DISABLE        0x4


enum csu_cslx_access {
	CSU_NS_SUP_R = 0x08,
	CSU_NS_SUP_W = 0x80,
	CSU_NS_SUP_RW = 0x88,
	CSU_NS_USER_R = 0x04,
	CSU_NS_USER_W = 0x40,
	CSU_NS_USER_RW = 0x44,
	CSU_S_SUP_R = 0x02,
	CSU_S_SUP_W = 0x20,
	CSU_S_SUP_RW = 0x22,
	CSU_S_USER_R = 0x01,
	CSU_S_USER_W = 0x10,
	CSU_S_USER_RW = 0x11,
	CSU_ALL_RW = 0xff,
};

struct csu_ns_dev_st {
	uintptr_t ind;
	uint32_t val;
};

void enable_layerscape_ns_access(struct csu_ns_dev_st *csu_ns_dev,
				     uint32_t num, uintptr_t nxp_csu_addr);

#endif /* CSU_H */
