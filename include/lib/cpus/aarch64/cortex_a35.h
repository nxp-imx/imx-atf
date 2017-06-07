/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CORTEX_A35_H__
#define __CORTEX_A35_H__

/* Cortex-A35 Main ID register for revision 0 */
#define CORTEX_A35_MIDR				0x410FD040

/* Retention timer tick definitions */
#define RETENTION_ENTRY_TICKS_2		0x1
#define RETENTION_ENTRY_TICKS_8		0x2
#define RETENTION_ENTRY_TICKS_32	0x3
#define RETENTION_ENTRY_TICKS_64	0x4
#define RETENTION_ENTRY_TICKS_128	0x5
#define RETENTION_ENTRY_TICKS_256	0x6
#define RETENTION_ENTRY_TICKS_512	0x7

/*******************************************************************************
 * CPU Extended Control register specific definitions.
 * CPUECTLR_EL1 is an implementation-specific register.
 ******************************************************************************/
#define CORTEX_A35_CPUECTLR_EL1			S3_1_C15_C2_1
#define CORTEX_A35_CPUECTLR_SMPEN_BIT		(1 << 6)

#define CPUECTLR_CPU_RET_CTRL_SHIFT		0
#define CPUECTLR_CPU_RET_CTRL_MASK		(0x7 << CPUECTLR_CPU_RET_CTRL_SHIFT)

#define CPUECTLR_FPU_RET_CTRL_SHIFT		3
#define CPUECTLR_FPU_RET_CTRL_MASK		(0x7 << CPUECTLR_FPU_RET_CTRL_SHIFT)

/*******************************************************************************
 * CPU Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define CPUMERRSR_EL1			S3_1_C15_C2_2	/* Instruction def. */

/*******************************************************************************
 * L2 Extended Control register specific definitions.
 ******************************************************************************/
#define L2ECTLR_EL1			S3_1_C11_C0_3	/* Instruction def. */

#define L2ECTLR_RET_CTRL_SHIFT		0
#define L2ECTLR_RET_CTRL_MASK		(0x7 << L2ECTLR_RET_CTRL_SHIFT)

/*******************************************************************************
 * L2 Memory Error Syndrome register specific definitions.
 ******************************************************************************/
#define L2MERRSR_EL1			S3_1_C15_C2_3	/* Instruction def. */

#endif /* __CORTEX_A35_H__ */
