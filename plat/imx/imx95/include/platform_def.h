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
#define PLATFORM_MAX_CPU_PER_CLUSTER	U(6)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	U(6)
#define PLATFORM_CORE_COUNT		U(6)

#define IMX_PWR_LVL0			MPIDR_AFFLVL0

#define PWR_DOMAIN_AT_MAX_LVL		U(1)
#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_MAX_OFF_STATE		U(4)
#define PLAT_MAX_RET_STATE		U(2)

/* OCRAM region 128KB */
#define BL31_BASE			U(0x204A0000)
#define BL31_LIMIT			U(0x204c0000)

#define IMX_SEPARATE_XLAT_TABLE 1
#define IMX_SEPARATE_NOBITS_BASE    U(0x204C0000)
#define IMX_SEPARATE_NOBITS_LIMIT   U(0x204D0000)

/* non-secure uboot base */
/* TODO */
#define PLAT_NS_IMAGE_OFFSET		U(0x90200000)

/* GICv4 base address */
#define PLAT_GICD_BASE			U(0x48000000)
#define PLAT_GICR_BASE			U(0x48060000)

#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)

#define MAX_XLAT_TABLES			14
#define MAX_MMAP_REGIONS		32

#ifdef IMX_EMU
#define IMX_LPUART_BASE			0x42570000
#else
#define IMX_LPUART_BASE			0x44380000
#endif

#define IMX_BOOT_UART_CLK_IN_HZ		24000000 /* Select 24MHz oscillator */
#define IMX_CONSOLE_BAUDRATE		115200

#define AIPSx_SIZE			U(0x800000)
#define AIPS1_BASE			U(0x44000000)
#define AIPS2_BASE			U(0x42000000)
#define AIPS3_BASE			U(0x42800000)
#define AIPS4_BASE			U(0x49000000)
#define MU_SECURE_BASE			U(0x44220000)
#define GPIO1_BASE			U(0x47400000)
#define GPIO2_BASE			U(0x43810000)
#define GPIO3_BASE			U(0x43820000)
#define GPIO4_BASE			U(0x43840000)
#define GPIO5_BASE			U(0x43850000)

#define CCM_BASE			U(0x44450000)
#define SRC_BASE			U(0x44460000)

#define TRDC_A_BASE			U(0x44270000)
#define TRDC_W_BASE			U(0x42460000)
#define TRDC_M_BASE			U(0x42810000)
#define TRDC_N_BASE			U(0x49010000)
#define TRDC_C_BASE			U(0x4ac40000)
#define TRDC_D_BASE			U(0x4b040000)
#define TRDC_E_BASE			U(0x4c840000)
#define TRDC_G_BASE			U(0x4d840000)
#define TRDC_H_BASE			U(0x4c040000)
#define TRDC_V_BASE			U(0x4c440000)

#define ELE_MU_BASE			U(0x47520000)

#define TRDC_x_SISE			U(0x20000)

#define SMT_BUFFER_BASE			U(0x204d6000)
#define SMT_BUFFER_SIZE			0x1000

#define IMX95_SCMI_PAYLOAD_BASE		0x44221000
#define IMX95_MU1_BASE		0x44220000
#define MU_GCR_OFF			0x114

#define COUNTER_FREQUENCY		24000000

#endif /* platform_def.h */
