/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DEF_H
#define PLAT_DEF_H

#include <arch.h>
/*
 * Required without TBBR. To include the defines for DDR PHY
 * Images.
 */
#include <tbbr_img_def.h>

#include <policy.h>
#include <soc.h>

#define NXP_SYSCLK_FREQ		100000000
#define NXP_DDRCLK_FREQ		133333333

/* UART related definition */
#define NXP_CONSOLE_ADDR	NXP_UART_ADDR
#define NXP_CONSOLE_BAUDRATE	115200

/* Size of cacheable stacks */
#if defined(IMAGE_BL2)
#if defined(TRUSTED_BOARD_BOOT)
#define PLATFORM_STACK_SIZE	0x2000
#else
#define PLATFORM_STACK_SIZE	0x1000
#endif
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE	0x2000
#endif

#define BL2_START		NXP_OCRAM_ADDR
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE)
#define BL2_NOLOAD_START	NXP_OCRAM_ADDR
#define BL2_NOLOAD_LIMIT	BL2_BASE

/* IO defines as needed by IO driver framework */
#define MAX_IO_DEVICES		4
#define MAX_IO_BLOCK_DEVICES	1
#define MAX_IO_HANDLES		4

/*
 * FIP image defines - Offset at which FIP Image would be present
 * Image would include Bl31 , Bl33 and Bl32 (optional)
 */
#ifdef POLICY_FUSE_PROVISION
#define MAX_FIP_DEVICES		2
#endif

/*
 * ID of the secure physical generic timer interrupt used by the BL32.
 */
#define BL32_IRQ_SEC_PHY_TIMER	29

#define BL31_WDOG_SEC		89

/*
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_LS_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BL32_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

/* SGI 15 and Secure watchdog interrupts assigned to Group 0 */
#define PLAT_LS_G0_IRQ_PROPS(grp)	\
	INTR_PROP_DESC(BL31_WDOG_SEC, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(15, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
#endif
