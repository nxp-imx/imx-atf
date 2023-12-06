/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

#define PLATFORM_STACK_SIZE		0X400
#define CACHE_WRITEBACK_GRANULE		64

#define PLAT_PRIMARY_CPU		U(0x0)

#define PLATFORM_CLUSTER0_CORE_COUNT	U(4)
#define PLATFORM_CLUSTER1_CORE_COUNT	U(2)

#ifdef COCKPIT_A72

#define PLATFORM_MAX_CPU_PER_CLUSTER	U(2)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define COCKPIT_CLUSTER_CORE_COUNT	U(2)
#define CLUSTER0_CCI_SLAVE_IFACE	U(4)
#define CLUSTER1_CCI_SLAVE_IFACE	U(0)

#define PLATFORM_CORE_COUNT		U(2)

#elif (defined COCKPIT_A53)

#define PLATFORM_MAX_CPU_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define COCKPIT_CLUSTER_CORE_COUNT	U(4)
#define CLUSTER0_CCI_SLAVE_IFACE	U(3)
#define CLUSTER1_CCI_SLAVE_IFACE	U(0)

#define PLATFORM_CORE_COUNT		U(4)

#else	/* !(defined COCKPIT_A53) */

#define PLATFORM_MAX_CPU_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(2)
#define CLUSTER0_CCI_SLAVE_IFACE	U(3)
#define CLUSTER1_CCI_SLAVE_IFACE	U(4)

#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT + \
					 PLATFORM_CLUSTER1_CORE_COUNT)
#endif	/* !(defined COCKPIT_A53) */


#define IMX_PWR_LVL0			MPIDR_AFFLVL0
#define IMX_PWR_LVL1			MPIDR_AFFLVL1
#define IMX_PWR_LVL2			MPIDR_AFFLVL2

#define PWR_DOMAIN_AT_MAX_LVL		U(1)
#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_RET_STATE		U(1)

#define PLAT_MU_SR_OFF			0x20
#define PLAT_MU_COLD_BOOT_FLG_MSK	0x40000000
#ifdef COCKPIT_A72
#define PLAT_BOOT_MU_BASE		0x5D1E0000

#define BL31_BASE			0xc0000000
#define BL31_LIMIT			0xc0020000
#define CPU_START_ADDR      0x80000000  /* jump onto trampoline */
#else	/* !(defined COCKPIT_A72) */
#define PLAT_BOOT_MU_BASE		0x5D1B0000

#define BL31_BASE			0x80000000
#define BL31_LIMIT			0x80020000
#define CPU_START_ADDR      BL31_BASE	/* default behavior */
#endif

/* TEE section */
#if defined(SPD_opteed) || defined(SPD_trusty)

#ifdef COCKPIT_A53
#define BL32_BASE			0xbe000000
#define BL32_SIZE			0x02000000
#else /* !(defined COCKPIT_A53) */
#define BL32_BASE			0xfe000000
#define BL32_SIZE			0x02000000
#endif/* !(defined COCKPIT_A53) */

#ifdef SPD_trusty
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE)
#else
#define BL32_LIMIT			(BL32_BASE + BL32_SIZE - BL32_SHM_SIZE)
#endif

#ifdef COCKPIT_A72
#define BL32_FDT_OVERLAY_ADDR		0xdd000000
#else /* !(defined COCKPIT_A72) */
#define BL32_FDT_OVERLAY_ADDR		0x9d000000
#endif

#ifdef SPD_trusty
#define SECURE_HEAP_BASE		0xe0000000
#define SECURE_HEAP_LIMIT		0x10000000

#define VPU_FIRMWARE_BASE		0x96000000
#define VPU_FIRMWARE_LIMIT		0x02000000
#endif

#endif /* TEE section */

#define OCRAM_BASE		0x100000
#define OCRAM_ALIAS_SIZE 0x18000 /* The lower 96KB is in OCRAM alias from 0x0 */

#define BL32_SHM_SIZE			0x00400000

#define PLAT_GICD_BASE			0x51a00000
#define PLAT_GICR_BASE			0x51b00000

#define PLATFORM_GIC_CORE_COUNT 6

#define PLAT_CCI_BASE			0x52090000

#if defined(COCKPIT_A72)
#define IMX_BOOT_UART_BASE		0x5a080000
#elif defined(COCKPIT_A53)
#define IMX_BOOT_UART_BASE		0x5a060000
#else
/* UART */
#if defined(IMX_USE_UART0)
#define IMX_BOOT_UART_BASE		0x5a060000
#elif defined(IMX_USE_UART1)
#define IMX_BOOT_UART_BASE		0x5a070000
#else
#error "Provide proper UART number in IMX_DEBUG_UART"
#endif
#endif
#define IMX_BOOT_UART_SIZE		0x1000
#define IMX_BOOT_UART_BAUDRATE		115200
#define IMX_BOOT_UART_CLK_IN_HZ		24000000
#define PLAT_CRASH_UART_BASE		IMX_BOOT_UART_BASE
#define PLAT__CRASH_UART_CLK_IN_HZ	24000000
#define IMX_CONSOLE_BAUDRATE		115200

#define SC_IPC_BASE			PLAT_BOOT_MU_BASE
#define SC_IPC_SIZE			0x10000
#ifdef COCKPIT_A72
#define SC_R_GPT			SC_R_GPT_1
#define IMX_GPT_LPCG_BASE		0x5d550000
#define IMX_GPT_BASE			0x5d150000
#else	/* !(defined COCKPIT_A72) */
#define SC_R_GPT			SC_R_GPT_0
#define IMX_GPT_LPCG_BASE		0x5d540000
#define IMX_GPT_BASE			0x5d140000
#endif
#define IMX_WUP_IRQSTR_BASE		0x51090000
#define IMX_REG_BASE			0x50000000
#define IMX_REG_SIZE			0x10000000
#define IMX_CAAM_BASE			0x31400000

#define COUNTER_FREQUENCY		8000000 /* 8MHz */

/* non-secure uboot base */
#define PLAT_NS_IMAGE_OFFSET		BL31_LIMIT

#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 36)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 36)

#ifdef SPD_trusty
#define MAX_XLAT_TABLES			10
#define MAX_MMAP_REGIONS		14
#else
#define MAX_XLAT_TABLES			8
#define MAX_MMAP_REGIONS		12
#endif

#ifdef SPD_trusty
#define DEBUG_CONSOLE_A53		1
#else
#define DEBUG_CONSOLE_A53		DEBUG_CONSOLE
#endif

#define IMX_TRUSTY_STACK_SIZE 0x200
#define TRUSTY_SHARED_MEMORY_OBJ_SIZE (12 * 1024)
#define IMX_SEPARATE_NOBITS_BASE	U(0x130000)
#define IMX_SEPARATE_NOBITS_LIMIT	U(0x140000)

#endif /* PLATFORM_DEF_H */
