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
 * Header file for the TIMER RPC implementation.
 *
 * @addtogroup TIMER_SVC
 * @{
 */

#ifndef _SC_TIMER_RPC_H
#define _SC_TIMER_RPC_H

/* Includes */

/* Defines */

/* Types */

/*!
 * This type is used to indicate RPC TIMER function calls.
 */
typedef enum timer_func_e {
	TIMER_FUNC_UNKNOWN = 0,	/* Unknown function */
	TIMER_FUNC_SET_WDOG_TIMEOUT = 1,	/* Index for timer_set_wdog_timeout() RPC call */
	TIMER_FUNC_START_WDOG = 2,	/* Index for timer_start_wdog() RPC call */
	TIMER_FUNC_STOP_WDOG = 3,	/* Index for timer_stop_wdog() RPC call */
	TIMER_FUNC_PING_WDOG = 4,	/* Index for timer_ping_wdog() RPC call */
	TIMER_FUNC_GET_WDOG_STATUS = 5,	/* Index for timer_get_wdog_status() RPC call */
	TIMER_FUNC_SET_WDOG_ACTION = 10,	/* Index for timer_set_wdog_action() RPC call */
	TIMER_FUNC_SET_RTC_TIME = 6,	/* Index for timer_set_rtc_time() RPC call */
	TIMER_FUNC_GET_RTC_TIME = 7,	/* Index for timer_get_rtc_time() RPC call */
	TIMER_FUNC_GET_RTC_SEC1970 = 9,	/* Index for timer_get_rtc_sec1970() RPC call */
	TIMER_FUNC_SET_RTC_ALARM = 8,	/* Index for timer_set_rtc_alarm() RPC call */
} timer_func_t;

/* Functions */

/*!
 * This function dispatches an incoming TIMER RPC request.
 *
 * @param[in]     caller_pt   caller partition
 * @param[in]     msg         pointer to RPC message
 */
void timer_dispatch(sc_rm_pt_t caller_pt, sc_rpc_msg_t *msg);

/*!
 * This function translates and dispatches an TIMER RPC request.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     msg         pointer to RPC message
 */
void timer_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#endif				/* _SC_TIMER_RPC_H */

/**@}*/
