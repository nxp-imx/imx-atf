/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <smcc_helpers.h>
#include <std_svc.h>
#include <types.h>
#include <mmio.h>
#include <platform_def.h>
#include <fsl_sip.h>
#include <soc.h>
#include <arch_helpers.h>


/* Status definitions */
enum hab_status {
	HAB_STS_ANY = 0x00,
	HAB_FAILURE = 0x33,
	HAB_WARNING = 0x69,
	HAB_SUCCESS = 0xf0
};

/* Security Configuration definitions */
enum hab_config {
	HAB_CFG_RETURN = 0x33,	/* < Field Return IC */
	HAB_CFG_OPEN = 0xf0,	/* < Non-secure IC */
	HAB_CFG_CLOSED = 0xcc	/* < Secure IC */
};

/* State definitions */
enum hab_state {
	HAB_STATE_INITIAL = 0x33,	/* Initialising state (transitory) */
	HAB_STATE_CHECK = 0x55,		/* Check state (non-secure) */
	HAB_STATE_NONSECURE = 0x66,	/* Non-secure state */
	HAB_STATE_TRUSTED = 0x99,	/* Trusted state */
	HAB_STATE_SECURE = 0xaa,	/* Secure state */
	HAB_STATE_FAIL_SOFT = 0xcc, /* Soft fail state */
	HAB_STATE_FAIL_HARD = 0xff, /* Hard fail state (terminal) */
	HAB_STATE_NONE = 0xf0,		/* No security state machine */
	HAB_STATE_MAX
};

typedef enum hab_status hab_rvt_report_event_t(enum hab_status, uint32_t,
		uint8_t* , size_t*);
typedef enum hab_status hab_rvt_report_status_t(enum hab_config *,
		enum hab_state *);
typedef enum hab_status hab_loader_callback_f_t(void**, size_t*, const void*);
typedef enum hab_status hab_rvt_entry_t(void);
typedef enum hab_status hab_rvt_exit_t(void);
typedef void *hab_rvt_authenticate_image_t(uint8_t, long,
		void **, size_t *, hab_loader_callback_f_t);

#define HAB_RVT_BASE			0x00000880 /* HAB_RVT for i.MX8MQ */

#define HAB_RVT_ENTRY_ARM64			((unsigned long)*(uint32_t *)(HAB_RVT_BASE + 0x08))
#define HAB_RVT_EXIT_ARM64			((unsigned long)*(uint32_t *)(HAB_RVT_BASE + 0x10))
#define HAB_RVT_AUTHENTICATE_IMAGE_ARM64	((unsigned long)*(uint32_t *)(HAB_RVT_BASE + 0x20))
#define HAB_RVT_REPORT_EVENT_ARM64		((unsigned long)*(uint32_t *)(HAB_RVT_BASE + 0x40))
#define HAB_RVT_REPORT_STATUS_ARM64		((unsigned long)*(uint32_t *)(HAB_RVT_BASE + 0x48))

#define hab_rvt_authenticate_image_p ((hab_rvt_authenticate_image_t *)HAB_RVT_AUTHENTICATE_IMAGE_ARM64)
#define hab_rvt_entry_p ((hab_rvt_entry_t *)HAB_RVT_ENTRY_ARM64)
#define hab_rvt_exit_p ((hab_rvt_exit_t *)HAB_RVT_EXIT_ARM64)
#define hab_rvt_report_event_p ((hab_rvt_report_event_t *)HAB_RVT_REPORT_EVENT_ARM64)
#define hab_rvt_report_status_p ((hab_rvt_report_status_t *)HAB_RVT_REPORT_STATUS_ARM64)

#define HAB_CID_ATF 2 /**< ATF Caller ID*/

int imx_hab_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
		    u_register_t x3, u_register_t x4)
{
	hab_rvt_authenticate_image_t *hab_rvt_authenticate_image;
	hab_rvt_entry_t *hab_rvt_entry;
	hab_rvt_exit_t *hab_rvt_exit;
	hab_rvt_report_event_t *hab_rvt_report_event;
	hab_rvt_report_status_t *hab_rvt_report_status;

	switch(x1) {
	case FSL_SIP_HAB_AUTHENTICATE:
		hab_rvt_authenticate_image = hab_rvt_authenticate_image_p;
		return (unsigned long)hab_rvt_authenticate_image(HAB_CID_ATF, x2, (void **)x3, (size_t *)x4, NULL);
	case FSL_SIP_HAB_ENTRY:
		hab_rvt_entry = hab_rvt_entry_p;
		return hab_rvt_entry();
	case FSL_SIP_HAB_EXIT:
		hab_rvt_exit = hab_rvt_exit_p;
		return hab_rvt_exit();
	case FSL_SIP_HAB_REPORT_EVENT:
		hab_rvt_report_event = hab_rvt_report_event_p;
		return hab_rvt_report_event(HAB_FAILURE, (uint32_t)x2, (uint8_t *)x3, (size_t *)x4);
	case FSL_SIP_HAB_REPORT_STATUS:
		hab_rvt_report_status = hab_rvt_report_status_p;
		return hab_rvt_report_status((enum hab_config *)x2, (enum hab_state *)x3);
	default:
		return SMC_UNK;

	};

	return 0;
}

