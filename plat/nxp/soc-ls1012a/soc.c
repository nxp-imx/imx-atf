/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <caam.h>
#include <cci.h>
#include <common/debug.h>
#include <dcfg.h>
#include <lib/mmio.h>
#include <ls_interconnect.h>
#ifdef POLICY_FUSE_PROVISION
#include <nxp_gpio.h>
#endif
#include <nxp_timer.h>
#include <plat_console.h>
#include <plat_gic.h>
#include <scfg.h>
#if defined(NXP_SFP_ENABLED)
#include <sfp.h>
#endif

#include <ns_access.h>
#ifdef CONFIG_OCRAM_ECC_EN
#include <ocram.h>
#endif
#include <plat_common.h>
#include <soc.h>

const unsigned char _power_domain_tree_desc[] = {1, 1, 1};

static dcfg_init_info_t dcfg_init_data = {
	.g_nxp_dcfg_addr = NXP_DCFG_ADDR,
	.nxp_sysclk_freq = NXP_SYSCLK_FREQ,
	.nxp_ddrclk_freq = NXP_DDRCLK_FREQ,
	.nxp_plat_clk_divider = NXP_PLATFORM_CLK_DIVIDER,
};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls1012_cluster_count);

/*
 * This function returns the SoC topology
 */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return _power_domain_tree_desc;
}

/*
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 */
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return CORES_PER_CLUSTER;
}

#ifdef IMAGE_BL2

#ifdef POLICY_FUSE_PROVISION
static gpio_init_info_t gpio_init_data = {
	.gpio1_base_addr = NXP_GPIO1_ADDR,
	.gpio2_base_addr = NXP_GPIO2_ADDR,
};
#endif

/*
 * Function to set the base counter frequency at the
 * the first entry of the Frequency Mode Table,
 * at CNTFID0 (0x20 offset).
 *
 * Set the value of the pirmary core register cntfrq_el0.
 */
void set_base_freq_CNTFID0(void)
{
	/*
	 * The system counter always works with 25 MHz frequency clock.
	 */
	unsigned int counter_base_frequency = 25000000;

	/*
	 * Setting the frequency in the Frequency modes table.
	 *
	 * Note: The value for ls1046ardb board at this offset
	 *       is not RW as stated. This offset have the
	 *       fixed value of 100000400 Hz.
	 *
	 * The below code line has no effect.
	 * Keeping it for other platforms where it has effect.
	 */
	mmio_write_32(NXP_TIMER_ADDR + CNTFID_OFF, counter_base_frequency);

	write_cntfrq_el0(counter_base_frequency);
}

/*
 * This function returns the number of clusters in the SoC
 */
static unsigned int get_num_cluster(void)
{
	return NUMBER_OF_CLUSTERS;
}

void soc_preload_setup(void)
{

}

/*
 * This function implements soc specific erratas
 * This is called before DDR is initialized or MMU is enabled
 */
void soc_early_init(void)
{
	dram_regions_info_t *dram_regions_info = get_dram_regions_info();

#ifdef CONFIG_OCRAM_ECC_EN
	ocram_init(NXP_OCRAM_ADDR, NXP_OCRAM_SIZE);
#endif
	dcfg_init(&dcfg_init_data);
#ifdef POLICY_FUSE_PROVISION
	gpio_init(&gpio_init_data);
	sec_init(NXP_CAAM_ADDR);
#endif
#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
			  NXP_UART_CLK_DIVIDER,
			  NXP_CONSOLE_BAUDRATE);
#endif
	set_base_freq_CNTFID0();

	/* Enable snooping on SEC read and write transactions */
	scfg_setbits32((void *)(NXP_SCFG_ADDR + SCFG_SNPCNFGCR_OFFSET),
			SCFG_SNPCNFGCR_SECRDSNP | SCFG_SNPCNFGCR_SECWRSNP);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_ls_interconnect_enter_coherency(get_num_cluster());

#if TRUSTED_BOARD_BOOT
	sfp_init(NXP_SFP_ADDR);

	/*
	 * For Mbedtls currently crypto is not supported via CAAM
	 * enable it when that support is there. In tbbr.mk
	 * the CAAM_INTEG is set as 0.
	 */
#ifndef MBEDTLS_X509
	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
	}
#endif
#elif defined(POLICY_FUSE_PROVISION)
	sfp_init(NXP_SFP_ADDR);
#endif

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init(NXP_TIMER_ADDR);
#ifdef DDR_INIT
	dram_regions_info->total_dram_size = init_ddr();
#endif
}

void soc_bl2_prepare_exit(void)
{
#if defined(NXP_SFP_ENABLED) && defined(DISABLE_FUSE_WRITE)
	set_sfp_wr_disable();
#endif
}

/*
 * This function returns the boot device based on RCW_SRC
 * LS1012 supports only QSPI as boot source
 */
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_QSPI;

	INFO("BOOT SRC is QSPI\n");
	return src;
}

#else /* IMAGE_BL2 */

void soc_early_platform_setup2(void)
{
	dcfg_init(&dcfg_init_data);
	/*
	 * Initialize system level generic timer for Socs
	 */
	delay_timer_init(NXP_TIMER_ADDR);

#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init(NXP_CONSOLE_ADDR,
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif
}

void soc_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	static uint32_t target_mask_array[PLATFORM_CORE_COUNT];
	/*
	 * On a GICv2 system, the Group 1 secure interrupts are treated
	 * as Group 0 interrupts.
	 */
	static interrupt_prop_t ls_interrupt_props[] = {
		PLAT_LS_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
		PLAT_LS_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
	};

	plat_ls_gic_driver_init(
				NXP_GICD_4K_ADDR,
				NXP_GICC_4K_ADDR,
				PLATFORM_CORE_COUNT,
				ls_interrupt_props,
				ARRAY_SIZE(ls_interrupt_props),
				target_mask_array);

	plat_ls_gic_init();
	enable_init_timer();
}

/*
 * This function initializes the soc from the BL31 module
 */
void soc_init(void)
{
	 /* low-level init of the soc */
	soc_init_lowlevel();
	_init_global_data();
	soc_init_percpu();
	_initialize_psci();

	/*
	 * Initialize the interconnect during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	cci_init(NXP_CCI_ADDR, cci_map, ARRAY_SIZE(cci_map));

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this but we can't
	 * assume so. No harm in executing this code twice.
	 */
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr()));

	/* Init CSU to enable non-secure access to peripherals */
	enable_layerscape_ns_access(ns_dev, ARRAY_SIZE(ns_dev), NXP_CSU_ADDR);

	/* Initialize the crypto accelerator if enabled */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
	}
}

void soc_runtime_setup(void)
{

}
#endif /* IMAGE_BL2 */

/*
 * Function to return the SoC SYS CLK
 */
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

/*
 * Function returns the base counter frequency
 * after reading the first entry at CNTFID0 (0x20 offset).
 *
 * Function is used by:
 *   1. ARM common code for PSCI management.
 *   2. ARM Generic Timer init.
 */
unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;
	/*
	 * Below register specifies the base frequency of the system counter.
	 * As per NXP Board Manuals:
	 * The system counter always works with 25000000Hz clock.
	 */
	counter_base_frequency = U(25000000);

	return counter_base_frequency;
}

/*
 * This function sets up access permissions on memory regions
 */
void soc_mem_access(void)
{
	/* Nothing to do for ls1012 as there is no TZASC block */
}

/*
 * This function sets up DTB address to be passed to next boot stage
 */
void plat_set_dt_address(entry_point_info_t *image_info)
{
	image_info->args.arg3 = BL32_FDT_OVERLAY_ADDR;
}
