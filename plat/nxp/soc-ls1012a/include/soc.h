/*
 * Copyright 2017-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <dcfg_lsch2.h>
#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>

/* ARM Required MACRO's */
/* Required platform porting definitions */
#define PLAT_PRIMARY_CPU		0x0

/* Number of cores in platform */
#define NUMBER_OF_CLUSTERS		1
#define CORES_PER_CLUSTER		1
#define PLATFORM_CORE_COUNT		(NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER)

 /* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS    	1

/* DDR Regions Info */
#define NUM_DRAM_REGIONS		3
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */
#define	NXP_DRAM1_ADDR			0x880000000
#define NXP_DRAM1_MAX_SIZE		0x780000000	/* 30 GB  */
#define	NXP_DRAM2_ADDR			0x8800000000
#define NXP_DRAM2_MAX_SIZE		0x7800000000	/* 480 GB */
/* DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

/* SVR Definition */
#define SVR_WO_E			0xFFFFFE

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

#define NUM_OF_DDRC			1

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		4

 /*
  * pwr mgmt features supported in the soc-specific code:
  *   value == 0x0  the soc code does not support this feature
  *   value != 0x0  the soc code supports this feature
  */
#undef SOC_CORE_RELEASE
#undef SOC_CORE_RESTART
#undef SOC_CORE_OFF
#undef SOC_CORE_STANDBY
#undef SOC_CORE_PWR_DWN
#undef SOC_CLUSTER_STANDBY
#undef SOC_CLUSTER_PWR_DWN
#undef SOC_SYSTEM_STANDBY
#undef SOC_SYSTEM_PWR_DWN
#undef SOC_SYSTEM_OFF
#undef SOC_SYSTEM_RESET
#undef SOC_SYSTEM_RESET2

#define SOC_CORE_RELEASE		0x0
#define SOC_CORE_RESTART		0x0
#define SOC_CORE_OFF			0x0
#define SOC_CORE_STANDBY		0x1
#define SOC_CORE_PWR_DWN		0x1
#define SOC_CLUSTER_STANDBY		0x1
#define SOC_CLUSTER_PWR_DWN		0x1
#define SOC_SYSTEM_STANDBY		0x1
#define SOC_SYSTEM_PWR_DWN		0x1
#define SOC_SYSTEM_OFF			0x1
#define SOC_SYSTEM_RESET		0x1
#define SOC_SYSTEM_RESET2		0x0

#define SYSTEM_PWR_DOMAINS		1
#define PLAT_NUM_PWR_DOMAINS   (PLATFORM_CORE_COUNT + \
				NUMBER_OF_CLUSTERS  + \
				SYSTEM_PWR_DOMAINS)

 /* Power state coordination occurs at the system level */
#define PLAT_PD_COORD_LVL MPIDR_AFFLVL2
#define PLAT_MAX_PWR_LVL  PLAT_PD_COORD_LVL

/* Local power state for power domains in Run state */
#define LS_LOCAL_STATE_RUN  PSCI_LOCAL_STATE_RUN

/* define retention state */
#define PLAT_MAX_RET_STATE  (PSCI_LOCAL_STATE_RUN + 1)
#define LS_LOCAL_STATE_RET  PLAT_MAX_RET_STATE

/* define power-down state */
#define PLAT_MAX_OFF_STATE  (PLAT_MAX_RET_STATE + 1)
#define LS_LOCAL_STATE_OFF  PLAT_MAX_OFF_STATE

/*
 * One cache line needed for bakery locks on ARM platforms
 * CACHE_WRITEBACK_GRANULE is defined in soc.def
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE (1 * CACHE_WRITEBACK_GRANULE)

#define	OCRAM_REGION_ALL    2

#ifndef __ASSEMBLER__
/* CCI slave interfaces */
static const int cci_map[] = {
	NXP_CCI_CLUSTER0_SL_IFACE_IX,
};

void soc_init_lowlevel(void);
void soc_init_percpu(void);
void _soc_set_start_addr(unsigned long addr);
#endif

#endif /* SOC_H */
