/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <std_svc.h>
#include <platform_def.h>
#include <sci/sci.h>
#include <runtime_svc.h>
#include <imx_sip.h>

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
	case IMX_SIP_SRTC_SET_TIME:
		ret = imx_srtc_set_time(x2, x3, x4);
		break;
	case IMX_SIP_SRTC_START_WDOG:
		ret = sc_timer_start_wdog(ipc_handle, !!x2);
		break;
	case IMX_SIP_SRTC_STOP_WDOG:
		ret = sc_timer_stop_wdog(ipc_handle);
		break;
	case IMX_SIP_SRTC_SET_WDOG_ACT:
		ret = imx_srtc_set_wdog_action(x2);
		break;
	case IMX_SIP_SRTC_PING_WDOG:
		ret = sc_timer_ping_wdog(ipc_handle);
		break;
	case IMX_SIP_SRTC_SET_TIMEOUT_WDOG:
		ret = sc_timer_set_wdog_timeout(ipc_handle, x2);
		break;
	case IMX_SIP_SRTC_SET_PRETIME_WDOG:
		ret = sc_timer_set_wdog_pre_timeout(ipc_handle, x2);
		break;
	case IMX_SIP_SRTC_GET_WDOG_STAT:
		ret = sc_timer_get_wdog_status(ipc_handle, &timeout,
						&max_timeout, &remaining);
		SMC_RET4(handle, ret, timeout, max_timeout, remaining);
	default:
		ret = SMC_UNK;
	}

	SMC_RET1(handle, ret);
}
