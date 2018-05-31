/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cci.h>
#include <console.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_imx8.h>
#include <sci/sci.h>
#include <xlat_tables.h>
#include <lpuart.h>
#include <sec_rsrc.h>
#include <imx8-pins.h>
#include <iomux.h>

/* linker defined symbols */
#if USE_COHERENT_MEM
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#endif

sc_ipc_t ipc_handle;

#define BL31_RO_BASE	(unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT	(unsigned long)(&__RO_END__)
#define BL31_END	(unsigned long)(&__BL31_END__)

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

#if DEBUG_CONSOLE_A35
/*
 * Note:
 * This piece code is from uboot, take care of license issue.
 */
static void lpuart32_serial_setbrg(unsigned int base, int baudrate)
{
	unsigned int sbr, osr, baud_diff, tmp_osr, tmp_sbr, tmp_diff, tmp;
	unsigned int clk;
	sc_pm_clock_rate_t rate;

	sc_pm_get_clock_rate(ipc_handle, SC_R_UART_0, 2, &rate);

	clk = (unsigned int)rate;

	baud_diff = baudrate;
	osr = 0;
	sbr = 0;
	for (tmp_osr = 4; tmp_osr <= 32; tmp_osr++)
	{
		tmp_sbr = (clk / (baudrate * tmp_osr));
		if (tmp_sbr == 0)
			tmp_sbr = 1;

		/*calculate difference in actual buad w/ current values */
		tmp_diff = ( clk / (tmp_osr * tmp_sbr));
		tmp_diff = tmp_diff - baudrate;

		/* select best values between sbr and sbr+1 */
		if (tmp_diff > (baudrate - (clk / (tmp_osr * (tmp_sbr + 1))))) {
			tmp_diff = baudrate - (clk / (tmp_osr * (tmp_sbr + 1)));
			tmp_sbr++;
		}

		if (tmp_diff <= baud_diff) {
			baud_diff = tmp_diff;
			osr = tmp_osr;
			sbr = tmp_sbr;
		}
	}

	/*TODO handle buadrate outside acceptable rate
	 * if (baudDiff > ((config->baudRate_Bps / 100) * 3))
	 *  {
	 *    Unacceptable baud rate difference of more than 3%
	 *    return kStatus_LPUART_BaudrateNotSupport;
	 *    }
	 */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + BAUD);

	if ((osr >3) && (osr < 8))
		tmp |= LPUART_BAUD_BOTHEDGE_MASK;

	tmp &= ~LPUART_BAUD_OSR_MASK;
	tmp |= LPUART_BAUD_OSR(osr-1);
	tmp &= ~LPUART_BAUD_SBR_MASK;
	tmp |= LPUART_BAUD_SBR(sbr);

	/* explicitly disable 10 bit mode & set 1 stop bit */
	tmp &= ~(LPUART_BAUD_M10_MASK | LPUART_BAUD_SBNS_MASK);

	mmio_write_32(IMX_BOOT_UART_BASE + BAUD, tmp);
}

static int lpuart32_serial_init(unsigned int base)
{
	unsigned int tmp;

	/*disable TX & RX before enabling clocks */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(CTRL_TE | CTRL_RE);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + MODIR, 0);
	mmio_write_32(IMX_BOOT_UART_BASE + FIFO, ~(FIFO_TXFE | FIFO_RXFE));

	mmio_write_32(IMX_BOOT_UART_BASE + MATCH, 0);

	/*
	 * provide data bits, parity, stop bit, etc
	 */
	lpuart32_serial_setbrg(base, IMX_BOOT_UART_BAUDRATE);

	/* eight data bits no parity bit */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(LPUART_CTRL_PE_MASK | LPUART_CTRL_PT_MASK | LPUART_CTRL_M_MASK);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + CTRL , CTRL_RE | CTRL_TE);

	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x0A);

	return 0;
}
#endif

void imx8_partition_resources(void)
{
	sc_err_t err;
	sc_rm_pt_t secure_part, os_part;
	sc_rm_mr_t mr, mr_record = 64;
	bool owned;
	sc_faddr_t start, end;
	int i;

	err = sc_rm_get_partition(ipc_handle, &secure_part);

	err = sc_rm_partition_alloc(ipc_handle, &os_part, false, false,
		false, false, false);

	err = sc_rm_set_parent(ipc_handle, os_part, secure_part);

	/* set secure resources to NOT-movable */
	for (i = 0; i < (sizeof(secure_rsrcs) / sizeof(sc_rsrc_t)); i++) {
		err = sc_rm_set_resource_movable(ipc_handle,
			 secure_rsrcs[i], secure_rsrcs[i], false);
	}

	owned = sc_rm_is_resource_owned(ipc_handle, SC_R_M4_0_PID0);
	if (owned)
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, false);
	/* move all movable resources and pins to non-secure partition */
	err = sc_rm_move_all(ipc_handle, secure_part, os_part, true, true);
	if (owned) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, true);
		err = sc_rm_assign_resource(ipc_handle, os_part, SC_R_M4_0_PID0);
	}

	/* iterate through peripherals to give NS OS part access */
	for (i = 0; i< (sizeof(ns_access_allowed) / sizeof(sc_rsrc_t)); i++) {
		err = sc_rm_set_peripheral_permissions(ipc_handle,
			 ns_access_allowed[i], os_part, SC_RM_PERM_FULL);
	}

	/*
	 * sc_rm_set_peripheral_permissions
	 *
	 * sc_rm_set_memreg_permissions
	 *
	 * sc_rm_set_pin_movable
	 *
	 */

	for (mr = 0; mr < 64; mr++) {
		owned = sc_rm_is_memreg_owned(ipc_handle, mr);
		if (owned) {
			err = sc_rm_get_memreg_info(ipc_handle, mr, &start, &end);
			if (err) {
				ERROR("Memreg get info failed, %u\n", mr);
			} else {
				NOTICE("Memreg %u 0x%lx -- 0x%lx\n", mr, start, end);

				if (BL31_BASE >= start && (BL31_LIMIT - 1) <= end) {
					mr_record = mr; /* Record the mr for ATF running */
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%lx -- 0x%lx, err %d\n", start, end, err);
				}
			}
		}
	}

	if (mr_record != 64) {
		err = sc_rm_get_memreg_info(ipc_handle, mr_record, &start, &end);
		if (err) {
			ERROR("Memreg get info failed, %u\n", mr_record);
		} else {
			if ((BL31_LIMIT - 1) < end) {
				err = sc_rm_memreg_alloc(ipc_handle, &mr, BL31_LIMIT, end);
				if (err) {
					ERROR("sc_rm_memreg_alloc failed, 0x%lx -- 0x%lx\n", (sc_faddr_t)BL31_LIMIT, end);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%lx -- 0x%lx\n", (sc_faddr_t)BL31_LIMIT, end);
				}
			}

			if (start < (BL31_BASE - 1)) {
				err = sc_rm_memreg_alloc(ipc_handle, &mr, start, BL31_BASE - 1);
				if (err) {
					ERROR("sc_rm_memreg_alloc failed, 0x%lx -- 0x%lx\n", start, (sc_faddr_t)BL31_BASE - 1);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%lx -- 0x%lx\n", start, (sc_faddr_t)BL31_BASE - 1);
				}
			}
		}
	}

	if (err)
		NOTICE("Partitioning Failed\n");
	else
		NOTICE("Non-secure Partitioning Succeeded\n");
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
               u_register_t arg2, u_register_t arg3)
{
	/* open the IPC channel */
	if (sc_ipc_open(&ipc_handle, SC_IPC_CH) != SC_ERR_NONE) {
		/* No console available now */
		while (1);
	}
#if DEBUG_CONSOLE_A35
	/* Power up UART0 */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_UART_0, SC_PM_PW_MODE_ON);

	/* Set UART0 clock root to 80 MHz */
	sc_pm_clock_rate_t rate = 80000000;
	sc_pm_set_clock_rate(ipc_handle, SC_R_UART_0, 2, &rate);

	/* Enable UART0 clock root */
	sc_pm_clock_enable(ipc_handle, SC_R_UART_0, 2, true, false);

#define UART_PAD_CTRL	(PADRING_IFMUX_EN_MASK | PADRING_GP_EN_MASK | \
			(SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PD << PADRING_PULL_SHIFT))
	/* Configure UART pads */
	sc_pad_set(ipc_handle, SC_P_UART0_RX, UART_PAD_CTRL);

	sc_pad_set(ipc_handle, SC_P_UART0_TX, UART_PAD_CTRL);

	lpuart32_serial_init(IMX_BOOT_UART_BASE);
#endif

#if DEBUG_CONSOLE
	console_init(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE);
#endif

	/* Turn on MU1 for non-secure OS/Hypervisor
	*
	*/
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_MU_1A, SC_PM_PW_MODE_ON);

	/* create new partition for non-secure OS/Hypervisor
	 *
	 * uses global structs defined in sec_rsrc.h
	 */
	imx8_partition_resources();

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
	/*
	 * add the mmap
	 * Change to 128KB?
	 * Fix Me
	 */
	mmap_add_region(BL31_BASE, BL31_BASE, 0x10000,
			MT_MEMORY | MT_RW);
	mmap_add_region(BL31_BASE, BL31_BASE, BL31_RO_LIMIT - BL31_RO_BASE,
			MT_MEMORY | MT_RO);
	mmap_add_region(IMX_BOOT_UART_BASE, IMX_BOOT_UART_BASE,
			0x1000,	MT_DEVICE | MT_RW);
	mmap_add_region(SC_IPC_CH, SC_IPC_CH, 0x10000,
			MT_DEVICE | MT_RW);
	mmap_add_region(PLAT_GICD_BASE, PLAT_GICD_BASE, 0x10000,
			MT_DEVICE | MT_RW);
	mmap_add_region(PLAT_GICR_BASE, PLAT_GICR_BASE, 0xc0000,
			MT_DEVICE | MT_RW);
//	mmap_add_region(IMX_GPT0_BASE, IMX_GPT0_BASE, 0x10000,
//			MT_DEVICE | MT_RW);
	mmap_add_region(IMX_WUP_IRQSTR, IMX_WUP_IRQSTR, 0x10000,
			MT_DEVICE | MT_RW);

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
	/* init the GICv3 cpu and distributor interface */
	plat_gic_driver_init();
	plat_gic_init();
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

unsigned long long plat_get_syscnt_freq(void)
{
	return COUNTER_FREQUENCY;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl31_plat_runtime_setup(void)
{
	return;
}
