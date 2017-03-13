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
 * Header file for the PM RPC implementation.
 *
 * @addtogroup PM_SVC
 * @{
 */

#ifndef _SC_PM_RPC_H
#define _SC_PM_RPC_H

/* Includes */

/* Defines */

/* Types */

/*!
 * This type is used to indicate RPC PM function calls.
 */
typedef enum pm_func_e
{
    PM_FUNC_UNKNOWN, //!< Unknown function
    PM_FUNC_SET_SYS_POWER_MODE, //!< Index for pm_set_sys_power_mode() RPC call
    PM_FUNC_GET_SYS_POWER_MODE, //!< Index for pm_get_sys_power_mode() RPC call
    PM_FUNC_SET_RESOURCE_POWER_MODE, //!< Index for pm_set_resource_power_mode() RPC call
    PM_FUNC_GET_RESOURCE_POWER_MODE, //!< Index for pm_get_resource_power_mode() RPC call
    PM_FUNC_SET_CLOCK_RATE, //!< Index for pm_set_clock_rate() RPC call
    PM_FUNC_GET_CLOCK_RATE, //!< Index for pm_get_clock_rate() RPC call
    PM_FUNC_CLOCK_ENABLE, //!< Index for pm_clock_enable() RPC call
    PM_FUNC_BOOT, //!< Index for pm_boot() RPC call
    PM_FUNC_REBOOT, //!< Index for pm_reboot() RPC call
    PM_FUNC_RESET_REASON, //!< Index for pm_reset_reason() RPC call
    PM_FUNC_CPU_START, //!< Index for pm_cpu_start() RPC call
    PM_FUNC_REBOOT_PARTITION, //!< Index for pm_reboot_partition() RPC call
    PM_FUNC_RESET, //!< Index for pm_reset() RPC call
} pm_func_t;

/* Functions */

/*!
 * This function dispatches an incoming PM RPC request.
 *
 * @param[in]     caller_pt   caller partition
 * @param[in]     msg         pointer to RPC message
 */
void pm_dispatch(sc_rm_pt_t caller_pt, sc_rpc_msg_t *msg);

/*!
 * This function translates and dispatches an PM RPC request.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     msg         pointer to RPC message
 */
void pm_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#endif /* _SC_PM_RPC_H */

/**@}*/

