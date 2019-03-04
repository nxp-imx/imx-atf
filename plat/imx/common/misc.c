/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <std_svc.h>
#include <stdbool.h>
#include <string.h>
#include <uuid.h>
#include <bl_common.h>
#include <platform_def.h>
#include <sci/sci.h>
#include <runtime_svc.h>
#include <imx_sip.h>

#if defined(PLAT_IMX8QM) || defined(PLAT_IMX8QX)
static bool wakeup_src_irqsteer;

bool imx_is_wakeup_src_irqsteer(void)
{
	return wakeup_src_irqsteer;
}

#if SC_CONSOLE
int putchar(int c)
{
	if (ipc_handle)
		sc_misc_debug_out(ipc_handle, (unsigned char)c);

	return c;
}
#endif

int imx_wakeup_src_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch(x1) {
	case IMX_SIP_WAKEUP_SRC_IRQSTEER:
		wakeup_src_irqsteer = true;
		break;
	case IMX_SIP_WAKEUP_SRC_SCU:
		wakeup_src_irqsteer = false;
		break;
	default:
		return SMC_UNK;
	}

	return SMC_OK;
}

int imx_otp_handler(uint32_t smc_fid,
                    void *handle,
                    u_register_t x1,
                    u_register_t x2)
{
        int ret;
        uint32_t fuse;

        switch (smc_fid) {
            case IMX_SIP_OTP_READ:
                ret = sc_misc_otp_fuse_read(ipc_handle, x1, &fuse);
                SMC_RET2(handle, ret, fuse);
                break;
            case IMX_SIP_OTP_WRITE:
                ret = sc_misc_otp_fuse_write(ipc_handle, x1, x2);
                SMC_RET1(handle, ret);
            default:
                ret = SMC_UNK;
                SMC_RET1(handle, ret);
        }
}

int imx_misc_set_temp_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	return sc_misc_set_temp(ipc_handle, x1, x2, x3, x4);
}

int imx_get_cpu_rev(uint32_t *cpu_id, uint32_t *cpu_rev)
{
	uint32_t id;
	sc_err_t err;

	if (!cpu_id || !cpu_rev)
		return -1;

	err = sc_misc_get_control(ipc_handle, SC_R_SYSTEM, SC_C_ID, &id);
	if (err != SC_ERR_NONE)
		return err;

	*cpu_rev = (id >> 5)  & 0xf;
	*cpu_id = id & 0x1f;

	return 0;
}
#endif

static uint64_t imx_get_commit_hash(u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	/* Parse the version_string */
	char* parse = (char*)version_string;
	uint64_t hash = 0;

	do {
		parse = strchr(parse, '-');
		if (parse) {
			parse += 1;
			if (*(parse) == 'g') {
				/* Default is 7 hexadecimal digits */
				memcpy((void *)&hash, (void *)(parse + 1), 7);
				break;
			}
		}

	} while (parse != NULL);

	return hash;
}

uint64_t imx_buildinfo_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	uint64_t ret;

	switch(x1) {
	case IMX_SIP_BUILDINFO_GET_COMMITHASH:
		ret = imx_get_commit_hash(x2, x3, x4);
		break;
	default:
		return SMC_UNK;
	}

	return ret;
}
