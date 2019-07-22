/*
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <stdbool.h>
#include <dram.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_imx8.h>
#include <xlat_tables.h>
#include <soc.h>
#include <tzc380.h>
#include <imx_csu.h>
#include <imx_rdc.h>
#include <uart.h>

IMPORT_SYM(unsigned long, __COHERENT_RAM_START__, BL31_COHERENT_RAM_START);
IMPORT_SYM(unsigned long, __COHERENT_RAM_END__, BL31_COHERENT_RAM_END);
IMPORT_SYM(unsigned long, __RO_START__, BL31_RO_START);
IMPORT_SYM(unsigned long, __RO_END__, BL31_RO_END);
IMPORT_SYM(unsigned long, __RW_START__, BL31_RW_START);
IMPORT_SYM(unsigned long, __RW_END__, BL31_RW_END);

#define CAAM_BASE       (0x30900000) /* HW address*/

#define JR0_BASE        (CAAM_BASE + 0x1000)

#define CAAM_JR0MID		(0x30900010)
#define CAAM_JR1MID		(0x30900018)
#define CAAM_JR2MID		(0x30900020)
#define CAAM_NS_MID		(0x1)

#define SM_P0_PERM      (JR0_BASE + 0xa04)
#define SM_P0_SMAG2     (JR0_BASE + 0xa08)
#define SM_P0_SMAG1     (JR0_BASE + 0xa0c)
#define SM_CMD          (JR0_BASE + 0xbe4)

/* secure memory command */
#define SMC_PAGE_SHIFT	16
#define SMC_PART_SHIFT	8

#define SMC_CMD_ALLOC_PAGE	0x01	/* allocate page to this partition */
#define SMC_CMD_DEALLOC_PART	0x03	/* deallocate partition */

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

#define IMX_DDR_BASE 0x40000000

#if defined(DECRYPTED_BUFFER_START) && defined(DECRYPTED_BUFFER_LEN)
#define DECRYPTED_BUFFER_END   	DECRYPTED_BUFFER_START + DECRYPTED_BUFFER_LEN
#endif

#if defined(DECODED_BUFFER_START) && defined(DECODED_BUFFER_LEN)
#define DECODED_BUFFER_END		DECODED_BUFFER_START + DECODED_BUFFER_LEN
#endif

#if !defined(DECRYPTED_BUFFER_END) && !defined(DECODED_BUFFER_END)
#define RDC_DISABLED
#else
static struct rdc_mda_conf masters_config[] = {
	{RDC_MDA_A53, 0, 1},
	{RDC_MDA_GPU, 1, 1},
	{RDC_MDA_VPU_DEC, 2, 1},
	{RDC_MDA_CAAM, 3, 1},
};
#endif

static struct csu_sa_conf sa_configs[] = {
	{CSU_SA_VPU, 0, 1},
	{CSU_SA_GPU, 0, 1},
	{CSU_SA_DCSS, 0, 1},
	{CSU_SA_M4, 0, 1},
	{CSU_SA_SDMA1, 0, 1},
	{CSU_SA_CSI, 0, 1},
	{CSU_SA_USB, 0, 1},
	{CSU_SA_PCIE, 0, 1},
	{CSU_SA_ENET, 0, 1},
	{CSU_SA_USDHC1, 0, 1},
	{CSU_SA_USDHC2, 0, 1},
	{CSU_SA_DAP, 0, 1},
};
static struct csu_slave_conf csu_csl_conf[] = {
	{CSU_CSLn_CAAM, CSU_SURW|CSU_SSRW, 1},
};

/* set RDC settings */
static void bl31_imx_rdc_setup(void)
{
#ifdef RDC_DISABLED
	NOTICE("RDC off \n");
#else
	struct imx_rdc_regs *imx_rdc = (struct imx_rdc_regs *)IMX_RDC_BASE;

	NOTICE("RDC imx_rdc_set_masters default \n");
	imx_rdc_set_masters(masters_config, ARRAY_SIZE(masters_config));

	/*
	 * Need to substact offset 0x40000000 from CPU address when
	 * programming rdc region for i.mx8mq.
	 */

#ifdef DECRYPTED_BUFFER_START
	/* Domain 2 no write access to memory region below decrypted video */
	/* Prevent VPU to decode outside secure decoded buffer */
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[2].mrsa), 0);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[2].mrea), (DECRYPTED_BUFFER_START - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[2].mrc), 0xC00000AF);
#endif // DECRYPTED_BUFFER_START

#ifdef DECRYPTED_BUFFER_END
	NOTICE("RDC setup memory_region[0] decrypted buffer DID0 W DID2 R/W\n");
	/* Domain 0 memory region W decrypted video */
	/* Domain 2 memory region R decrypted video */
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[0].mrsa), (DECRYPTED_BUFFER_START - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[0].mrea), (DECRYPTED_BUFFER_END - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[0].mrc), 0xC0000061);
#endif // DECRYPTED_BUFFER_END

#ifdef DECODED_BUFFER_END
	NOTICE("RDC setup memory_region[1] decoded buffer DID2 R/W DID3 R/W\n");
	/* Domain 1+2 memory region R/W decoded video */
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[1].mrsa), (DECODED_BUFFER_START - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[1].mrea), (DECODED_BUFFER_END - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[1].mrc), 0xC000003D);

	/* Domain 1+2+3 no access to memory region above decoded video */
	/* Only CPU in secure mode can access TEE memory region (cf TZASC configuration) */
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[3].mrsa), (DECODED_BUFFER_END - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[3].mrea), (0xC0000000 - IMX_DDR_BASE) >> 1);
	mmio_write_32((uintptr_t)&(imx_rdc->mem_region[3].mrc), 0xC00000C3);
#endif // DECODED_BUFFER_END

#endif // RDC_DISABLED
}

/* get SPSR for BL33 entry */
static uint32_t get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned long mode;
	uint32_t spsr;

	/* figure out what mode we enter the non-secure world */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

#define SCTR_BASE_ADDR		0x306c0000
#define CNTFID0_OFF		0x20
#define CNTFID1_OFF		0x24

#define SC_CNTCR_ENABLE         (1 << 0)
#define SC_CNTCR_HDBG           (1 << 1)
#define SC_CNTCR_FREQ0          (1 << 8)
#define SC_CNTCR_FREQ1          (1 << 9)

#define GPR_TZASC_EN		(1 << 0)
#define GPR_TZASC_EN_LOCK	(1 << 16)

void bl31_tzc380_setup(void)
{
	unsigned int val;

	val = mmio_read_32(IMX_IOMUX_GPR_BASE + 0x28);
	if ((val & GPR_TZASC_EN) != GPR_TZASC_EN)
		return;

	NOTICE("Configuring TZASC380\n");

	tzc380_init(IMX_TZASC_BASE);

	/*
	 * Need to substact offset 0x40000000 from CPU address when
	 * programming tzasc region for i.mx8mq.
	 */

#ifdef SPD_trusty
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G) | TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_NS_RW);
	tzc380_configure_region(1, (BL32_BASE - IMX_DDR_BASE), TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_32M) | TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_S_RW);
#else
	/* Enable 1G-5G S/NS RW */
	tzc380_configure_region(0, 0x00000000, TZC_ATTR_REGION_SIZE(TZC_REGION_SIZE_4G) | TZC_ATTR_REGION_EN_MASK | TZC_ATTR_SP_ALL);
#endif

	tzc380_dump_state();
}

static void imx8mm_caam_config(void)
{
	uint32_t sm_cmd;

	/* Dealloc part 0 and 2 with current DID */
	sm_cmd = (0 << SMC_PART_SHIFT | SMC_CMD_DEALLOC_PART);
	mmio_write_32(SM_CMD, sm_cmd);

	sm_cmd = (2 << SMC_PART_SHIFT | SMC_CMD_DEALLOC_PART);
	mmio_write_32(SM_CMD, sm_cmd);

	/* config CAAM JRaMID set MID to Cortex A */
	mmio_write_32(CAAM_JR0MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR1MID, CAAM_NS_MID);
	mmio_write_32(CAAM_JR2MID, CAAM_NS_MID);

	/* Alloc partition 0 writing SMPO and SMAGs */
	mmio_write_32(SM_P0_PERM, 0xff);
	mmio_write_32(SM_P0_SMAG2, 0xffffffff);
	mmio_write_32(SM_P0_SMAG1, 0xffffffff);

	/* Allocate page 0 and 1 to partition 0 with DID set */
	sm_cmd = (0 << SMC_PAGE_SHIFT
					| 0 << SMC_PART_SHIFT
					| SMC_CMD_ALLOC_PAGE);
	mmio_write_32(SM_CMD, sm_cmd);

	sm_cmd = (1 << SMC_PAGE_SHIFT
					| 0 << SMC_PART_SHIFT
					| SMC_CMD_ALLOC_PAGE);
	mmio_write_32(SM_CMD, sm_cmd);

}
static void imx8mm_aips_config(void)
{
	/* config the AIPSTZ1 */
	mmio_write_32(0x301f0000, 0x77777777);
	mmio_write_32(0x301f0004, 0x77777777);
	mmio_write_32(0x301f0040, 0x0);
	mmio_write_32(0x301f0044, 0x0);
	mmio_write_32(0x301f0048, 0x0);
	mmio_write_32(0x301f004c, 0x0);
	mmio_write_32(0x301f0050, 0x0);

	/* config the AIPSTZ2 */
	mmio_write_32(0x305f0000, 0x77777777);
	mmio_write_32(0x305f0004, 0x77777777);
	mmio_write_32(0x305f0040, 0x0);
	mmio_write_32(0x305f0044, 0x0);
	mmio_write_32(0x305f0048, 0x0);
	mmio_write_32(0x305f004c, 0x0);
	mmio_write_32(0x305f0050, 0x0);

	/* config the AIPSTZ3 */
	mmio_write_32(0x309f0000, 0x77777777);
	mmio_write_32(0x309f0004, 0x77777777);
	mmio_write_32(0x309f0040, 0x0);
	mmio_write_32(0x309f0044, 0x0);
	mmio_write_32(0x309f0048, 0x0);
	mmio_write_32(0x309f004c, 0x0);
	mmio_write_32(0x309f0050, 0x0);

	/* config the AIPSTZ4 */
	mmio_write_32(0x32df0000, 0x77777777);
	mmio_write_32(0x32df0004, 0x77777777);
	mmio_write_32(0x32df0040, 0x0);
	mmio_write_32(0x32df0044, 0x0);
	mmio_write_32(0x32df0048, 0x0);
	mmio_write_32(0x32df004c, 0x0);
	mmio_write_32(0x32df0050, 0x0);
}

void bl31_setup_secure_policy() {
	csu_set_slaves_modes(csu_csl_conf, (uint32_t)ARRAY_SIZE(csu_csl_conf));
	csu_set_sa_configs(sa_configs, (uint32_t)ARRAY_SIZE(sa_configs));
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
#if DEBUG_CONSOLE
	static console_uart_t console;
#endif

	int i;
	/* enable CSU NS access permission */
	for (i = 0; i < 64; i++) {
		mmio_write_32(0x303e0000 + i * 4, 0x00ff00ff);
	}

	/* config the aips access permission */
	imx8mm_aips_config();

	/* config the caam access permission */
	imx8mm_caam_config();

#if DEBUG_CONSOLE
	console_uart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		IMX_CONSOLE_BAUDRATE, &console);
#endif

	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#ifdef TEE_IMX8
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;
#ifdef SPD_trusty
	bl32_image_ep_info.args.arg0 = BL32_SIZE;
	bl32_image_ep_info.args.arg1 = BL32_BASE;
#else
	/* Pass TEE base and size to uboot */
	bl33_image_ep_info.args.arg1 = 0xBE000000;
#endif
	/* TEE size + RDC reserved memory = 0x2000000 + 0x2000000 + 0x30000000 */
#ifdef DECRYPTED_BUFFER_START
	bl33_image_ep_info.args.arg2 = 0xC0000000 - DECRYPTED_BUFFER_START;
#else
	bl33_image_ep_info.args.arg2 = 0x2000000;
#endif
#endif

	bl31_tzc380_setup();

	/* Assign M4 to domain 1 */
	mmio_write_32(IMX_RDC_BASE + 0x204, 0x1);
	mmio_write_32(IMX_RDC_BASE + 0x518, 0xfc);
	mmio_write_32(IMX_RDC_BASE + 0x5A4, 0xf3);

#if defined (CSU_RDC_TEST)
	csu_test();
	rdc_test();
#endif

	bl31_imx_rdc_setup();

	bl31_setup_secure_policy();

}

void bl31_plat_arch_setup(void)
{
	/* add the mmap */
	mmap_add_region(0x900000, 0x900000, 0x40000,
			MT_MEMORY | MT_RW);
	mmap_add_region(0x100000, 0x100000, 0x10000,
			MT_MEMORY | MT_RW);
	mmap_add_region(0x40000000, 0x40000000, 0xc0000000,
			MT_MEMORY | MT_RW | MT_NS);

	mmap_add_region(BL31_BASE, BL31_BASE, BL31_RO_END - BL31_RO_START,
			MT_MEMORY | MT_RO);

	mmap_add_region(IMX_ROM_BASE, IMX_ROM_BASE,
			0x40000, MT_MEMORY | MT_RO);
	/* Map GPV */
	mmap_add_region(IMX_GPV_BASE, IMX_GPV_BASE, IMX_GPV_SIZE, MT_DEVICE | MT_RW);
	/* Map AIPS 1~3 */
	mmap_add_region(IMX_AIPS_BASE, IMX_AIPS_BASE, IMX_AIPS_SIZE, MT_DEVICE | MT_RW);
	/* map AIPS4 */
	mmap_add_region(0x32c00000, 0x32c00000, 0x400000, MT_DEVICE | MT_RW);
	/* map GIC */
	mmap_add_region(PLAT_GIC_BASE, PLAT_GIC_BASE, 0x100000,  MT_DEVICE | MT_RW);

	/* Map DDRC/PHY/PERF */
	mmap_add_region(0x3c000000, 0x3c000000, 0x4000000, MT_DEVICE | MT_RW);

	mmap_add_region(0x180000, 0x180000, 0x8000, MT_MEMORY | MT_RW);

	mmap_add_region(0x38330000, 0x38330000, 0x100000, MT_DEVICE | MT_RW);

#ifdef SPD_trusty
	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW);
#endif

#if USE_COHERENT_MEM
	mmap_add_region(BL31_COHERENT_RAM_BASE, BL31_COHERENT_RAM_BASE,
		BL31_COHERENT_RAM_LIMIT - BL31_COHERENT_RAM_BASE,
		MT_DEVICE | MT_RW);
#endif
	/* setup xlat table */
	init_xlat_tables();

	/* enable the MMU */
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* select the CKIL source to 32K OSC */
	mmio_write_32(0x30360124, 0x1);

	/* init the dram info */
	dram_info_init(SAVED_DRAM_TIMING_BASE);

	/* init the GICv3 cpu and distributor interface */
	plat_gic_driver_init();
	plat_gic_init();

	/* gpc init */
	imx_gpc_init();

}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl31_plat_runtime_setup(void)
{
	return;
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
