/*
 * Copyright 2021 NXP
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
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <upower_soc_defs.h>
#include <upower_api.h>

#include <imx8_lpuart.h>
#include <imx8ulp_caam.h>
#include <plat_imx8.h>
#include <platform_def.h>
#include <upower_soc_defs.h>
#include <upower_api.h>

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW),
	/* For SCMI shared memory region */
	MAP_REGION_FLAT(0x2201f000, 0x1000, MT_RW | MT_DEVICE),
	{0}
};

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

	/* config the TPM5 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xd0, 0x92000000);
	mmio_write_32(IMX_PCC3_BASE + 0xd0, 0xd2000000);

	/* enable the GPIO D,E,F non-secure access by default */
	mmio_write_32(IMX_PCC4_BASE + 0x78, 0xc0000000);
	mmio_write_32(IMX_PCC4_BASE + 0x7c, 0xc0000000);
	mmio_write_32(IMX_PCC5_BASE + 0x114, 0xc0000000);
	mmio_write_32(0x2d000010, 0xffffffff);
	mmio_write_32(0x2d000014, 0x3);
	mmio_write_32(0x2d000018, 0xffffffff);
	mmio_write_32(0x2d00001c, 0x3);

	mmio_write_32(0x2d010010, 0xffffffff);
	mmio_write_32(0x2d010014, 0x3);
	mmio_write_32(0x2d010018, 0xffffffff);
	mmio_write_32(0x2d01001c, 0x3);

	mmio_write_32(0x2e200010, 0xffffffff);
	mmio_write_32(0x2e200014, 0x3);
	mmio_write_32(0x2e200018, 0xffffffff);
	mmio_write_32(0x2e20001c, 0x3);

	console_lpuart_register(IMX_LPUART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);

	/* This console is only used for boot stage */
	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);

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
	mmap_add_region(BL31_BASE, BL31_BASE, (BL31_LIMIT - BL31_BASE),
		MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE, (BL_CODE_END - BL_CODE_BASE),
		MT_MEMORY | MT_RO | MT_SECURE);
#if USE_COHERENT_MEM
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
		(BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE),
		MT_DEVICE | MT_RW | MT_SECURE);
#endif
	mmap_add(imx_mmap);

#if defined(SPD_opteed) || defined(SPD_trusty)
	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW);
#endif

	init_xlat_tables();

	enable_mmu_el3(0);

	/* TODO: Hack, refine this piece, scmi channel free */
	mmio_write_32(0x2201f004, 1);
}

extern uint32_t upower_init(void);
extern void imx8ulp_init_scmi_server(void);
void bl31_platform_setup(void)
{
	/* select the arch timer source */
	mmio_setbits_32(IMX_SIM1_BASE + 0x30, 0x8000000);

	generic_delay_timer_init();

	plat_gic_driver_init();
	plat_gic_init();

	imx8ulp_init_scmi_server();
	upower_init();
	imx8ulp_caam_init();
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
void plat_trusty_set_boot_args(aapcs64_params_t *args) {
	args->arg0 = BL32_SIZE;
	args->arg1 = BL32_BASE;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;
}
#endif
