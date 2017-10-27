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

/*!
 * File containing client-side RPC functions for the TIMER service. These
 * functions are ported to clients that communicate to the SC.
 *
 * @addtogroup TIMER_SVC
 * @{
 */

/* Includes */

#include <sci/types.h>
#include <sci/svc/rm/api.h>
#include <sci/svc/pm/api.h>
#include <sci/rpc.h>
#include <stdlib.h>
#include "rpc.h"

/* Local Defines */

/* Local Types */
typedef uint8_t sc_timer_wdog_action_t;
typedef uint32_t sc_timer_wdog_time_t;
/* Local Functions */

sc_err_t sc_timer_set_rtc_time(sc_ipc_t ipc, uint16_t year, uint8_t mon,
			       uint8_t day, uint8_t hour, uint8_t min,
			       uint8_t sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_SET_RTC_TIME;
	RPC_U16(&msg, 0) = year;
	RPC_U8(&msg, 2) = mon;
	RPC_U8(&msg, 3) = day;
	RPC_U8(&msg, 4) = hour;
	RPC_U8(&msg, 5) = min;
	RPC_U8(&msg, 6) = sec;
	RPC_SIZE(&msg) = 3;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_get_rtc_time(sc_ipc_t ipc, uint16_t *year, uint8_t *mon,
			       uint8_t *day, uint8_t *hour, uint8_t *min,
			       uint8_t *sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_GET_RTC_TIME;
	RPC_SIZE(&msg) = 1;

	sc_call_rpc(ipc, &msg, false);

	if (year != NULL)
		*year = RPC_U16(&msg, 0);
	result = RPC_R8(&msg);
	if (mon != NULL)
		*mon = RPC_U8(&msg, 2);
	if (day != NULL)
		*day = RPC_U8(&msg, 3);
	if (hour != NULL)
		*hour = RPC_U8(&msg, 4);
	if (min != NULL)
		*min = RPC_U8(&msg, 5);
	if (sec != NULL)
		*sec = RPC_U8(&msg, 6);
	return (sc_err_t)result;
}

sc_err_t sc_timer_get_rtc_sec1970(sc_ipc_t ipc, uint32_t *sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_GET_RTC_SEC1970;
	RPC_SIZE(&msg) = 1;

	sc_call_rpc(ipc, &msg, false);

	if (sec != NULL)
		*sec = RPC_U32(&msg, 0);
	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_rtc_alarm(sc_ipc_t ipc, uint16_t year, uint8_t mon,
				uint8_t day, uint8_t hour, uint8_t min,
				uint8_t sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_SET_RTC_ALARM;
	RPC_U16(&msg, 0) = year;
	RPC_U8(&msg, 2) = mon;
	RPC_U8(&msg, 3) = day;
	RPC_U8(&msg, 4) = hour;
	RPC_U8(&msg, 5) = min;
	RPC_U8(&msg, 6) = sec;
	RPC_SIZE(&msg) = 3;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_start_wdog(sc_ipc_t ipc, bool lock)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_START_WDOG;
	RPC_U8(&msg, 0) = lock;
	RPC_SIZE(&msg) = 2;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_stop_wdog(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_STOP_WDOG;
	RPC_SIZE(&msg) = 1;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_wdog_action(sc_ipc_t ipc,
				  sc_rm_pt_t pt, sc_timer_wdog_action_t action)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_SET_WDOG_ACTION;
	RPC_U8(&msg, 0) = pt;
	RPC_U8(&msg, 1) = action;
	RPC_SIZE(&msg) = 2;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_wdog_timeout(sc_ipc_t ipc, sc_timer_wdog_time_t timeout)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_SET_WDOG_TIMEOUT;
	RPC_U32(&msg, 0) = timeout;
	RPC_SIZE(&msg) = 2;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_ping_wdog(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = TIMER_FUNC_PING_WDOG;
	RPC_SIZE(&msg) = 1;

	sc_call_rpc(ipc, &msg, false);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}
/**@}*/
