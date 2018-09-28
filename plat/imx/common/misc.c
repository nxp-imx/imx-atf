/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <types.h>
#include <platform_def.h>
#include <fsl_sip.h>
#include <sci/sci.h>

extern sc_ipc_t ipc_handle;

int imx_misc_set_temp_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	return sc_misc_set_temp(ipc_handle, x1, x2, x3, x4);
}

#if SC_CONSOLE
int putchar(int c)
{
	if (ipc_handle)
		sc_misc_debug_out(ipc_handle, (unsigned char)c);

	return c;
}
#endif
