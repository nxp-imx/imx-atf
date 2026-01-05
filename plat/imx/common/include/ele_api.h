/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ELE_API_H
#define ELE_API_H

#define ELE_SUCCESS_STATUS	(0xD6)

#define ELE_GET_INFO_REQ	U(0x17DA0406)
#define ELE_GET_RNG		U(0x17CD0407)
#define ELE_GET_TRNG_STATE	U(0x17A40106)
#define ELE_PROGRAM_BBSM_REQ	U(0x17BB0406)

/* ELE Program BBSM API operation flags */
#define ELE_PROGRAM_BBSM_OP_READ_REG			(0x6A)
#define ELE_PROGRAM_BBSM_OP_WRITE_REG			(0x71)
#define ELE_PROGRAM_BBSM_OP_SET_BBSM_EVENT_POLICIES	(0xCA)
#define ELE_PROGRAM_BBSM_OP_CLEAR_INTERRUPT		(0xFE)
/* BBSM register offset for reading if any external tamper event reported */
#define BBSM_REG_OFFSET_EXT_TAMPER_ACTIVITY		(0x18)

struct ele_soc_info {
	uint32_t hdr;
	uint32_t soc;
	uint32_t lc;
	uint32_t uid[4];
	uint32_t sha256_rom_patch[8];
	uint32_t sha_fw[8];
	uint32_t oem_srkh[16];
	uint32_t state;
	uint32_t oem_pqc_srkh[16];
	uint32_t reserved[8];
};

int imx9_soc_info_handler(uint32_t smc_fid, void *handle);
void ele_get_soc_info(void);
int ele_get_trng(void* addr, uint32_t len);
int ele_program_bbsm(uint8_t operation, uint16_t policy_mask, uint32_t reg_offset,
		     uint32_t reg_value, uint32_t *resp, uint32_t *ret_reg_value);

#endif /* ELE_API_H */
