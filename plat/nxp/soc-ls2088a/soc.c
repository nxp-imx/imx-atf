/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include "caam.h"
#include <ccn.h>
#include <common/debug.h>
#include <dcfg.h>
#ifdef I2C_INIT
#include <i2c.h>
#endif
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include "ls_interconnect.h"
#ifdef POLICY_FUSE_PROVISION
#include <nxp_gpio.h>
#endif
#include <nxp_smmu.h>
#include <nxp_timer.h>
#include <plat_console.h>
#include <plat_gic.h>
#include <plat_tzc400.h>
#include <pmu.h>
#if defined(NXP_SFP_ENABLED)
#include <sfp.h>
#endif

#include <errata.h>
#ifdef CONFIG_OCRAM_ECC_EN
#include <ocram.h>
#endif
#include "plat_common.h"
#include "platform_def.h"
#include "soc.h"

static struct soc_type soc_list[] =  {
	SOC_ENTRY(LS2080A, LS2080A, 4, 2),
	SOC_ENTRY(LS2080AE, LS2080AE, 4, 2),
	SOC_ENTRY(LS2088A, LS2088A, 4, 2),
	SOC_ENTRY(LS2088AE, LS2088AE, 4, 2),
	SOC_ENTRY(LS2084A, LS2084A, 4, 2),
	SOC_ENTRY(LS2084AE, LS2084AE, 4, 2),
	SOC_ENTRY(LS2081A, LS2081A, 4, 2),
	SOC_ENTRY(LS2048A, LS2048A, 2, 2),
	SOC_ENTRY(LS2048AE, LS2048AE, 2, 2),
	SOC_ENTRY(LS2044A, LS2044A, 2, 2),
	SOC_ENTRY(LS2044AE, LS2044AE, 2, 2),
	SOC_ENTRY(LS2041A, LS2041A, 2, 2),
	SOC_ENTRY(LS2040A, LS2040A, 2, 2),
	SOC_ENTRY(LS2040AE, LS2040AE, 2, 2),
};

static dcfg_init_info_t dcfg_init_data = {
	.g_nxp_dcfg_addr = NXP_DCFG_ADDR,
	.nxp_sysclk_freq = NXP_SYSCLK_FREQ,
	.nxp_ddrclk_freq = NXP_DDRCLK_FREQ,
	.nxp_plat_clk_divider = NXP_PLATFORM_CLK_DIVIDER,
};

static const unsigned char master_to_2rn_id_map[] = {
	PLAT_2CLUSTER_TO_CCN_ID_MAP
};

static const unsigned char master_to_rn_id_map[] = {
	PLAT_CLUSTER_TO_CCN_ID_MAP
};

static const ccn_desc_t plat_two_cluster_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_2rn_id_map),
	.master_to_rn_id_map = master_to_2rn_id_map
};

static const ccn_desc_t plat_ccn_desc = {
	.periphbase = NXP_CCN_ADDR,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

CASSERT(NUMBER_OF_CLUSTERS && NUMBER_OF_CLUSTERS <= 256,
		assert_invalid_ls2088a_cluster_count);

/* This function returns the total number of cores in the SoC */
unsigned int get_tot_num_cores(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);

	return (num_clusters * cores_per_cluster);
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
	 * The system counter always works with SYS_REF_CLK/4 frequency clock.
	 */
	counter_base_frequency = mmio_read_32(NXP_TIMER_ADDR + CNTFID_OFF);

	return counter_base_frequency;
}

/* Functions used by BL2 */
#ifdef IMAGE_BL2

#ifdef POLICY_FUSE_PROVISION
static gpio_init_info_t gpio_init_data = {
	.gpio1_base_addr = NXP_GPIO1_ADDR,
	.gpio2_base_addr = NXP_GPIO2_ADDR,
	.gpio3_base_addr = NXP_GPIO3_ADDR,
	.gpio4_base_addr = NXP_GPIO4_ADDR,
};
#endif

static void soc_interconnect_config(void)
{
	unsigned long long val = 0x0;
	uint32_t rni_map[] = {0, 2, 6, 12, 16, 20};
	uint32_t idx = 0;
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);

	if (num_clusters == 2) {
		ccn_init(&plat_two_cluster_ccn_desc);
	} else {
		ccn_init(&plat_ccn_desc);
	}

	/* Enable Interconnect coherency for the primary CPU's cluster.*/
	plat_ls_interconnect_enter_coherency(num_clusters);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, PCIeRC_RN_I_NODE_ID_OFFSET);
	val |= (1 << 12);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, PCIeRC_RN_I_NODE_ID_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(ENABLE_RESERVE_BIT53);
	val |= SERIALIZE_DEV_nGnRnE_WRITES;
	ccn_write_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, PoS_CONTROL_REG_OFFSET);
	val &= ~(HNI_POS_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, PoS_CONTROL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET);
	val &= ~(POS_EARLY_WR_COMP_EN);
	ccn_write_node_reg(NODE_TYPE_HNI, 10, SA_AUX_CTRL_REG_OFFSET, val);

#if POLICY_PERF_WRIOP
	val = ccn_read_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET, val);
#else
	val = ccn_read_node_reg(NODE_TYPE_RNI, 12, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_WUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 12, SA_AUX_CTRL_REG_OFFSET, val);
#endif

	val = ccn_read_node_reg(NODE_TYPE_RNI, 6, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_FORCE_RD_QUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 6, SA_AUX_CTRL_REG_OFFSET, val);

	val = ccn_read_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET);
	val |= ENABLE_FORCE_RD_QUO;
	ccn_write_node_reg(NODE_TYPE_RNI, 20, SA_AUX_CTRL_REG_OFFSET, val);

	/*
	 * RN-I map in MN Memory map is in-correctly set.
	 * Bit corresponding to Node Id 0 is not set.
	 * Hence loop below is started from 1.
	 */
	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S0_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S0_CTRL_REG_RNI,
			val);

	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S1_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S1_CTRL_REG_RNI,
			val);

	val = mmio_read_64(NXP_CCN_ADDR + (128 * 0x10000)
						+ PORT_S2_CTRL_REG_RNI);
	val |= QOS_SETTING;
	mmio_write_64(NXP_CCN_ADDR + (128 * 0x10000) + PORT_S2_CTRL_REG_RNI,
			val);

	for (idx = 1; idx < (sizeof(rni_map)/sizeof(uint32_t)); idx++) {
		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S0_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S0_CTRL_REG_RNI, val);

		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S1_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S1_CTRL_REG_RNI, val);

		val = ccn_read_node_reg(NODE_TYPE_RNI, rni_map[idx],
							PORT_S2_CTRL_REG_RNI);
		val |= QOS_SETTING;
		ccn_write_node_reg(NODE_TYPE_RNI, rni_map[idx],
						PORT_S2_CTRL_REG_RNI, val);
	}
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
	enum  boot_device dev = get_boot_dev();
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
				NXP_UART_CLK_DIVIDER, NXP_CONSOLE_BAUDRATE);
#endif

	enable_timer_base_to_cluster(NXP_PMU_ADDR);
	enable_core_tb(NXP_PMU_ADDR);
	soc_interconnect_config();

	/*
	 * Mark the buffer for SD in OCRAM as non secure.
	 * The buffer is assumed to be at end of OCRAM for
	 * the logic below to calculate TZPC programming
	 */
	if (dev == BOOT_DEVICE_EMMC) {
		/*
		 * Calculate the region in OCRAM which is secure
		 * The buffer for SD needs to be marked non-secure
		 * to allow SD to do DMA operations on it
		 */
		uint32_t secure_region = (NXP_OCRAM_SIZE
						- NXP_SD_BLOCK_BUF_SIZE);
		uint32_t mask = secure_region/TZPC_BLOCK_SIZE;

		mmio_write_32(NXP_OCRAM_TZPC_ADDR, mask);

		/* Add the entry for buffer in MMU Table */
		mmap_add_region(NXP_SD_BLOCK_BUF_ADDR, NXP_SD_BLOCK_BUF_ADDR,
				NXP_SD_BLOCK_BUF_SIZE,
				MT_DEVICE | MT_RW | MT_NS);
	}

	soc_errata();

#if (TRUSTED_BOARD_BOOT) || defined(POLICY_FUSE_PROVISION)
	sfp_init(NXP_SFP_ADDR);
#endif

    /*
     * Unlock write access for SMMU SMMU_CBn_ACTLR in all Non-secure contexts.
     */
    smmu_cache_unlock(NXP_SMMU_ADDR);
    INFO("SMMU Cache Unlocking is Configured.\n");

#if TRUSTED_BOARD_BOOT
	uint32_t mode;

	/*
	 * For secure boot disable SMMU.
	 * Later when platform security policy comes in picture,
	 * this might get modified based on the policy
	 */
	if (check_boot_mode_secure(&mode) == true) {
		bypass_smmu(NXP_SMMU_ADDR);
	}

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
#endif

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init(NXP_TIMER_ADDR);

#ifdef DDR_INIT
	i2c_init(NXP_I2C_ADDR);
	dram_regions_info->total_dram_size = init_ddr();
#endif
}

void soc_bl2_prepare_exit(void)
{
#if defined(NXP_SFP_ENABLED) && defined(DISABLE_FUSE_WRITE)
	set_sfp_wr_disable();
#endif
}

/* This function returns the boot device based on RCW_SRC */
enum boot_device get_boot_dev(void)
{
	enum boot_device src = BOOT_DEVICE_NONE;
	uint32_t porsr1;
	uint32_t rcw_src, val;

	porsr1 = read_reg_porsr1();

	rcw_src = (porsr1 & PORSR1_RCW_MASK) >> PORSR1_RCW_SHIFT;

	/* RCW SRC NOR */
	val = rcw_src & RCW_SRC_TYPE_MASK;
	if (val == NOR_16B_VAL) {
		src = BOOT_DEVICE_IFC_NOR;
		INFO("RCW BOOT SRC is IFC NOR\n");
	} else {
		val = rcw_src & RCW_SRC_SERIAL_MASK;
		switch (val) {
		case QSPI_VAL:
			src = BOOT_DEVICE_QSPI;
			INFO("RCW BOOT SRC is QSPI\n");
			break;
		case SDHC_VAL:
			src = BOOT_DEVICE_EMMC;
			INFO("RCW BOOT SRC is SD/EMMC\n");
			break;
		case EMMC_VAL:
			src = BOOT_DEVICE_EMMC;
			INFO("RCW BOOT SRC is SD/EMMC\n");
			break;
		default:
			src = BOOT_DEVICE_NONE;
		}
	}

	return src;
}

void soc_mem_access(void)
{
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	struct tzc400_reg tzc400_reg_list[MAX_NUM_TZC_REGION];
	int dram_idx, index = 0;

	for (dram_idx = 0; dram_idx < info_dram_regions->num_dram_regions;
			dram_idx++) {
		if (info_dram_regions->region[dram_idx].size == 0) {
			ERROR("DDR init failure, or");
			ERROR("DRAM regions not populated correctly.\n");
			break;
		}

		index = populate_tzc400_reg_list(tzc400_reg_list,
				dram_idx, index,
				info_dram_regions->region[dram_idx].addr,
				info_dram_regions->region[dram_idx].size,
				NXP_SECURE_DRAM_SIZE, NXP_SP_SHRD_DRAM_SIZE);
	}

	mem_access_setup(NXP_TZC_ADDR, index, tzc400_reg_list);
	mem_access_setup(NXP_TZC2_ADDR, index, tzc400_reg_list);
}

#else /* IMAGE_BL2 */

static unsigned char _power_domain_tree_desc[NUMBER_OF_CLUSTERS + 2];

/* This function returns the PMU IDLE Cluster mask. */
unsigned int get_pmu_idle_cluster_mask(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);

	return ((1 << num_clusters) - 2);
}

/* This function returns the PMU Flush Cluster mask. */
unsigned int get_pmu_flush_cluster_mask(void)
{
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);

	return ((1 << num_clusters) - 2);
}

/* This function returns the PMU IDLE Core mask.*/
unsigned int get_pmu_idle_core_mask(void)
{
	return ((1 << get_tot_num_cores()) - 2);
}

/*
 * This function dynamically constructs the topology according to
 *  SoC Flavor and returns it.
 */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i;
	uint8_t num_clusters, cores_per_cluster;

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);

	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	_power_domain_tree_desc[0] = 1;
	_power_domain_tree_desc[1] = num_clusters;

	for (i = 0; i < _power_domain_tree_desc[1]; i++)
		_power_domain_tree_desc[i + 2] = cores_per_cluster;

	return _power_domain_tree_desc;
}

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
	static uintptr_t target_mask_array[PLATFORM_CORE_COUNT];
	static interrupt_prop_t ls_interrupt_props[] = {
		PLAT_LS_G1S_IRQ_PROPS(INTR_GROUP1S),
		PLAT_LS_G0_IRQ_PROPS(INTR_GROUP0)
	};

	plat_ls_gic_driver_init(NXP_GICD_ADDR, NXP_GICR_ADDR,
				PLATFORM_CORE_COUNT,
				ls_interrupt_props,
				ARRAY_SIZE(ls_interrupt_props),
				target_mask_array,
				plat_core_pos);

	plat_ls_gic_init();
	enable_init_timer();
#ifdef LS_SYS_TIMCTL_BASE
	ls_configure_sys_timer(LS_SYS_TIMCTL_BASE,
			       LS_CONFIG_CNTACR,
			       PLAT_LS_NSTIMER_FRAME_ID);
#endif
}

/* This function initializes the soc from the BL31 module */
void soc_init(void)
{
	uint8_t num_clusters, cores_per_cluster;

	/* low-level init of the soc */
	soc_init_lowlevel();
	_init_global_data();
	soc_init_percpu();
	_initialize_psci();

#ifndef TEST_BL31
	if (ccn_get_part0_id(NXP_CCN_ADDR) != CCN_504_PART0_ID) {
		ERROR("Unrecognized CCN variant detected. Only CCN-504 is supported");
		panic();
	}
#endif

	get_cluster_info(soc_list, ARRAY_SIZE(soc_list), &num_clusters,
			&cores_per_cluster);
	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	if (num_clusters == 2)
		ccn_init(&plat_two_cluster_ccn_desc);
	else
		ccn_init(&plat_ccn_desc);

	plat_ls_interconnect_enter_coherency(num_clusters);

	/* Set platform security policies */
	_set_platform_security();

	/* Init SEC Engine which will be used by SiP */
	if (is_sec_enabled() == false) {
		INFO("SEC is disabled.\n");
	} else {
		sec_init(NXP_CAAM_ADDR);
	}
}

/* Function to return the SoC SYS CLK */
unsigned int get_sys_clk(void)
{
	return NXP_SYSCLK_FREQ;
}

void soc_runtime_setup(void)
{

}

#endif /* IMAGE_BL2 */

/*
 * This function sets up DTB address to be passed to next boot stage
 */
void plat_set_dt_address(entry_point_info_t *image_info)
{
	image_info->args.arg3 = BL32_FDT_OVERLAY_ADDR;
}
