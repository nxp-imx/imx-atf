/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <stdint.h>
#include <services/std_svc.h>
#include <string.h>
#include <platform_def.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <imx_sip_svc.h>
#include <sci/sci.h>

#if defined(PLAT_imx8qm) || defined(PLAT_imx8qx) || defined(PLAT_imx8dx) || defined(PLAT_imx8dxl)

#ifdef PLAT_imx8qm
const static int ap_cluster_index[PLATFORM_CLUSTER_COUNT] = {
	SC_R_A53, SC_R_A72,
};
#endif

static int imx_srtc_set_time(uint32_t year_mon,
			unsigned long day_hour,
			unsigned long min_sec)
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

	switch (x1) {
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

static void imx_cpufreq_set_target(uint32_t cluster_id, unsigned long freq)
{
	sc_pm_clock_rate_t rate = (sc_pm_clock_rate_t)freq;

#ifdef PLAT_imx8qm
	sc_pm_set_clock_rate(ipc_handle, ap_cluster_index[cluster_id], SC_PM_CLK_CPU, &rate);
#endif
#if defined(PLAT_imx8qx) || defined(PLAT_imx8dx) || defined(PLAT_imx8dxl)
	sc_pm_set_clock_rate(ipc_handle, SC_R_A35, SC_PM_CLK_CPU, &rate);
#endif
}

int imx_cpufreq_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch (x1) {
	case IMX_SIP_SET_CPUFREQ:
		imx_cpufreq_set_target(x2, x3);
		break;
	default:
		return SMC_UNK;
	}

	return 0;
}

static bool wakeup_src_irqsteer;

bool imx_is_wakeup_src_irqsteer(void)
{
	return wakeup_src_irqsteer;
}

int imx_wakeup_src_handler(uint32_t smc_fid,
		    u_register_t x1,
		    u_register_t x2,
		    u_register_t x3)
{
	switch (x1) {
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
		break;
	default:
		ret = SMC_UNK;
		SMC_RET1(handle, ret);
		break;
	}

	return ret;
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
#endif /* defined(PLAT_imx8qm) || defined(PLAT_imx8qx) || defined(PLAT_imx8dx) || defined(PLAT_imx8qm) || defined(PLAT_imx8dxl) */

static uint64_t imx_get_commit_hash(u_register_t x2,
		    u_register_t x3,
		    u_register_t x4)
{
	/* Parse the version_string */
	char *parse = (char *)version_string;
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

	switch (x1) {
	case IMX_SIP_BUILDINFO_GET_COMMITHASH:
		ret = imx_get_commit_hash(x2, x3, x4);
		break;
	default:
		return SMC_UNK;
	}

	return ret;
}

#if SC_CONSOLE
int putchar(int c)
{
	if (ipc_handle)
		sc_misc_debug_out(ipc_handle, (unsigned char)c);

	return c;
}
#endif
