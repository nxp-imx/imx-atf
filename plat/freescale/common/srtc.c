/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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

static int imx_srtc_set_time(uint32_t year_mon, unsigned long day_hour, unsigned long min_sec)
{
	return sc_timer_set_rtc_time(ipc_handle,
		year_mon >> 16, year_mon & 0xffff,
		day_hour >> 16, day_hour & 0xffff,
		min_sec >> 16, min_sec & 0xffff);
}

static int imx_srtc_set_wdog_action(uint32_t x2)
{
	sc_rm_pt_t secure_part;
	sc_err_t err;

	err = sc_rm_get_partition(ipc_handle, &secure_part);
	if (err)
		return err;

	err = sc_timer_set_wdog_action(ipc_handle, secure_part, x2);

	return err;
}

int imx_srtc_handler(uint32_t smc_fid,
		    void *handle,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	int ret;
	sc_timer_wdog_time_t timeout, max_timeout, remaining;

	switch(x1) {
	case FSL_SIP_SRTC_SET_TIME:
		ret = imx_srtc_set_time(x2, x3, x4);
		break;
	case FSL_SIP_SRTC_START_WDOG:
		ret = sc_timer_start_wdog(ipc_handle, !!x2);
		break;
	case FSL_SIP_SRTC_STOP_WDOG:
		ret = sc_timer_stop_wdog(ipc_handle);
		break;
	case FSL_SIP_SRTC_SET_WDOG_ACT:
		ret = imx_srtc_set_wdog_action(x2);
		break;
	case FSL_SIP_SRTC_PING_WDOG:
		ret = sc_timer_ping_wdog(ipc_handle);
		break;
	case FSL_SIP_SRTC_SET_TIMEOUT_WDOG:
		ret = sc_timer_set_wdog_timeout(ipc_handle, x2);
		break;
	case FSL_SIP_SRTC_SET_PRETIME_WDOG:
		ret = sc_timer_set_wdog_pre_timeout(ipc_handle, x2);
		break;
	case FSL_SIP_SRTC_GET_WDOG_STAT:
		ret = sc_timer_get_wdog_status(ipc_handle, &timeout,
						&max_timeout, &remaining);
		SMC_RET4(handle, ret, timeout, max_timeout, remaining);
	default:
		ret = SMC_UNK;
	}

	SMC_RET1(handle, ret);
}
