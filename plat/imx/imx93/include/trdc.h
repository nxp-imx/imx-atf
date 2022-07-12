/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX93_TRDC_H
#define IMX93_XRDC_H

#define MBC_BLK_ALL 255
#define MRC_REG_ALL 16

struct trdc_glbac_config {
	uint8_t mbc_mrc_id;
	uint8_t glbac_id;
	uint32_t glbac_val;
};

struct trdc_mbc_config {
	uint8_t mbc_id;
	uint8_t dom_id;
	uint8_t mem_id;
	uint8_t blk_id;
	uint8_t glbac_id;
	bool secure;
};

struct trdc_mrc_config {
	uint8_t mrc_id;
	uint8_t dom_id;
	uint8_t region_id;
	uint32_t region_start;
	uint32_t region_size;
	uint8_t glbac_id;
	bool secure;
};

/* APIs to apply and enable TRDC */
void trdc_config(void);

#endif
