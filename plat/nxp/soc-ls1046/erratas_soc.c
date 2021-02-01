/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <cci.h>
#include <mmio.h>
#include <soc_default_base_addr.h>
#include <ls_interconnect.h>
#include <plat_common.h>
#include <dcfg.h>
#include <scfg.h>

void erratum_a008850_early(void)
{
#ifdef ERRATA_SOC_A008850
	/* part 1 of 2 */
	uintptr_t cci_base = NXP_CCI_ADDR;
	uint32_t val = mmio_read_32(cci_base + CTRL_OVERRIDE_REG);

	/* enabling forced barrier termination on CCI400 */
	mmio_write_32(cci_base + CTRL_OVERRIDE_REG,
		      (val | CCI_TERMINATE_BARRIER_TX));

#endif
}

void erratum_a008850_post(void)
{
#ifdef ERRATA_SOC_A008850
	/* part 2 of 2 */
	uintptr_t cci_base = NXP_CCI_ADDR;
	uint32_t val = mmio_read_32(cci_base + CTRL_OVERRIDE_REG);

	/* Clear the BARRIER_TX bit */
	val = val & ~(CCI_TERMINATE_BARRIER_TX);

	/*
	 * Disable barrier termination on CCI400, allowing
	 * barriers to propagate across CCI
	 */
	mmio_write_32(cci_base + CTRL_OVERRIDE_REG, val);
#endif
}

void erratum_a010539(void)
{
#ifdef ERRATA_SOC_A010539
#if POLICY_OTA
	/*
	 * For POLICY_OTA Bootstrap, BOOT_DEVICE_EMMC is used to get FIP and
	 * other firmware on SD card. The actual boot source is QSPI. So this
	 * erratum workaround should be executed too.
	 */
	if ((get_boot_dev() == BOOT_DEVICE_EMMC) || (get_boot_dev() == BOOT_DEVICE_QSPI)) {
#else
	if (get_boot_dev() == BOOT_DEVICE_QSPI) {
#endif
		unsigned int *porsr1 = (void *)(NXP_DCFG_ADDR + DCFG_PORSR1_OFFSET);
		uint32_t val;

		val = (gur_in32(porsr1) & ~PORSR1_RCW_MASK);
		gur_out32((void *)(NXP_DCSR_DCFG_ADDR + DCFG_DCSR_PORCR1_OFFSET), val);
		scfg_out32((void *)(NXP_SCFG_ADDR + 0x1a8), 0xffffffff);
	}

#endif
}
