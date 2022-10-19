/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

#define PLATFORM_STACK_SIZE		0xB00
#define CACHE_WRITEBACK_GRANULE		64

#define PLAT_PRIMARY_CPU		U(0x0)
#define PLATFORM_MAX_CPU_PER_CLUSTER	U(2)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	U(2)
#define PLATFORM_CORE_COUNT		U(2)

#define IMX_PWR_LVL0			MPIDR_AFFLVL0

#define PWR_DOMAIN_AT_MAX_LVL		U(1)
#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_MAX_OFF_STATE		U(4)
#define PLAT_MAX_RET_STATE		U(2)

#define BL31_BASE			U(0x204E0000)
#define BL31_LIMIT			U(0x20520000)
#define SAVED_DRAM_TIMING_BASE		U(0x2051c000)

/* non-secure uboot base */
/* TODO */
#define PLAT_NS_IMAGE_OFFSET		U(0x80200000)
#define BL32_FDT_OVERLAY_ADDR           (PLAT_NS_IMAGE_OFFSET + 0x3000000)

/* GICv4 base address */
#define PLAT_GICD_BASE			U(0x48000000)
#define PLAT_GICR_BASE			U(0x48040000)

#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)

#define MAX_XLAT_TABLES			12
#define MAX_MMAP_REGIONS		16

#define IMX_LPUART_BASE			0x44380000
#define IMX_BOOT_UART_CLK_IN_HZ		24000000 /* Select 24MHz oscillator */
#define IMX_CONSOLE_BAUDRATE		115200

#define AIPSx_SIZE			U(0x800000)
#define AIPS1_BASE			U(0x44000000)
#define AIPS2_BASE			U(0x42000000)
#define AIPS3_BASE			U(0x42800000)
#define NIC_MAIN_GPV_BASE		U(0x43900000)
#define AIPS4_BASE			U(0x49000000)
#define GPIO1_BASE			U(0x47400000)
#define GPIO2_BASE			U(0x43810000)
#define GPIO3_BASE			U(0x43820000)
#define GPIO4_BASE			U(0x43830000)
#define GPIO_BASE			U(0x53810000)
#define GPIO_SIZE			U(0x30000)
#define S400_MU_BASE			U(0x47520000)
#define DDRMIX_BASE			U(0x4E000000)
#define DDRMIX_SIZE			U(0x400000)

#define TRDC_A_BASE			U(0x44270000)
#define TRDC_W_BASE			U(0x42460000)
#define TRDC_M_BASE			U(0x42810000)
#define TRDC_N_BASE			U(0x49010000)
#define TRDC_x_SISE			U(0x20000)

#define COUNTER_FREQUENCY		24000000

#endif /* platform_def.h */
