/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#ifndef NXP_COINED_BB
#include <flash_info.h>
#include <fspi.h>
#include <fspi_api.h>
#endif
#include <lib/mmio.h>
#include <plat_nv_storage.h>
#ifdef NXP_COINED_BB
#include <snvs.h>
#endif

/*This structure will be a static structure and
 * will be populated as first step of BL2 booting-up.
 * fspi_strorage.c . To be located in the fspi driver folder.
 */

static nv_app_data_t nv_app_data;

uint8_t read_nv_app_data(void)
{
	int ret = 0;

#ifdef NXP_COINED_BB
	uint8_t *nv_app_data_array = (uint8_t *) &nv_app_data;
	uint8_t offset = 0;

	ret = snvs_read_app_data();
	do {
		nv_app_data_array[offset] = snvs_read_app_data_bit(offset);
		offset++;

	} while (offset < APP_DATA_MAX_OFFSET);
	snvs_clear_app_data();
#else
	uintptr_t nv_base_addr = NV_STORAGE_BASE_ADDR;

	ret = fspi_init(NXP_FLEXSPI_ADDR, NXP_FLEXSPI_FLASH_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}

	xspi_read(nv_base_addr,
		  (uint32_t *)&nv_app_data, sizeof(nv_app_data_t));
	xspi_sector_erase((uint32_t) nv_base_addr,
				F_SECTOR_ERASE_SZ);
#endif
	return ret;
}

uint8_t wr_nv_app_data(int data_offset,
			uint8_t *data,
			int data_size)
{
	int ret = 0;
#ifdef NXP_COINED_BB
#if !TRUSTED_BOARD_BOOT
	snvs_disable_zeroize_lp_gpr();
#endif
	/* In case LP SecMon General purpose register,
	 * only 1 bit flags can be saved.
	 */
	if ((data_size > 1) || (*data != DEFAULT_SET_VALUE)) {
		ERROR("Only binary value is allowed to be written.\n");
		ERROR("Use flash instead of SNVS GPR as NV location.\n");
		return -ENODEV;
	}
	snvs_write_app_data_bit(data_offset);
#else
	uint8_t read_val[sizeof(nv_app_data_t)];
	uint8_t ready_to_write_val[sizeof(nv_app_data_t)];
	uintptr_t nv_base_addr = NV_STORAGE_BASE_ADDR;

	assert((nv_base_addr + data_offset + data_size) > (nv_base_addr + F_SECTOR_ERASE_SZ));

	ret = fspi_init(NXP_FLEXSPI_ADDR, NXP_FLEXSPI_FLASH_ADDR);

	if (ret) {
		ERROR("Failed to initialized driver flexspi-nor.\n");
		ERROR("exiting warm-reset request.\n");
		return -ENODEV;
	}

	ret = xspi_read(nv_base_addr + data_offset, (uint32_t *)read_val, data_size);

	memset(ready_to_write_val, READY_TO_WRITE_VALUE, ARRAY_SIZE(ready_to_write_val));

	if (memcmp(read_val, ready_to_write_val, data_size) == 0)
		xspi_write(nv_base_addr + data_offset, data, data_size);
#endif

	return ret;
}

const nv_app_data_t *get_nv_data(void)
{
	return (const nv_app_data_t *) &nv_app_data;
}
