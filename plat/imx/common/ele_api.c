/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <ele_api.h>

#define ELE_MU_RSR	(ELE_MU_BASE + 0x12c)
#define ELE_MU_TRx(i)	(ELE_MU_BASE + 0x200 + (i) * 4)
#define ELE_MU_RRx(i)	(ELE_MU_BASE + 0x280 + (i) * 4)

struct ele_soc_info soc_info;

int imx9_soc_info_handler(uint32_t smc_fid, void *handle)
{
	SMC_RET4(handle, 0x0, soc_info.soc,
		 soc_info.uid[1] | (uint64_t)soc_info.uid[0] << 32,
		 soc_info.uid[3] | (uint64_t)soc_info.uid[2] << 32);
}

void ele_get_soc_info(void)
{
	uint32_t msg, resp;

	flush_dcache_range((uint64_t)&soc_info, sizeof(struct ele_soc_info));

	mmio_write_32(ELE_MU_TRx(0), ELE_GET_INFO_REQ);
	mmio_write_32(ELE_MU_TRx(1), ((uint64_t) &soc_info) >> 32);
	mmio_write_32(ELE_MU_TRx(2), ((uint64_t) &soc_info) & 0xffffffff);
	mmio_write_32(ELE_MU_TRx(3), sizeof(struct ele_soc_info));

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);
}

#define ELE_RELEASE_GMID	0x17e40106
void ele_release_gmid(void)
{
	uint32_t msg, resp;

	mmio_write_32(ELE_MU_TRx(0), ELE_RELEASE_GMID);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	NOTICE("msg : %x, resp: %x\n", msg, resp);
}

#define IMX_ELE_TRNG_STATUS_READY 0x3
#define IMX_ELE_CSAL_STATUS_READY 0x2
static int ele_get_trng_state(void)
{
	uint32_t msg, resp, state;

	mmio_write_32(ELE_MU_TRx(0), ELE_GET_TRNG_STATE);

	do {
		resp = mmio_read_32(ELE_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(ELE_MU_RRx(0));
	resp = mmio_read_32(ELE_MU_RRx(1));
	state = mmio_read_32(ELE_MU_RRx(2));
	VERBOSE("msg : %x, resp: %x\n", msg, resp);

	if (resp != 0xd6 ||
	    ((state >> 0) & 0xff) != IMX_ELE_TRNG_STATUS_READY ||
	    ((state >> 8) & 0xff) != IMX_ELE_CSAL_STATUS_READY)
		return -1;
	return 0;
}

#define ELE_TRNG_MAX_SIZE 16
int ele_get_trng(void* addr, uint32_t len)
{
	uint32_t msg, resp;
	/* Natural Alignment to 64 Bit */
	uint64_t buffer[ELE_TRNG_MAX_SIZE/(sizeof(uint64_t))] = {0};
	uint8_t* current_pos = addr;

	if (addr == NULL || len == 0) {
		return -1;
	}

	if (ele_get_trng_state() != 0)
	{
		NOTICE("TRNG is not ready, EXIT!\n");
		return -1;
	}

	do {
		flush_dcache_range((uint64_t)buffer, ELE_TRNG_MAX_SIZE);

		mmio_write_32(ELE_MU_TRx(0), ELE_GET_RNG);
		mmio_write_32(ELE_MU_TRx(1), 0x2);
		mmio_write_32(ELE_MU_TRx(2), ((uint64_t)buffer) & 0xffffffff);
		mmio_write_32(ELE_MU_TRx(3), ELE_TRNG_MAX_SIZE);

		do {
			resp = mmio_read_32(ELE_MU_RSR);
		} while ((resp & 0x3) != 0x3);

		msg = mmio_read_32(ELE_MU_RRx(0));
		resp = mmio_read_32(ELE_MU_RRx(1));
		VERBOSE("msg : %x, resp: %x\n", msg, resp);

		if (resp != 0xd6)
		{
			NOTICE("TRNG generated failed!\n");
			return -1;
		}

		if (len <= ELE_TRNG_MAX_SIZE) {
			memcpy(current_pos, buffer, len);
			break;
		} else {
			memcpy(current_pos, buffer, ELE_TRNG_MAX_SIZE);
			current_pos += ELE_TRNG_MAX_SIZE;
			len -= ELE_TRNG_MAX_SIZE;
		}

	} while (1);

	return 0;
}

#if defined(PLAT_imx93)
#define ELE_RT_MU_RSR		(ELE_RT_MU_BASE + 0x12c)
#define ELE_RT_MU_TRx(i)	(ELE_RT_MU_BASE + 0x200 + (i) * 4)
#define ELE_RT_MU_RRx(i)	(ELE_RT_MU_BASE + 0x280 + (i) * 4)
/*
 * During Interrupt Handling for Tamper interrupt, need to send this Program
 * BBSM API command for Clear Interrupt operation, as result of SMC call.
 * Already allocated MU(s) (Normal, Trust) cannot be used for this task,
 * as this command may overwrite their existing MU data, hence using RT MU here.
 */
int ele_program_bbsm(uint8_t operation, uint16_t policy_mask, uint32_t reg_offset,
		     uint32_t reg_value, uint32_t *resp, uint32_t *ret_reg_value)
{
	int ret = 0;
	uint32_t msg_op_policy, rsr_status, resp_hdr, resp_code, resp_reg_value;

	msg_op_policy = (policy_mask << 16) | operation;

	mmio_write_32(ELE_RT_MU_TRx(0), ELE_PROGRAM_BBSM_REQ);
	mmio_write_32(ELE_RT_MU_TRx(1), msg_op_policy);
	mmio_write_32(ELE_RT_MU_TRx(2), reg_offset);
	mmio_write_32(ELE_RT_MU_TRx(3), reg_value);

	do {
		rsr_status = mmio_read_32(ELE_RT_MU_RSR);
	} while ((rsr_status & 0x7) != 0x7);

	resp_hdr = mmio_read_32(ELE_RT_MU_RRx(0));
	resp_code = mmio_read_32(ELE_RT_MU_RRx(1));
	resp_reg_value = mmio_read_32(ELE_RT_MU_RRx(2));

	VERBOSE("resp_hdr: %x, resp_code: %x, resp_reg_value %x\n",
		resp_hdr, resp_code, resp_reg_value);

	if (resp_code != ELE_SUCCESS_STATUS) {
		NOTICE("Program BBSM API operation failed!\n");
		ret = -1;
	}

	if (resp)
		*resp = resp_code;

	if (ret_reg_value)
		*ret_reg_value = resp_reg_value;

	return ret;
}
#endif
