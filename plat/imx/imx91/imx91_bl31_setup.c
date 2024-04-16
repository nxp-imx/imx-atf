/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/nxp/trdc/imx_trdc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <ele_api.h>
#include <dram.h>
#include <imx8_lpuart.h>
#include <plat_imx8.h>
#include <platform_def.h>

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

static const mmap_region_t imx_mmap[] = {
	/* APIS2 mapping */
	MAP_REGION_FLAT(AIPS2_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(AIPS3_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(AIPS1_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW), /* ECO fix , secure can access nonsecure */
	/* AIPS4 */
	MAP_REGION_FLAT(AIPS4_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW | MT_NS),

	MAP_REGION_FLAT(PLAT_GICD_BASE, 0x200000, MT_DEVICE | MT_RW), /* ECO fix, secure can access nonsecure */

	MAP_REGION_FLAT(TRDC_A_BASE, TRDC_x_SISE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(TRDC_W_BASE, TRDC_x_SISE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(TRDC_M_BASE, TRDC_x_SISE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(TRDC_N_BASE, TRDC_x_SISE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(FSB_BASE, 0x10000, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(S400_MU_BASE, 0x10000, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(DDRMIX_BASE, DDRMIX_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GPIO_BASE, GPIO_SIZE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(NIC_MAIN_GPV_BASE, 0x200000, MT_DEVICE | MT_RW),

	{0},
};

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

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

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	static console_t console;

	console_lpuart_register(IMX_LPUART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);

	/* This console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT);

	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if defined(SPD_opteed) || defined(SPD_trusty)
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;

	/* Pass TEE base and size to bl33 */
	bl33_image_ep_info.args.arg1 = BL32_BASE;
	bl33_image_ep_info.args.arg2 = BL32_SIZE;

#ifdef SPD_trusty
	bl32_image_ep_info.args.arg0 = BL32_SIZE;
	bl32_image_ep_info.args.arg1 = BL32_BASE;
#else
	/* Make sure memory is clean */
	mmio_write_32(BL32_FDT_OVERLAY_ADDR, 0);
	bl33_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
	bl32_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
#endif
#endif
}

void bl31_plat_arch_setup(void)
{
	/* Assign all the GPIO pins to non-secure world by default */
	mmio_write_32(GPIO2_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO2_BASE + 0x14, 0x3);
	mmio_write_32(GPIO2_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO2_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO3_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO3_BASE + 0x14, 0x3);
	mmio_write_32(GPIO3_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO3_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO4_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO4_BASE + 0x14, 0x3);
	mmio_write_32(GPIO4_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO4_BASE + 0x1c, 0x3);

	mmio_write_32(GPIO1_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO1_BASE + 0x14, 0x3);
	mmio_write_32(GPIO1_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO1_BASE + 0x1c, 0x3);

	mmap_add_region(OCRAM_BASE, OCRAM_BASE, OCRAM_SIZE,
		MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL31_BASE, BL31_BASE, (BL31_LIMIT - BL31_BASE),
		MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE, (BL_CODE_END - BL_CODE_BASE),
		MT_MEMORY | MT_RO | MT_SECURE);

#ifdef SPD_trusty
	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW);
#endif

	mmap_add(imx_mmap);

	init_xlat_tables();

	enable_mmu_el3(0);

	trdc_config();
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	/* get soc info */
	ele_get_soc_info();

	/* Init the dram info */
	dram_info_init(SAVED_DRAM_TIMING_BASE);

	plat_gic_driver_init();
	plat_gic_init();
}

void bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);

	return;
}


entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	}

	if (type == SECURE) {
		return &bl32_image_ep_info;
	}

	return NULL;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args) {
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
