/*
 * Copyright (c) 2014-2016 Freescale Semiconductor, Inc
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <endian.h>

#include <common/debug.h>
#include <csu.h>
#include <lib/mmio.h>
#include <ns_access.h>

static void enable_devices_ns_access(struct csu_ns_dev_st *csu_ns_dev,
				     uint32_t num, uintptr_t nxp_csu_addr)
{
	uint32_t *base = (uint32_t *)nxp_csu_addr;
	uint32_t *reg;
	uint32_t val;
	int i;

	for (i = 0; i < num; i++) {
		reg = base + csu_ns_dev[i].ind / 2;
		val = be32toh(mmio_read_32((uintptr_t)reg));
		if (csu_ns_dev[i].ind % 2 == 0) {
			val &= 0x0000ffff;
			val |= csu_ns_dev[i].val << 16;
		} else {
			val &= 0xffff0000;
			val |= csu_ns_dev[i].val;
		}
		mmio_write_32((uintptr_t)reg, htobe32(val));
	}
}

void enable_layerscape_ns_access(uintptr_t nxp_csu_addr)
{
	enable_devices_ns_access(ns_dev, ARRAY_SIZE(ns_dev), nxp_csu_addr);
}
