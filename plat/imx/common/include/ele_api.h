/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ELE_API_H
#define ELE_API_H

#define ELE_GET_INFO_REQ	U(0x17DA0406)

struct ele_soc_info {
	uint32_t hdr;
	uint32_t soc;
	uint32_t lc;
	uint32_t uid[4];
	uint32_t sha256_rom_patch[8];
	uint32_t sha_fw[8];
	uint32_t oem_srkh[16];
	uint32_t state;
};

int imx9_soc_info_handler(uint32_t smc_fid, void *handle);
void ele_get_soc_info(void);

#endif /* ELE_API_H */
