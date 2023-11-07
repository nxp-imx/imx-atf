/*
 * Copyright 2022 NXP
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

#include <ele_api.h>
#include <imx8_lpuart.h>
#include <platform_def.h>
#include <plat_imx8.h>

extern void imx9_init_scmi_server();

static const mmap_region_t imx_mmap[] = {
	/* APIS2 mapping  */
	MAP_REGION_FLAT(AIPS2_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(PLAT_GICD_BASE, 0x200000, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(AIPS1_BASE, AIPSx_SIZE, MT_DEVICE | MT_RW),
	/* GPIO2-5 */
	MAP_REGION_FLAT(GPIO2_BASE, 0x20000, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(GPIO4_BASE, 0x20000, MT_DEVICE | MT_RW),
	MAP_REGION_FLAT(ELE_MU_BASE, 0x10000, MT_DEVICE | MT_RW),

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
	console_set_scope(&console, CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);

	/*
	 * tell BL3-1 where the non-secure software image is located
	 * and the entry state information.
	 */
	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
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

	mmio_write_32(GPIO5_BASE + 0x10, 0xffffffff);
	mmio_write_32(GPIO5_BASE + 0x14, 0x3);
	mmio_write_32(GPIO5_BASE + 0x18, 0xffffffff);
	mmio_write_32(GPIO5_BASE + 0x1c, 0x3);

	mmap_add_region(BL31_BASE, BL31_BASE, (BL31_LIMIT - BL31_BASE),
		MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE, (BL_CODE_END - BL_CODE_BASE),
		MT_MEMORY | MT_RO | MT_SECURE);

	mmap_add(imx_mmap);

	init_xlat_tables();

	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	generic_delay_timer_init();

	plat_gic_driver_init();
	/* Ensure to mark the core as asleep, required for reset case. */
	plat_gic_cpuif_disable();
	plat_gic_init();

	/* get soc info */
	ele_get_soc_info();

	extern void plat_imx95_setup(void);
	plat_imx95_setup();
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
