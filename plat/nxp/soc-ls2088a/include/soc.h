/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <dcfg_lsch3.h>
#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>

#define NUM_DRAM_REGIONS		2
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */

#define NXP_DRAM1_ADDR			0x8080000000
#define NXP_DRAM1_MAX_SIZE		0x7F80000000	/* 510 G */

/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

#define SVR_WO_E			0xFFFFFE

/*
 * A: without security
 * AE: with security
 */
#define SVR_LS2080A			0x870111
#define SVR_LS2080AE			0x870110
#define SVR_LS2040A			0x870131
#define SVR_LS2040AE			0x870130
#define SVR_LS2088A			0x870901
#define SVR_LS2088AE			0x870900
#define SVR_LS2048A			0x870921
#define SVR_LS2048AE			0x870920
#define SVR_LS2084A			0x870911
#define SVR_LS2084AE			0x870910
#define SVR_LS2044A			0x870931
#define SVR_LS2044AE			0x870930
#define SVR_LS2081A			0x870918
#define SVR_LS2041A			0x870914

/* PORSR1 */
#define PORSR1_RCW_MASK			0xFF800000
#define PORSR1_RCW_SHIFT		23

/* CFG_RCW_SRC[6:0] */
#define RCW_SRC_TYPE_MASK		0x70

/* RCW SRC NOR */
#define	NOR_16B_VAL			0x20

/*
 * RCW SRC Serial Flash
 * 1. SERAIL NOR (QSPI)
 * 2. OTHERS (SD/MMC, SPI, I2C1
 */
#define RCW_SRC_SERIAL_MASK		0x7F
#define QSPI_VAL			0x62
#define SDHC_VAL			0x40
#define EMMC_VAL			0x41


/* Number of cores in platform */
/* Used by common code for array initialization */
#define NUMBER_OF_CLUSTERS		4
#define CORES_PER_CLUSTER		2
#define PLATFORM_CORE_COUNT		(NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER)

/*
 * Required LS standard platform porting definitions
 * for CCN-504 - Read from RN-F node ID register
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP	1, 9, 11, 19
#define PLAT_2CLUSTER_TO_CCN_ID_MAP	1, 9

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	2
#define NXP_UART_CLK_DIVIDER		2

#define MPIDR_AFFINITY0_MASK		0x00FF
#define MPIDR_AFFINITY1_MASK		0xFF00
#define CPUECTLR_DISABLE_TWALK_PREFETCH 0x4000000000
#define CPUECTLR_INS_PREFETCH_MASK	0x1800000000
#define CPUECTLR_DAT_PREFETCH_MASK	0x0300000000
#define OSDLR_EL1_DLK_LOCK		0x1
#define CNTP_CTL_EL0_EN			0x1
#define CNTP_CTL_EL0_IMASK		0x2
/* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS		1

/*
 * pwr mgmt features supported in the soc-specific code:
 *   value == 0x0, the soc code does not support this feature
 *   value != 0x0, the soc code supports this feature
 */
#define SOC_CORE_RELEASE		0x1
#define SOC_CORE_RESTART		0x1
#define SOC_CORE_OFF			0x1
#define SOC_CORE_STANDBY		0x1
#define SOC_CORE_PWR_DWN		0x1
#define SOC_CLUSTER_STANDBY		0x1
#define SOC_CLUSTER_PWR_DWN		0x1
#define SOC_SYSTEM_STANDBY		0x1
#define SOC_SYSTEM_PWR_DWN		0x1
#define SOC_SYSTEM_OFF			0x1
#define SOC_SYSTEM_RESET		0x1

#define SYSTEM_PWR_DOMAINS		1
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					NUMBER_OF_CLUSTERS  + \
					SYSTEM_PWR_DOMAINS)

/* Power state coordination occurs at the system level */
#define PLAT_PD_COORD_LVL		MPIDR_AFFLVL2
#define PLAT_MAX_PWR_LVL		PLAT_PD_COORD_LVL

/* Local power state for power domains in Run state */
#define LS_LOCAL_STATE_RUN		PSCI_LOCAL_STATE_RUN

/* define retention state */
#define PLAT_MAX_RET_STATE		(PSCI_LOCAL_STATE_RUN + 1)
#define LS_LOCAL_STATE_RET		PLAT_MAX_RET_STATE

/* define power-down state */
#define PLAT_MAX_OFF_STATE		(PLAT_MAX_RET_STATE + 1)
#define LS_LOCAL_STATE_OFF		PLAT_MAX_OFF_STATE

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 *
 * CACHE_WRITEBACK_GRANULE is defined in soc.def
 *
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE	(1 * CACHE_WRITEBACK_GRANULE)

#ifndef __ASSEMBLER__

void set_base_freq_CNTFID0(void);
void soc_init_lowlevel(void);
void soc_init_percpu(void);
void _soc_set_start_addr(unsigned long addr);
void _set_platform_security(void);

#endif

#endif /* SOC_H */
