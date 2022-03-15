/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <scfg.h>

#include <bl31_data.h>
#include <plat_psci.h>
#include <platform_def.h>

.global soc_init_lowlevel
.global soc_init_percpu

.global _soc_sys_reset
.global _soc_sys_off
.global _soc_ck_disabled
.global _soc_core_prep_stdby
.global _soc_core_entr_stdby
.global _soc_core_exit_stdby
.global _soc_core_prep_pwrdn
.global _soc_core_entr_pwrdn
.global _soc_core_exit_pwrdn
.global _soc_clstr_prep_stdby
.global _soc_clstr_exit_stdby
.global _soc_clstr_prep_pwrdn
.global _soc_clstr_exit_pwrdn
.global _soc_sys_prep_stdby
.global _soc_sys_exit_stdby
.global _soc_sys_prep_pwrdn
.global _soc_sys_pwrdn_wfi
.global _soc_sys_exit_pwrdn


#define SWLPM20_WA		1

#define DAIF_DATA		AUX_01_DATA

#define DEVDISR1_MASK_OFFSET	0x0
#define DEVDISR5_MASK_OFFSET	0x8
#define CPUACTLR_DATA_OFFSET	0x10

#define IPSTPACK_RETRY_CNT	0x10000
#define DDR_SLEEP_RETRY_CNT	0x10000
#define CPUACTLR_EL1		S3_1_C15_C2_0
#define CPUACTLR_L1PCTL_MASK	0x0000E000
#define DDR_SDRAM_CFG_2_FRCSR	0x80000000
#define DDR_SDRAM_CFG_2_OFFSET	0x114
#define DDR_CNTRL_BASE_ADDR	0x01080000

#define ERROR_DDR_SLEEP		-1
#define ERROR_DDR_WAKE		-2
#define ERROR_NO_QUIESCE	-3

/*
 * set this switch to 1 if you need to keep the debug block
 * clocked during system power-down
 */
#define DEBUG_ACTIVE		0
/*
 * set this switch to 1 if you need to keep the ocram 1&2
 * clocked during system power-down
 */
#define OCRAM_ACTIVE		1

#define IPPDEXPCR_PFE_MAC1	0x80000000
#define IPPDEXPCR_PFE_MAC2	0x40000000
#define IPPDEXPCR_PFE_PE	0x20000000
#define IPPDEXPCR_PFE_250M	0x10000000
#define IPPDEXPCR_I2C1		0x00080000
#define IPPDEXPCR_FLEXTIMER1	0x00020000
#define IPPDEXPCR_OCRAM1	0x00010000
#define IPPDEXPCR_GPIO1		0x00000040
#define IPPDEXPCR_PFE		0x00000020

#define IPPDEXPCR_PFE_MASK	\
	(IPPDEXPCR_PFE_MAC1 | IPPDEXPCR_PFE_MAC2 | IPPDEXPCR_PFE_PE | \
	 IPPDEXPCR_PFE_250M | IPPDEXPCR_PFE)

#define DEVDISR1_SEC		0x00000200
#define DEVDISR1_USB3		0x00004000
#define DEVDISR1_SATA		0x00008000
#define DEVDISR1_USB2		0x00040000
#define DEVDISR1_PFE		0x00080000
#define DEVDISR1_EDMA		0x00400000
#define DEVDISR1_ESDHC2		0x10000000
#define DEVDISR1_ESDHC1		0x20000000
#define DEVDISR1_PBL		0x80000000

#define DEVDISR1_VALUE		\
	(DEVDISR1_PBL | DEVDISR1_ESDHC1 | DEVDISR1_ESDHC2 | \
	 DEVDISR1_EDMA | DEVDISR1_PFE | DEVDISR1_USB2 | DEVDISR1_SATA | \
	 DEVDISR1_USB3 | DEVDISR1_SEC)

#define DEVDISR4_QSPI		0x08000000
#define DEVDISR4_DUART1		0x20000000

#define DEVDISR4_VALUE		(DEVDISR4_QSPI | DEVDISR4_DUART1)

#define DEVDISR5_CCI400		0x00000001
#define DEVDISR5_I2C_1		0x00000002
#define DEVDISR5_I2C_2		0x00000004
#define DEVDISR5_SPI1		0x00000100
#define DEVDISR5_WDOG2		0x00000200
#define DEVDISR5_FLEXTIMER	0x00000400
#define DEVDISR5_WDOG1		0x00000800
#define DEVDISR5_SAI5		0x00040000
#define DEVDISR5_SAI4		0x00080000
#define DEVDISR5_SAI3		0x00100000
#define DEVDISR5_DBG		0x00200000
#define DEVDISR5_GPIO		0x00400000
#define DEVDISR5_OCRAM2		0x01000000
#define DEVDISR5_OCRAM1		0x02000000
#define DEVDISR5_SAI2		0x04000000
#define DEVDISR5_SAI1		0x08000000
#define DEVDISR5_DDR		0x80000000

#define DEVDISR5_BASE		\
	(DEVDISR5_SAI1 | DEVDISR5_SAI2 | \
	 DEVDISR5_GPIO | DEVDISR5_SAI3 | \
	 DEVDISR5_SAI4 | DEVDISR5_SAI5 | DEVDISR5_WDOG1 | \
	 DEVDISR5_FLEXTIMER | DEVDISR5_WDOG2 | DEVDISR5_SPI1 | \
	 DEVDISR5_I2C_2 | DEVDISR5_I2C_1 )

#if (OCRAM_ACTIVE && DEBUG_ACTIVE)
  #define DEVDISR5_VALUE	DEVDISR5_BASE
#elif (!OCRAM_ACTIVE && !DEBUG_ACTIVE)
  #define DEVDISR5_VALUE	\
	(DEVDISR5_BASE | DEVDISR5_OCRAM1 | DEVDISR5_OCRAM2 | DEVDISR5_DBG)
#elif !OCRAM_ACTIVE
  #define DEVDISR5_VALUE	\
	(DEVDISR5_BASE | DEVDISR5_OCRAM1 | DEVDISR5_OCRAM2)
#elif !DEBUG_ACTIVE
  #define DEVDISR5_VALUE	(DEVDISR5_BASE | DEVDISR5_DBG)
#endif

#define DEVDISR5_MEM		DEVDISR5_DDR

/*
 * Note that the IPSTPCRn and IPSTPACKRn registers have the same bit
 * definition as DEVDISRn.
 * IPSTPCR0 to DEVDISR1, IPSTPCR3 to DEVDISR4, IPSTPCR4 to DEVDISR5
 */
#define IPSTPCR0_VALUE		DEVDISR1_VALUE
#define IPSTPCR3_VALUE		DEVDISR4_VALUE
#define IPSTPCR4_VALUE		DEVDISR5_VALUE

/*
 * this function initialize the soc.
 * in:
 * out:
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11
 */
func soc_init_lowlevel
	ret
endfunc soc_init_lowlevel

/*
 * this function performs any soc-specific initialization that is needed on
 * a per-core basis
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func soc_init_percpu
	mov	x3, x30

	bl	plat_my_core_mask
	mov	x2, x0

	/* x2 = core mask */

	/* see if this core is marked for prefetch disable */
	mov	x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data
	tst	x0, x2
	b.eq	1f
	bl	_disable_ldstr_pfetch_A53
1:
	mov	x30, x3
	ret
endfunc soc_init_percpu

/*
 * this function resets the system via SoC-specific methods
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4
 */
_soc_sys_reset:
	ldr	x2, =NXP_DCFG_ADDR

	/* make sure the mask is cleared in the reset request mask register */
	mov	w1, wzr
	str	w1, [x2, #DCFG_RSTRQMR1_OFFSET]

	/* set the reset request */
	ldr	w1, =RSTCR_RESET_REQ
	ldr	x4, =DCFG_RSTCR_OFFSET
	rev	w0, w1
	str	w0, [x2, x4]

	/*
	 * just in case this address range is mapped as cacheable,
	 * flush the write out of the dcaches
	 */
	add	x4, x2, x4
	dc	cvac, x4
	dsb	st
	isb

	/* this function does not return */
	b	.

/*
 * part of SYSTEM_OFF
 * this function turns off the SoC clocks
 * Note: this function is not intended to return, and the only allowable
 *       recovery is POR
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, [x13, x14, x15]
 */
_soc_sys_off:
	/* mask interrupts at the core */
	mrs	x1, DAIF
	mov	x0, #DAIF_SET_MASK
	orr	x0, x1, x0
	msr	DAIF, x0

	/* disable icache, dcache, mmu @ EL1 */
	mov	x1, #SCTLR_I_C_M_MASK
	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0

	/* disable dcache for EL3 */
	mrs	x1, SCTLR_EL3
	bic	x1, x1, #SCTLR_C_MASK
	/* make sure icache is enabled */
	orr	x1, x1, #SCTLR_I_MASK
	msr	SCTLR_EL3, x1
	isb

	/* set WFIL2_EN in SCFG_COREPMCR */
	mov	x0, #SCFG_COREPMCR_OFFSET
	mov	x1, #COREPMCR_WFIL2
	bl	write_reg_scfg

	/* set OVRD_EN in RCPM2_POWMGTDCR */
	mov	x0, #RCPM2_POWMGTDCR_OFFSET
	mov	x1, #POWMGTDCR_OVRD_EN
	bl	write_reg_rcpm2

	/* read IPPDEXPCR0 @ RCPM_IPPDEXPCR0 */
	mov	x0, #RCPM_IPPDEXPCR0_OFFSET
	bl	read_reg_rcpm
	mov	w7, w0

	/* get DEVDISR1 mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA

	mov	w5, wzr
	ldr	w6, =IPPDEXPCR_PFE_MASK
	and	w6, w6, w7
	cbz	w6, 10f

	orr	w5, w5, #DEVDISR1_PFE
10:
	str	w5, [x2, #DEVDISR1_MASK_OFFSET]

	/* get DEVDISR5 mask */
	mov	w5, wzr
	mov	w6, #IPPDEXPCR_I2C1
	and	w6, w6, w7
	cbz	w6, 12f

	orr	w5, w5, #DEVDISR5_I2C_1
12:
	mov	w6, #IPPDEXPCR_FLEXTIMER1
	and	w6, w6, w7
	cbz	w6, 14f

	orr	w5, w5, #DEVDISR5_FLEXTIMER
14:
	mov	w6, #IPPDEXPCR_OCRAM1
	and	w6, w6, w7
	cbz	w6, 16f

	orr	w5, w5, #DEVDISR5_OCRAM1
16:
	mov	w6, #IPPDEXPCR_GPIO1
	and	w6, w6, w7
	cbz	w6, 18f

	orr	w5, w5, #DEVDISR5_GPIO
18:
	str	w5, [x2, #DEVDISR5_MASK_OFFSET]

	ldr	w6, [x2, #DEVDISR1_MASK_OFFSET]

	/* w6 = DEVDISR1 override mask */

	/* write IPSTPCR0 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR0_OFFSET
	ldr	w1, =IPSTPCR0_VALUE
	bic	w1, w1, w6
	bl	write_reg_rcpm2

	/* write IPSTPCR3 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR3_OFFSET
	ldr	w1, =IPSTPCR3_VALUE
	bl	write_reg_rcpm2

	/* w5 = DEVDISR5 override mask */

	/* write IPSTPCR4 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR4_OFFSET
	ldr	w1, =IPSTPCR4_VALUE
	bic	w1, w1, w5
	bl	write_reg_rcpm2

	/* w6 = DEVDISR1 override mask */

	/* poll on IPSTPACK0 */
	mov	x3, #RCPM2_IPSTPACKR0_OFFSET
	ldr	w4, =IPSTPCR0_VALUE
	bic	w4, w4, w6
	ldr	x7, =IPSTPACK_RETRY_CNT
3:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	14f
	sub	x7, x7, #1
	cbnz	x7, 3b

14:
	/* poll on IPSTPACK3 */
	mov	x3, #RCPM2_IPSTPACKR3_OFFSET
	ldr	w4, =IPSTPCR3_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
6:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	17f
	sub	x7, x7, #1
	cbnz	x7, 6b

	/* w5 = DEVDISR5 override mask */
17:
	/* poll on IPSTPACK4 */
	mov	x3, #RCPM2_IPSTPACKR4_OFFSET
	ldr	w4, =IPSTPCR4_VALUE
	bic	w4, w4, w5
	ldr	x7, =IPSTPACK_RETRY_CNT
7:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	18f
	sub	x7, x7, #1
	cbnz	x7, 7b

18:
	ldr	x7, =BC_PSCI_BASE
	add	x7, x7, #AUX_01_DATA

	/* w6 = DEVDISR1 override mask */
	/* x7 = [soc_data_area] */

	/* DEVDISR1 - load new value */
	mov	x0, #DCFG_DEVDISR1_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR1_OFFSET
	ldr	w1, =DEVDISR1_VALUE
	bic	w1, w1, w6
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* DEVDISR4 - load new value */
	mov	x0, #DCFG_DEVDISR4_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR4_OFFSET
	ldr	w1, =DEVDISR4_VALUE
	bl	write_reg_dcfg

	/* w5 = DEVDISR5 override mask */

	/* DEVDISR5 - load new value */
	mov	x0, #DCFG_DEVDISR5_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR5_OFFSET
	ldr	w1, =DEVDISR5_VALUE
	bic	w1, w1, w5
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* disable data prefetch */
	mrs	x0, CPUACTLR_EL1
	bic	x0, x0, #CPUACTLR_L1PCTL_MASK
	msr	CPUACTLR_EL1, x0

	/* w5 = DEVDISR5 override mask */

	/* setup registers for cache-only execution */
	ldr	w6, =IPSTPCR4_VALUE
	bic	w5, w6, w5
	mov	x6, #DDR_CNTRL_BASE_ADDR
	mov	x7, #DCSR_RCPM2_BASE
	mov	x8, #NXP_DCFG_ADDR
	dsb	sy
	isb

	/*
	 * w5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
	 * x6  = DDR_CNTRL_BASE_ADDR
	 * x7  = DCSR_RCPM2_BASE
	 * x8  = NXP_DCFG_ADDR
	 */

	/* enter the cache-only sequence - there is no return from this function */
	b	final_shutdown

/*
 * this function determines if a core is disabled via COREDISR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *      w0 != 0, core disabled
 * uses x0, x1, x2
 */
_soc_ck_disabled:
	/* always return 'none disabled' */
	mov	w0, #0
	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core into standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_core_entr_stdby:
	/* X0 = core mask lsb */
	dsb	sy
	isb
	wfi

	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_core_prep_stdby:
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_core_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_core_prep_pwrdn:
	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	/* clear retention control bits */
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
	isb

	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core into a power-down state
 * ph20 is defeatured for this device, so pw15 is the lowest core pwr state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_core_entr_pwrdn:
	/* X0 = core mask lsb */
	dsb	sy
	isb
	wfi

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_core_exit_pwrdn:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_clstr_prep_stdby:
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_clstr_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_clstr_prep_pwrdn:
	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	/* clear retention control bits */
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
	isb

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_clstr_exit_pwrdn:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_prep_stdby:
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_sys_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to
 * suspend-to-power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4
 */
_soc_sys_prep_pwrdn:
	mov	x4, x30

	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
	isb

	/* set WFIL2_EN in SCFG_COREPMCR */
	mov	x0, #SCFG_COREPMCR_OFFSET
	mov	x1, #COREPMCR_WFIL2
	bl	write_reg_scfg

	/* set OVRD_EN in RCPM2_POWMGTDCR */
	mov	x0, #RCPM2_POWMGTDCR_OFFSET
	mov	x1, #POWMGTDCR_OVRD_EN
	bl	write_reg_rcpm2

	mov	x30, x4
	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *      x0 < 0, failure
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x13, x14, x15,
 *      x16, x17, x18
 */
_soc_sys_pwrdn_wfi:
	mov	x18, x30

	/* read IPPDEXPCR0 @ RCPM_IPPDEXPCR0 */
	mov	x0, #RCPM_IPPDEXPCR0_OFFSET
	bl	read_reg_rcpm
	mov	w7, w0

	/* w7 = IPPDEXPCR */

	/* get DEVDISR1 mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA

	mov	w5, wzr
	ldr	w6, =IPPDEXPCR_PFE_MASK
	and	w6, w6, w7
	cbz	w6, 10f

	orr	w5, w5, #DEVDISR1_PFE
10:
	str	w5, [x2, #DEVDISR1_MASK_OFFSET]

	/* get DEVDISR5 mask */
	mov	w5, wzr
	mov	w6, #IPPDEXPCR_I2C1
	and	w6, w6, w7
	cbz	w6, 12f

	orr	w5, w5, #DEVDISR5_I2C_1
12:
	mov	w6, #IPPDEXPCR_FLEXTIMER1
	and	w6, w6, w7
	cbz	w6, 14f

	orr	w5, w5, #DEVDISR5_FLEXTIMER
14:
	mov	w6, #IPPDEXPCR_OCRAM1
	and	w6, w6, w7
	cbz	w6, 16f

	orr	w5, w5, #DEVDISR5_OCRAM1
16:
	mov	w6, #IPPDEXPCR_GPIO1
	and	w6, w6, w7
	cbz	w6, 18f

	orr	w5, w5, #DEVDISR5_GPIO
18:
	str	w5, [x2, #DEVDISR5_MASK_OFFSET]
	ldr	w6, [x2, #DEVDISR1_MASK_OFFSET]

	/* w6 = DEVDISR1 override mask */

	/* write IPSTPCR0 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR0_OFFSET
	ldr	w1, =IPSTPCR0_VALUE
	bic	w1, w1, w6
	bl	write_reg_rcpm2

	/* write IPSTPCR3 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR3_OFFSET
	ldr	w1, =IPSTPCR3_VALUE
	bl	write_reg_rcpm2

	/* w5 = DEVDISR5 override mask */

	/* write IPSTPCR4 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR4_OFFSET
	ldr	w1, =IPSTPCR4_VALUE
	bic	w1, w1, w5
	bl	write_reg_rcpm2

	/* w6 = DEVDISR1 override mask */

	/* poll on IPSTPACK0 */
	mov	x3, #RCPM2_IPSTPACKR0_OFFSET
	ldr	w4, =IPSTPCR0_VALUE
	bic	w4, w4, w6
	ldr	x7, =IPSTPACK_RETRY_CNT
3:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	14f
	sub	x7, x7, #1
	cbnz	x7, 3b

14:
	/* poll on IPSTPACK3 */
	mov	x3, #RCPM2_IPSTPACKR3_OFFSET
	ldr	w4, =IPSTPCR3_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
6:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	17f
	sub	x7, x7, #1
	cbnz	x7, 6b

	/* w5 = DEVDISR5 override mask */
17:
	/* poll on IPSTPACK4 */
	mov	x3, #RCPM2_IPSTPACKR4_OFFSET
	ldr	w4, =IPSTPCR4_VALUE
	bic	w4, w4, w5
	ldr	x7, =IPSTPACK_RETRY_CNT
7:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	w0, w4
	b.eq	18f
	sub	x7, x7, #1
	cbnz	x7, 7b

18:
	ldr	x7, =BC_PSCI_BASE
	add	x7, x7, #AUX_01_DATA

	/* w6 = DEVDISR1 override mask */
	/* x7 = [soc_data_area] */

	/* save DEVDISR1 and load new value */
	mov	x0, #DCFG_DEVDISR1_OFFSET
	bl	read_reg_dcfg
	mov	w13, w0
	mov	x0, #DCFG_DEVDISR1_OFFSET
	ldr	w1, =DEVDISR1_VALUE
	bic	w1, w1, w6
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* save DEVDISR4 and load new value */
	mov	x0, #DCFG_DEVDISR4_OFFSET
	bl	read_reg_dcfg
	mov	w16, w0
	mov	x0, #DCFG_DEVDISR4_OFFSET
	ldr	w1, =DEVDISR4_VALUE
	bl	write_reg_dcfg

	/* w5 = DEVDISR5 override mask */

	/* save DEVDISR5 and load new value */
	mov	x0, #DCFG_DEVDISR5_OFFSET
	bl	read_reg_dcfg
	mov	w17, w0
	mov	x0, #DCFG_DEVDISR5_OFFSET
	ldr	w1, =DEVDISR5_VALUE
	bic	w1, w1, w5
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* save cpuactlr and disable data prefetch */
	mrs	x0, CPUACTLR_EL1
	str	x0, [x7, #CPUACTLR_DATA_OFFSET]
	bic	x0, x0, #CPUACTLR_L1PCTL_MASK
	msr	CPUACTLR_EL1, x0

	/* w5 = DEVDISR5 override mask */

	/* setup registers for cache-only execution */
	ldr	w6, =IPSTPCR4_VALUE
	bic	w5, w6, w5
	mov	x6, #DDR_CNTRL_BASE_ADDR
	mov	x7, #DCSR_RCPM2_BASE
	mov	x8, #NXP_DCFG_ADDR
	dsb	sy
	isb

	/*
	 * w5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
	 * x6  = DDR_CNTRL_BASE_ADDR
	 * x7  = DCSR_RCPM2_BASE
	 * x8  = NXP_DCFG_ADDR
	 * w13 = DEVDISR1 saved value
	 * w16 = DEVDISR4 saved value
	 * w17 = DEVDISR5 saved value
	 */

	/* enter the cache-only sequence */
	bl	final_pwrdown

	/* when we are here, the core has come out of wfi and the SoC is back up */

	mov	x30, x18
	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, [x13, x14, x15]
 */
_soc_sys_exit_pwrdn:
	/* restore cpuactlr_el1 */
	ldr	x1, =BC_PSCI_BASE
	add	x1, x1, #AUX_01_DATA
	ldr	x0, [x1, #CPUACTLR_DATA_OFFSET]
	msr	CPUACTLR_EL1, x0

	/* clear POWMGTDCR */
	mov	x1, #DCSR_RCPM2_BASE
	str	wzr, [x1, #RCPM2_POWMGTDCR_OFFSET]

	/* clear WFIL2_EN in SCFG_COREPMCR */
	mov	x1, #NXP_SCFG_ADDR
	str	wzr, [x1, #SCFG_COREPMCR_OFFSET]

	ret

/*
 * this function loads a 64-bit execution address of the core
 * in the soc registers BOOTLOCPTRL/H
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * uses x0, x1, x2, x3
 */
_soc_set_start_addr:
	/* get the 64-bit base address of the scfg block */
	ldr	x2, =NXP_SCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register (offset 0x604 in the scfg block) */
	mov	x1, x0
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register (offset 0x600 in the scfg block) */
	lsr	x1, x0, #32
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRH_OFFSET]
	ret

/*
 * this function returns a 64-bit execution address of the core in x0
 * out: x0, address found in BOOTLOCPTRL/H
 * uses x0, x1, x2
 */
_soc_get_start_addr:
	/* get the 64-bit base address of the scfg block */
	ldr	x1, =NXP_SCFG_ADDR

	/* read the 32-bit BOOTLOCPTRL register (offset 0x604 in the scfg block) */
	ldr	w0, [x1, #SCFG_BOOTLOCPTRL_OFFSET]
	/* swap bytes for BE */
	rev	w2, w0

	/* read the 32-bit BOOTLOCPTRH register (offset 0x600 in the scfg block) */
	ldr	w0, [x1, #SCFG_BOOTLOCPTRH_OFFSET]
	rev	w1, w0
	/* create a 64-bit BOOTLOCPTR address */
	orr	x0, x2, x1, LSL #32
	ret

/*
 * this function sets the security mechanisms in the SoC to implement the
 * Platform Security Policy
 */
_set_platform_security:
	ret

/*
 * this function returns the base address of the gic distributor
 * in:  none
 * out: x0 = base address of gic distributor
 * uses x0
 */
_getGICD_BaseAddr:
	ldr	x0, =NXP_GICD_4K_ADDR
	ret

/*
 * this function returns the base address of the gic controller
 * in: none
 * out: x0 = base address of gic controller
 * uses x0
 */
_getGICC_BaseAddr:
	ldr	x0, =NXP_GICC_4K_ADDR
	ret

/*
 * this function will shutdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * in:  w5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
 *      x6  = DDR_CNTRL_BASE_ADDR
 *      x7  = DCSR_RCPM2_BASE
 *      x8  = NXP_DCFG_ADDR
 *      w13 = DEVDISR1 saved value
 *      w16 = DEVDISR4 saved value
 *      w17 = DEVDISR5 saved value
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x13, x16, x17
 */

/* 4Kb aligned */
.align 12
final_pwrdown:
	mov	x0, xzr
	b	touch_line_0
start_line_0:
	mov	x0, #1
	/* ddr in self refresh - start */
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	orr	w4, w4, w2
	rev	w3, w4
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	/* ddr in self refresh - end */
	/* quiesce ddr clocks - start */
	orr	w3, w5, #DEVDISR5_MEM
	rev	w4, w3
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]
	/* quiesce ddr clocks - end */

	mov	w3, #DEVDISR5_MEM
	/* polling mask */
	rev	w3, w3
	/* poll ipstpack4 - start */
	mov	x2, #DDR_SLEEP_RETRY_CNT
touch_line_0:
	cbz	x0, touch_line_1

start_line_1:
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET]
	tst	w1, w3
	b.ne	1f
	subs	x2, x2, #1
	b.gt	start_line_1
	/* poll ipstpack4 - end */

	/* if we get here, we have a timeout err */
	rev	w4, w5
	/* re-enable ddr clks interface */
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]
	/* load error code */
	mov	x0, #ERROR_DDR_SLEEP
	b	2f
1:
	/* disable ddr cntrlr clk in devdisr5 */
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]
	/* stop the final core */
	wfi

	rev	w4, w5
	/* re-enable ddr in devdisr5 */
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]
	/* re-enable ddr clk in ipstpcr4 */
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]
	nop
touch_line_1:
	cbz	x0, touch_line_2

start_line_2:
	/* poll on ipstpack4 - start */
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET]
	tst	w1, w3
	b.eq	2f
	nop
	/* poll on ipstpack4 - end */
	b	start_line_2
2:
	/* take ddr out-of self refresh - start */
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	bic	w4, w4, w2
	rev	w3, w4
	/* wait for ddr cntrlr clock - start */
	mov	x1, #DDR_SLEEP_RETRY_CNT
3:
	subs	x1, x1, #1
	/* wait for ddr cntrlr clock - end */
	b.gt	3b
	/* take ddr out-of self refresh - end */
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w1, w17
touch_line_2:
	cbz	x0, touch_line_3

start_line_3:
	/* reset devdisr5 */
	str	w1, [x8, #DCFG_DEVDISR5_OFFSET]
	rev	w1, w16
	/* reset devdisr4 */
	str	w1, [x8, #DCFG_DEVDISR4_OFFSET]
	rev	w1, w13
	/* reset devdisr1 */
	str	w1, [x8, #DCFG_DEVDISR1_OFFSET]
	/* reset ipstpcr4 */
	str	wzr, [x7, #RCPM2_IPSTPCR4_OFFSET]
	/* reset ipstpcr3 */
	str	wzr, [x7, #RCPM2_IPSTPCR3_OFFSET]
	/* reset ipstpcr0 */
	str	wzr, [x7, #RCPM2_IPSTPCR0_OFFSET]
	b	continue_restart
	nop
	nop
	nop
	nop
	nop
	nop
touch_line_3:
	cbz	x0, start_line_0

/* execute here after ddr is back up */
continue_restart:
	/* if x0 = 1, all is well */
	/* if x0 < 1, we had an error */
	cmp	x0, #1
	b.ne	4f
	mov	x0, #0
4:
	ret

/*
 * this function will shutdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * Note: there is no return allowed from this function
 * in:  w5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
 *      x6  = DDR_CNTRL_BASE_ADDR
 *      x7  = DCSR_RCPM2_BASE
 *      x8  = NXP_DCFG_ADDR
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8
 */

/* 4Kb aligned */
.align 12
final_shutdown:
	mov	x0, xzr
	b	touch_line0
start_line0:
	mov	x0, #1
	/* ddr in self refresh - start */
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	orr	w4, w4, w2
	rev	w3, w4
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	/* ddr in self refresh - end */
	/* quiesce ddr clocks - start */
	orr	w3, w5, #DEVDISR5_MEM
	rev	w4, w3
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]
	/* quiesce ddr clocks - end */

	mov	w3, #DEVDISR5_MEM
	/* polling mask */
	rev	w3, w3
	/* poll ipstpack4 - start */
	mov	x2, #DDR_SLEEP_RETRY_CNT
touch_line0:
	cbz	x0, touch_line1

start_line1:
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET]
	tst	w1, w3
	b.ne	1f
	subs	x2, x2, #1
	b.gt	start_line1
	/* poll ipstpack4 - end */
	nop
	nop
	nop
	nop
1:
	/* disable ddr cntrlr clk */
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]
2:
	/* stop the final core */
	wfi
	/* stay here until POR */
	b	2b
	nop
	nop
	nop
touch_line1:
	cbz	x0, start_line0

/*
 * write a register in the SCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_scfg:
	ldr	x2, =NXP_SCFG_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the SCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_scfg:
	ldr	x2, =NXP_SCFG_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the RCPM block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_rcpm:
	ldr	x2, =NXP_RCPM_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the RCPM block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_rcpm:
	ldr	x2, =NXP_RCPM_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the DCSR-RCPM2 block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_rcpm2:
	ldr	x2, =NXP_DCSR_RCPM2_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the DCSR-RCPM2 block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_rcpm2:
	ldr	x2, =NXP_DCSR_RCPM2_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the DCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the DCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret
