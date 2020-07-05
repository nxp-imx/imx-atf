/* SPDX-License-Identifier: BSD-3-Clause */
/* +FHDR------------------------------------------------------------------------
 * Copyright (c) 2020 NXP Semiconductor N.V.
 * -----------------------------------------------------------------------------
 * FILE NAME      : upower_soc_defs.h
 * DEPARTMENT     : BSTC - Campinas, Brazil
 * AUTHOR         : Celso Brites
 * AUTHOR'S EMAIL : celso.brites@nxp.com
 * -----------------------------------------------------------------------------
 * RELEASE HISTORY
 * VERSION DATE        AUTHOR                  DESCRIPTION
 *
 * $Log: upower_soc_defs.h.rca $
 * 
 *  Revision: 1.154 Mon Nov  9 14:39:19 2020 nxf42682
 *  powersys_fw_048.011.010.005
 * 
 *  Revision: 1.48 Wed Nov  4 15:38:55 2020 nxa11511
 *  Adds typedef upwr_pmc_reg_t;
 * 
 *  Revision: 1.47 Fri Oct 23 11:49:56 2020 nxa11511
 *  Deleted the GPL license statements, leaving only BSD, as it is compatible with Linux and good for closed ROM/firmware code.
 * 
 *  Revision: 1.46 Thu Sep 24 16:44:46 2020 nxa11511
 *  Reduces UPWR_API_BUFFER_ENDPLUS 64 bytes to give room to diag buffer.
 * 
 *  Revision: 1.39 Tue Sep  1 12:47:49 2020 nxa11511
 *  Adds back GPL-2.0 license, keeping BSD3 (dual licensing).
 * 
 *  Revision: 1.33 Thu Jun 18 11:30:48 2020 nxa11511
 *  RDY2PATCH replaces APD_BOOTED in sic_gpor_t.
 * 
 *  Revision: 1.30 Thu Jun  4 07:57:09 2020 nxa11511
 *  Adds power management parameter bit SLP_ALLOW
 * 
 *  Revision: 1.29 Tue Jun  2 05:55:04 2020 nxf42682
 *  Updated upwr_mon_cfg_union_t bitfields to uint32_t
 * 
 *  Revision: 1.28 Thu May 28 10:50:03 2020 nxa11511
 *  Removed #defines for memory bias min/max voltages.
 * 
 *  Revision: 1.21 Thu May  7 11:38:41 2020 nxf42682
 *  Merge 1.20 with 1.15.1.1
 * 
 *  Revision: 1.20 Wed May  6 12:40:51 2020 nxa11511
 *  Adds #ifdefs for SoC VE compilation.
 * 
 *  Revision: 1.14 Thu Apr 16 15:22:16 2020 nxa08113
 *  Change the position of UPWR_APD_CORES define
 * 
 *  Revision: 1.12 Thu Apr 16 09:54:42 2020 nxa11511
 *  typedefs needed by API users moved from pmc_api.h to upower_soc_defs.h
 * 
 *  Revision: 1.10 Thu Apr  9 19:28:19 2020 nxa10721
 *  Use offsets instead pointers on APD config struct and routines, as it must be
 * 
 *  Revision: 1.9 Thu Apr  9 05:50:36 2020 nxf42682
 *  Returned to v1.7, for 1.8 DID NOT compile for FW releases
 * 
 *  Revision: 1.7 Mon Apr  6 11:27:32 2020 nxa10721
 *  Added AVD PMIC mode msk
 * 
 *  Revision: 1.6 Mon Apr  6 11:11:34 2020 nxa11511
 *  Adds typedef SOC_BOOT_TYPE_T, moved from 8ulp_pmc_hal.h
 * 
 *  Revision: 1.4 Mon Mar 30 22:52:00 2020 nxa10721
 *  Added PMIC controls for AVD domain
 * 
 *  Revision: 1.3 Fri Mar 27 17:18:26 2020 nxa11511
 *  Adds #ifndef guards for the RAM and word count #defines.
 * 
 *  Revision: 1.2 Tue Mar 24 10:51:42 2020 nxa11511
 *  Adds typedef soc_domain_t.
 *  Moves #include "upower_defs.h" to fix compile errors.
 *  Adds Power Mode configuration definitions.
 * 
 * -----------------------------------------------------------------------------
 * KEYWORDS: micro-power uPower driver API
 * -----------------------------------------------------------------------------
 * PURPOSE: SoC-dependent uPower driver API #defines and typedefs shared
 *          with the firmware
 * -----------------------------------------------------------------------------
 * PARAMETERS:
 * PARAM NAME RANGE:DESCRIPTION:       DEFAULTS:                           UNITS
 * -----------------------------------------------------------------------------
 * REUSE ISSUES: no reuse issues
 * -FHDR--------------------------------------------------------------------- */

#ifndef _UPWR_SOC_DEFS_H
#define _UPWR_SOC_DEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef _UPWR_DEFS_H
#error "upower_defs.h or upower_api.h included before upower_soc_defs.h"
#endif

#define UPWR_MU_MSG_SIZE            (2) /* words */

#ifdef   NUM_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS          NUM_PMC_SWT_WORDS
#endif

#ifdef   NUM_PMC_RAM_WORDS
#define UPWR_PMC_MEM_WORDS          NUM_PMC_RAM_WORDS
#endif

#ifndef UPWR_DRAM_SHARED_BASE_ADDR
#define UPWR_DRAM_SHARED_BASE_ADDR      (0x28330000)
#endif

#ifndef UPWR_DRAM_SHARED_SIZE
#define UPWR_DRAM_SHARED_SIZE           (2048)
#endif

#define UPWR_DRAM_SHARED_ENDPLUS        (UPWR_DRAM_SHARED_BASE_ADDR+\
					 UPWR_DRAM_SHARED_SIZE)

#ifndef UPWR_API_BUFFER_BASE
#define UPWR_API_BUFFER_BASE            (0x28330600)
#endif

#ifndef UPWR_API_BUFFER_ENDPLUS
#define UPWR_API_BUFFER_ENDPLUS         (UPWR_DRAM_SHARED_ENDPLUS - 64)
#endif

#ifndef UPWR_PMC_SWT_WORDS
#define UPWR_PMC_SWT_WORDS              (1)
#endif

#ifndef UPWR_PMC_MEM_WORDS
#define UPWR_PMC_MEM_WORDS              (2)
#endif

#include "upower_defs.h"

#ifdef  __cplusplus
#ifndef UPWR_NAMESPACE /* extern "C" 'cancels' the effect of namespace */
extern "C" {
#endif
#endif

#define UPWR_APD_CORES      (2)
#define UPWR_RTD_CORES      (1)

typedef enum {
	RTD_DOMAIN        = 0,
	APD_DOMAIN        = 1,
	UPWR_MAIN_DOMAINS,                           /* RTD, AVD */
	AVD_DOMAIN        = UPWR_MAIN_DOMAINS,
	UPWR_DOMAIN_COUNT,                           /* RTD, APD, AVD */
	PSD_DOMAIN        = UPWR_DOMAIN_COUNT,
	UPWR_ALL_DOMAINS                             /* RTD, APD, AVD, PSD */
} soc_domain_t;

/*=========================================================================
 * UNIT CONVERSION MACROS
 *   These macros convert physical units to the values passed as arguments
 *   in API functions.
 *=========================================================================*/

#define UPWR_VOLT_MILIV(v) (v)        /* voltage in mV    to argument value */
#define UPWR_BIAS_MILIV(v) ((v)/50)   /* bias voltage(mV) to argument value */
#define UPWR_FREQ_KHZ(f)   (f)        /* frequency (kHz)  to argument value */

/*=========================================================================
 * Exception Service Group definitions
 *=========================================================================*/

/* defaults are all zeroes */

typedef union {
	uint32_t R;
	struct {
		uint32_t ALARM_INT   :1; /* 1= use MU GPI1 for alarm interrupt;
					    0= use MU GPI0 for alarm interrupt;
					    this configuration is valid for
					    RTD only
					  */
		uint32_t CFG_IOMUX  : 1; /* 1= tells uPower fw/PMIC driver to 
					       config 8ULP IOMUX for the PMIC
					       I2C and mode pins;
					    0= uPower fw/PMIC must not config
					       8ULP IOMUX, leave it to host
					  */
		uint32_t DGNBUFBITS : 4; /* defines the diagnostic buffer size
					    according to the formula:
					    size = 2^(DGNBUFBITS+3) bytes */
		uint32_t RSV        :26; /* reserved bits: should be all 0s  */
	} B;
} upwr_xcp_config_t;

typedef enum {
	UPWR_ALARM_INTERNAL,             /* internal error */
	UPWR_ALARM_EXCEPTION,            /* core exception */
	UPWR_ALARM_SLACK,                /* delay path too slow */
	UPWR_ALARM_VOLTAGE,              /* voltage drop */
	UPWR_ALARM_LAST = UPWR_ALARM_VOLTAGE
} upwr_alarm_t;

/*=========================================================================
 * Power Management parameters
 *=========================================================================*/

                                     /* values in mV: */

#define UPWR_DOM_RBBN_MAX     (1300) /* max. Domain Reverse Back Bias N-Well */
#define UPWR_DOM_RBBN_MIN      (100) /* min. Domain Reverse Back Bias N-Well */

#define UPWR_DOM_RBBP_MAX     (1300) /* max. Domain Reverse Back Bias P-Well */
#define UPWR_DOM_RBBP_MIN      (100) /* min. Domain Reverse Back Bias P-Well */

typedef enum {               /* bias modes (both domain and memory): */
	NBB_BIAS_MODE  = 0,  /* bias disabled */
	RBB_BIAS_MODE  = 1,  /* reverse back bias enabled */
	AFBB_BIAS_MODE = 2,  /* assimetrical forward bias */
	ARBB_BIAS_MODE = 3   /* assimetrical reverse bias */
} upwr_bias_mode_t;

typedef union {
	uint32_t R;
	struct {
		uint32_t DPD_ALLOW   :1; /* 1= uPower can go Deep Power Down */
		uint32_t DSL_DIS     :1; /* 1= uPower won't go Deep Sleep    */
		uint32_t SLP_ALLOW   :1; /* 1= uPower goes Sleep in the same
					       conditions as Active, and even
					       DSL if DSL_DIS=1  */
		uint32_t DSL_BGAP_OFF:1; /* 1= turn bandgap off when uPower
					       goes Deep Sleep               */
		uint32_t DPD_BGAP_ON :1; /* 1= leave bandgap on when uPower 
                                               goes Deep Power Down          */
		uint32_t RSV        :27; /* reserved bits: should be all 0s  */
	} B;
} upwr_pwm_param_t;

/*=========================================================================
 * Power modes
 *=========================================================================*/

typedef enum {/* from msb->lsb: Azure bit, dual boot bit, low power boot bit */
	SOC_BOOT_SINGLE   = 0,
	SOC_BOOT_LOW_PWR  = 1,
	SOC_BOOT_DUAL     = 2,
	SOC_BOOT_AZURE    = 4
} SOC_BOOT_TYPE_T;

#define GEN_CASE_ENUM_NAME(e) \
  case(e): return (char*)#e

/* Power modes for RTD domain  */
typedef enum {
	DPD_RTD_PWR_MODE, /* Real Time Deep Power Down mode */
	PD_RTD_PWR_MODE,  /* Real Time Power Down mode */
	DSL_RTD_PWR_MODE, /* Real Time Domain Deep Sleep Mode */
	HLD_RTD_PWR_MODE, /* Real Time Domain Hold Mode */
	SLP_RTD_PWR_MODE, /* Sleep Mode */
	ADMA_RTD_PWR_MODE,/* Active DMA Mode */
	ACT_RTD_PWR_MODE, /* Active Domain Mode */
	NUM_RTD_PWR_MODES
} upwr_ps_rtd_pwr_mode_t;

static inline const char* get_rtd_pwr_mode_name(upwr_ps_rtd_pwr_mode_t mode)
{
	switch(mode) {
		GEN_CASE_ENUM_NAME(DPD_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(PD_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(DSL_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(HLD_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(SLP_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(ADMA_RTD_PWR_MODE);
		GEN_CASE_ENUM_NAME(ACT_RTD_PWR_MODE);
		default: return (char*)"WRONG_RTD_PWER_MODE";
	}
}

/* Abstract power modes */
typedef enum {
	DPD_PWR_MODE,
	PD_PWR_MODE,
	PACT_PWR_MODE,
	DSL_PWR_MODE,
	HLD_PWR_MODE,
	SLP_PWR_MODE,
	ADMA_PWR_MODE,
	ACT_PWR_MODE,
	NUM_PWR_MODES,
	NUM_APD_PWR_MODES = NUM_PWR_MODES,
	TRANS_PWR_MODE    = NUM_PWR_MODES,
	INVALID_PWR_MODE  = TRANS_PWR_MODE + 1
} abs_pwr_mode_t;

static inline const char* upwr_dom_name(soc_domain_t dom)
{
	switch (dom)
	{
		case RTD_DOMAIN: return "RTD";
		case APD_DOMAIN: return "APD";
		case AVD_DOMAIN: return "AVD";
		case PSD_DOMAIN: return "PSD";
		default: return "*unknown domain*";
	}
}

typedef struct {
	abs_pwr_mode_t  mode;
	bool            ok;
} pch_trans_t;

typedef pch_trans_t rtd_trans_t;

typedef struct {
	abs_pwr_mode_t  mode;
	pch_trans_t     core[UPWR_APD_CORES];
} apd_trans_t;


/* Get name of a power mode */
static inline char *get_abs_pwr_mode_name(abs_pwr_mode_t mode)
{
	switch(mode) {
		GEN_CASE_ENUM_NAME(DPD_PWR_MODE);
		GEN_CASE_ENUM_NAME(PD_PWR_MODE);
		GEN_CASE_ENUM_NAME(PACT_PWR_MODE);
		GEN_CASE_ENUM_NAME(DSL_PWR_MODE);
		GEN_CASE_ENUM_NAME(HLD_PWR_MODE);
		GEN_CASE_ENUM_NAME(SLP_PWR_MODE);
		GEN_CASE_ENUM_NAME(ADMA_PWR_MODE);
		GEN_CASE_ENUM_NAME(ACT_PWR_MODE);
		default: return (char*)"WRONG_ABS_PWR_MODE";
	}
}


/* Power modes for APD cores. PCH pactive is one-hot w/ these values */
#if 0 // TODO: remove it?
typedef enum {
  PD_CORE_PWR_MODE,
  SLP_CORE_PWR_MODE,
  ADMA_CORE_PWR_MODE,
  ACT_CORE_PWR_MODE,
  NUM_CORE_PWR_MODES
} upwr_core_pwr_mode_t;

static inline const char* get_core_pwr_mode_name(upwr_core_pwr_mode_t mode) {
  switch(mode) {
  GEN_CASE_ENUM_NAME(PD_CORE_PWR_MODE);
  GEN_CASE_ENUM_NAME(SLP_CORE_PWR_MODE);
  GEN_CASE_ENUM_NAME(ADMA_CORE_PWR_MODE);
  GEN_CASE_ENUM_NAME(ACT_CORE_PWR_MODE);
  default: return (char*)"WRONG_CORE_PWR_MODE";
  }
}
#endif

/* Codes for APD pwr mode as programmed in LPMODE reg */
typedef enum {
	ACT_APD_LPM,
	SLP_APD_LPM    = 1,
	DSL_APD_LPM    = 3,
	PACT_APD_LPM   = 7,
	PD_APD_LPM     = 15,
	DPD_APD_LPM    = 31,
	HLD_APD_LPM    = 63
} upwr_apd_lpm_t;

static inline const char *get_apd_pwr_mode_name(upwr_apd_lpm_t mode)
{
	switch(mode) {
		GEN_CASE_ENUM_NAME(ACT_APD_LPM);
		GEN_CASE_ENUM_NAME(SLP_APD_LPM);
		GEN_CASE_ENUM_NAME(DSL_APD_LPM);
		GEN_CASE_ENUM_NAME(PACT_APD_LPM);
		GEN_CASE_ENUM_NAME(PD_APD_LPM);
		GEN_CASE_ENUM_NAME(DPD_APD_LPM);
		GEN_CASE_ENUM_NAME(HLD_APD_LPM);
		default: return (char*)"WRONG_APD_LPM";
	}
}

/*=*************************************************************************
 * RTD
 *=*************************************************************************/

/* Config pmc PADs */

struct upwr_pmc_pad_cfg_t {
	uint32_t pad_close;   /* PMC PAD close config */ 
	uint32_t pad_reset;   /* PMC PAD reset config */
	uint32_t pad_tqsleep; /* PMC PAD TQ Sleep config */
};

/* Regulator ids */

typedef enum {
	RTD_PMC_REG,
	APD_PMC_REG,
	RTD_BIAS_PMC_REG,
	APD_BIAS_PMC_REG,
	RTD_LVD_PMC_MON,
	APD_LVD_PMC_MON,
	AVD_LVD_PMC_MON
} upwr_pmc_reg_t;

/* Config regulator (internal and external) */

struct upwr_reg_cfg_t {
	uint32_t volt;  /* Regulator voltage config */
	uint32_t mode;  /* Regulator mode config */
};

/* Config pmc monitors */

struct  upwr_pmc_mon_cfg_t {
	uint32_t mon_hvd_en; /* PMC mon HVD */
	uint32_t mon_lvd_en; /* PMC mon LVD */
	uint32_t mon_lvdlvl; /* PMC mon LVDLVL */
};

/* Same monitor config for RTD (for compatibility) */

#define upwr_pmc_mon_rtd_cfg_t upwr_pmc_mon_cfg_t

typedef swt_config_t ps_rtd_swt_cfgs_t[NUM_RTD_PWR_MODES];
typedef swt_config_t ps_apd_swt_cfgs_t[NUM_APD_PWR_MODES];

/*=*************************************************************************
 * APD
 *=*************************************************************************/

/* PowerSys PMIC config */
struct upwr_pmic_cfg_t {
	uint32_t                      volt;
	uint32_t                      mode;
	uint32_t                      mode_msk;
};

typedef uint32_t offs_t;

struct ps_apd_pwr_mode_cfg_t {
	#ifdef UPWR_SIMULATOR_ONLY
	struct upwr_switch_board_t*   swt_board_offs;
	struct upwr_mem_switches_t*   swt_mem_offs;
	#else
	offs_t                        swt_board_offs;
	offs_t                        swt_mem_offs;
	#endif
	struct upwr_pmic_cfg_t        pmic_cfg;
	struct upwr_pmc_pad_cfg_t     pad_cfg;
	struct upwr_pmc_bias_cfg_t    bias_cfg;
};

/* Get the pointer to swt config */
static inline struct upwr_switch_board_t*
get_apd_swt_cfg(volatile struct ps_apd_pwr_mode_cfg_t *cfg)
{
	char  *ptr;
	ptr = (char*)cfg;
	ptr += (uint64_t)cfg->swt_board_offs;
	return (struct upwr_switch_board_t*)ptr;
}

/* Get the pointer to mem config */
static inline struct upwr_mem_switches_t*
get_apd_mem_cfg(volatile struct ps_apd_pwr_mode_cfg_t *cfg)
{
	char  *ptr;
	ptr =  (char*)cfg;
	ptr += (uint64_t)cfg->swt_mem_offs;
	return (struct upwr_mem_switches_t*)ptr;
}

/* Power Mode configuration */

#define ps_rtd_pwr_mode_cfg_t upwr_power_mode_cfg_t

/* these typedefs are just for RISC-V sizeof purpose */
typedef uint32_t swt_board_ptr_t;
typedef uint32_t swt_mem_ptr_t;

struct upwr_power_mode_cfg_t {
	#ifdef UPWR_SIMULATOR_ONLY
	struct upwr_switch_board_t*   swt_board; /* Swt board for mem. */
	struct upwr_mem_switches_t*   swt_mem;   /* Swt to mem. arrays, perif */
	#else
	#ifdef __LP64__
	uint32_t                      swt_board;
	uint32_t                      swt_mem;
	#else
	struct upwr_switch_board_t*   swt_board; /* Swt board for mem. */
	struct upwr_mem_switches_t*   swt_mem;   /* Swt to mem. arrays, perif */
	#endif
	#endif
	struct upwr_reg_cfg_t         in_reg_cfg; /* internal regulator config*/
	struct upwr_reg_cfg_t         pmic_cfg;   /* external regulator - pmic*/
	struct upwr_pmc_pad_cfg_t     pad_cfg;  /* Pad conf for power trans*/
	struct upwr_pmc_mon_rtd_cfg_t mon_cfg;    /*monitor configuration */
	struct upwr_pmc_bias_cfg_t    bias_cfg;   /* Memomry/Domain Bias conf */
	struct upwr_powersys_cfg_t    pwrsys_lpm_cfg;/* pwrsys low power config*/
};

static inline int unsigned upwr_sizeof_pmode_cfg(uint32_t domain)
{
	switch (domain)
	{
		case RTD_DOMAIN: return sizeof(struct upwr_power_mode_cfg_t) +
					(sizeof(struct upwr_switch_board_t)*
					 UPWR_PMC_SWT_WORDS) +
					(sizeof(struct upwr_mem_switches_t)*
					 UPWR_PMC_MEM_WORDS) -
					2*(sizeof(void*) - sizeof(swt_board_ptr_t));
		case APD_DOMAIN: return sizeof(struct ps_apd_pwr_mode_cfg_t) +
					(sizeof(struct upwr_switch_board_t)*
					 UPWR_PMC_SWT_WORDS) +
					(sizeof(struct upwr_mem_switches_t)*
					 UPWR_PMC_MEM_WORDS);
	}

	return 0;
}

/*=*************************************************************************
 * SIC
 *=*************************************************************************/

/* SIC GPO according to Integration Guide */
typedef union {
	volatile uint32_t       R;
	struct {
		/* b[0] */
		volatile uint32_t     PMODE         : 7;
		volatile uint32_t     MODECHG       : 1;
		/* b[1] */
		volatile uint32_t     SNTL_RETN     : 1;
		volatile uint32_t     rsrv_1        : 2;
		volatile uint32_t     IRAM_RETN     : 1;
		volatile uint32_t     DRAM_RETN     : 1;
		volatile uint32_t     RTD_KEEP_RST  : 1;
		volatile uint32_t     APD_KEEP_RST  : 1;
		volatile uint32_t     RDY2PATCH     : 1;
		/* b[2] */
		volatile uint32_t     RTD_LLWU      : 1;
		volatile uint32_t     APD_LLWU      : 1;
		volatile uint32_t     rsrv_3        : 1;
		volatile uint32_t     AVD_RST_HOLD  : 1;
		volatile uint32_t     USB0_RETN     : 1;
		volatile uint32_t     MIPI_DSI_ENA  : 1;
		volatile uint32_t     DDR_RETN      : 1;
		volatile uint32_t     PMIC_WAIT_DIS : 1;
		/* b[3] */
		volatile uint32_t     RTD_EARLY_REL : 1;
		volatile uint32_t     RTD_ASYNC_REL : 1;
		volatile uint32_t     RTD_CORE_REL  : 1;
		volatile uint32_t     RTD_RST_HOLD  : 1;
		volatile uint32_t     APD_EARLY_REL : 1;
		volatile uint32_t     APD_ASYNC_REL : 1;
		volatile uint32_t     APD_CORE_REL  : 1;
		volatile uint32_t     APD_RST_HOLD  : 1;
	}               B;
	volatile uint8_t        b[4];
} sic_gpor_t;

/* SIC GPI according to Integration Guide */

/* AVD domain power switches */
#define AVD_PWR_SWITCH_MASK           ((1 <<  7)|\
                                       (1 <<  8)|\
                                       (1 <<  9)|\
                                       (1 << 10)|\
                                       (1 << 11)|\
                                       (1 << 12)|\
                                       (1 << 13)|\
                                       (1 << 14)|\
                                       (1 << 15)|\
                                       (1 << 16))

typedef union {
	volatile uint32_t          R;
	struct {
		/* AVD Slave */
		volatile uint32_t       LPAV_MASTER         : 1;
		volatile uint32_t       LPAV_SAI6           : 1;
		volatile uint32_t       LPAV_SAI7           : 1;
		volatile uint32_t       LPAV_SEMA42         : 1;
		volatile uint32_t       LPAV_LPTMP8         : 1;
		volatile uint32_t       LPAV_SPDIF          : 1;
		volatile uint32_t       rsrv_1              : 2;
		/* AVD Master */
		volatile uint32_t       LPAV_PXP            : 1;
		volatile uint32_t       LPAV_GPU2D          : 1;
		volatile uint32_t       LPAV_GPU3D          : 1;
		volatile uint32_t       LPAV_DCNANO         : 1;
		volatile uint32_t       LPAV_MIPI_DSI       : 1;
		volatile uint32_t       rsrv_2              : 1;
		volatile uint32_t       LPAV_EPDC           : 1;
		volatile uint32_t       LPAV_HIFI4          : 1;
		/* APD LPMODE */
		volatile uint32_t       APD_LPMODE          : 6;
		volatile uint32_t       rsrv_3              : 2;
		/* General */
		volatile uint32_t       rsrv_4              : 4;
		volatile uint32_t       SENT_BUSY           : 1;
		volatile uint32_t       APD_RES_RTD         : 1;
		volatile uint32_t       SENT_ACK            : 1;
		volatile uint32_t       LDOEN               : 1;
	} B;
} sic_gpir_t;

/* Mask the AVD peripherals in sic_gpir_t */
#define AVD_PERIPH_OWNER_MSK          (0xffffUL & ~(0x3UL<<6) & ~(0x1UL<<13))

/*=*************************************************************************
 * PMC
 *=*************************************************************************/

/* Operating modes of devices */
typedef enum {
	OFF_PMC_MODE,
	ON_PMC_MODE,
	LP_PMC_MODE,
	HP_PMC_MODE,
	ENA_PMC_MODE,
	DIS_PMC_MODE
} pmc_dev_mode_t;

/* Monitor Inputs types */
typedef enum {
	RTD_LVD_INP,
	APD_LVD_INP,
	AVD_LVD_INP,
	RTD_HVD_INP,
	APD_HVD_INP,
	AVD_HVD_INP,
	POR_INP,
	LDOEN_INP
} pmc_inp_t;

typedef enum {
	PAD_CLOSE_EVT,
	PAD_RST_EVT,
	PAD_TQSLEEP_EVT
} pmc_pad_evt_t;

/*=*************************************************************************
 * PMIC
 *=*************************************************************************/

#define PMIC_MODE_PINS      4
#define PMIC_MODE_NUM       (1UL<<PMIC_MODE_PINS)

typedef uint8_t             pmic_mode_lvl_t;
typedef pmic_mode_lvl_t     pmic_mode_lvls_t[APD_DOMAIN+1][PMIC_MODE_NUM];

#define MAX_PMIC_MODE_LVL   0xff


/*=*************************************************************************
 * All configs
 *=*************************************************************************/

/* LVD/HVD monitor config for a single domain */

/* Domain + AVD monitor config
 * For RTD, mapped in mon_cfg.mon_hvd_en
 * For APD, mapped temporarily in pad_cfg.pad_tqsleep
 */
typedef union upwr_mon_cfg_union_t {
		volatile uint32_t         R;
		struct {
		/* Original config, not change */
		volatile uint32_t          rsrv_1          : 8;
		/* DOM */
		volatile uint32_t          dom_lvd_irq_ena : 1;
		volatile uint32_t          dom_lvd_rst_ena : 1;
		volatile uint32_t          dom_hvd_irq_ena : 1;
		volatile uint32_t          dom_hvd_rst_ena : 1;
		volatile uint32_t          dom_lvd_lvl     : 4;
		volatile uint32_t          dom_lvd_ena     : 1;
		volatile uint32_t          dom_hvd_ena     : 1;
		/* AVD */
		volatile uint32_t          avd_lvd_irq_ena : 1;
		volatile uint32_t          avd_lvd_rst_ena : 1;
		volatile uint32_t          avd_hvd_irq_ena : 1;
		volatile uint32_t          avd_hvd_rst_ena : 1;
		volatile uint32_t          avd_lvd_lvl     : 4;
		volatile uint32_t          avd_lvd_ena     : 1;
		volatile uint32_t          avd_hvd_ena     : 1;
	}                         B;
} upwr_mon_cfg_t;

/* Get the monitor config word from RAM (domaind and AVD) */ 

static inline uint32_t get_mon_cfg(uint8_t dom, void *mode_cfg)
{
	if (dom == RTD_DOMAIN) {
		return
		  ((struct ps_rtd_pwr_mode_cfg_t*)mode_cfg)->mon_cfg.mon_hvd_en;
	}
	else {
		return
		 ((struct ps_apd_pwr_mode_cfg_t*)mode_cfg)->pad_cfg.pad_tqsleep;
	}
}

/* Set the monitor config word in RAM (domaind and AVD) */ 

static inline void  set_mon_cfg(uint8_t        dom,
				void          *mode_cfg,
				upwr_mon_cfg_t mon_cfg)
{
	uint32_t        *cfg;
	if (dom == RTD_DOMAIN) {
		cfg = (uint32_t*)
		 &((struct ps_rtd_pwr_mode_cfg_t*)mode_cfg)->mon_cfg.mon_hvd_en;
	}
	else {
		cfg = (uint32_t*)
		&((struct ps_apd_pwr_mode_cfg_t*)mode_cfg)->pad_cfg.pad_tqsleep;
	}
	*cfg = mon_cfg.R;
}

/* Uniformize access to PMIC cfg for RTD and APD */ 

typedef union {
	struct upwr_reg_cfg_t     RTD;
	struct upwr_pmic_cfg_t    APD;
} pmic_cfg_t;

/* Access to PMIC mode mask and AVD mode */

typedef union {
	uint32_t                  R;
	struct {
		uint8_t                   mode;     /* Domain PMIC mode */
		uint8_t                   msk;      /* Domain PMIC mode mask */
		uint8_t                   avd_mode; /* AVD PMIC mode */
		uint8_t                   avd_msk;  /* AVD PMIC mode mask */
	}                         B;
} pmic_mode_cfg_t;

/* Access RTD, APD and AVD modes and masks */

static inline pmic_mode_cfg_t *get_pmic_mode_cfg(uint8_t dom, pmic_cfg_t *cfg)
{
	uint32_t        *mode_cfg;

	if (dom == RTD_DOMAIN) mode_cfg = &cfg->RTD.mode;
			  else mode_cfg = &cfg->APD.mode;
	return (pmic_mode_cfg_t*)mode_cfg;
}

static inline uint8_t get_pmic_mode(uint8_t dom, pmic_cfg_t *cfg) 
{
	return get_pmic_mode_cfg(dom, cfg)->B.mode;
}

static inline void set_pmic_mode(uint8_t dom, pmic_cfg_t *cfg, uint8_t mode) 
{
	get_pmic_mode_cfg(dom, cfg)->B.mode = mode;
}

static inline uint8_t get_pmic_mode_msk(uint8_t dom, pmic_cfg_t *cfg) 
{
	pmic_mode_cfg_t   *mode_cfg;

	if (dom == RTD_DOMAIN) {
		mode_cfg = (pmic_mode_cfg_t*)&cfg->RTD.mode;
		return mode_cfg->B.msk;
	}
	else return cfg->APD.mode_msk;
}

static inline void set_pmic_mode_msk(uint8_t dom, pmic_cfg_t *cfg, uint8_t msk) 
{
	pmic_mode_cfg_t   *mode_cfg;

	if (dom == RTD_DOMAIN) {
		mode_cfg = (pmic_mode_cfg_t*)&cfg->RTD.mode;
		mode_cfg->B.msk = msk;
	}
	else cfg->APD.mode_msk = msk;
}

/* Getters and setters for AVD mode and mask */
static inline uint8_t get_avd_pmic_mode(uint8_t dom, pmic_cfg_t *cfg) 
{
	return get_pmic_mode_cfg(dom, cfg)->B.avd_mode;
}

static inline void set_avd_pmic_mode(uint8_t dom, pmic_cfg_t *cfg, uint8_t mode)
{
	get_pmic_mode_cfg(dom, cfg)->B.avd_mode = mode;
}

static inline uint8_t get_avd_pmic_mode_msk(uint8_t dom, pmic_cfg_t *cfg) 
{
	return get_pmic_mode_cfg(dom, cfg)->B.avd_msk;
}

static inline void set_avd_pmic_mode_msk(uint8_t     dom,
					 pmic_cfg_t *cfg,
					 uint8_t     msk) 
{
	get_pmic_mode_cfg(dom, cfg)->B.avd_msk = msk;
}


typedef struct ps_rtd_pwr_mode_cfg_t ps_rtd_pwr_mode_cfgs_t[NUM_RTD_PWR_MODES];
typedef struct ps_apd_pwr_mode_cfg_t ps_apd_pwr_mode_cfgs_t[NUM_APD_PWR_MODES];

struct ps_pwr_mode_cfg_t {
	ps_rtd_pwr_mode_cfgs_t  ps_rtd_pwr_mode_cfg;
	ps_rtd_swt_cfgs_t       ps_rtd_swt_cfg;
	ps_apd_pwr_mode_cfgs_t  ps_apd_pwr_mode_cfg ;
	ps_apd_swt_cfgs_t       ps_apd_swt_cfg;
	pmic_mode_lvls_t        pmic_mode_lvls;
};

#define UPWR_XCP_MIN_ADDR   (0x28350000)
#define UPWR_XCP_MAX_ADDR   (0x2836FFFC)

struct upwr_reg_access_t {
	uint32_t     addr;
	uint32_t     data;
	uint32_t     mask; /* mask=0 commands read */
};

typedef upwr_pointer_msg upwr_xcp_access_msg;

/* unions for the shared memory buffer */

typedef union {
	struct upwr_reg_access_t            reg_access;
} upwr_xcp_union_t;

typedef union {
	struct {
		struct ps_rtd_pwr_mode_cfg_t rtd_struct;
		struct upwr_switch_board_t   rtd_switch;
		struct upwr_mem_switches_t   rtd_memory;
	}                                                rtd_pwr_mode;
	struct {
		struct ps_apd_pwr_mode_cfg_t apd_struct;
		struct upwr_switch_board_t   apd_switch;
		struct upwr_mem_switches_t   apd_memory;
	}                                                apd_pwr_mode;
} upwr_pwm_union_t;


#ifdef  __cplusplus
#ifndef UPWR_NAMESPACE /* extern "C" 'cancels' the effect of namespace */
} /* extern "C" */
#endif
#endif

#endif /* #ifndef _UPWR_SOC_DEFS_H */
