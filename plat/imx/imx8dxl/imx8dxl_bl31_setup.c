/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/arm/cci.h>
#include <drivers/console.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <imx8dxl_pads.h>
#include <imx8_iomux.h>
#include <imx8_lpuart.h>
#include <plat_imx8.h>
#include <sci/sci.h>
#include <sec_rsrc.h>
#include <imx_sip_svc.h>
#include <string.h>

#define TRUSTY_PARAMS_LEN_BYTES      (4096*2)
int data_section_restore_flag = 0x1;

IMPORT_SYM(unsigned long, __RW_START__, BL31_RW_START);
IMPORT_SYM(unsigned long, __DATA_START__, BL31_DATA_START);
IMPORT_SYM(unsigned long, __DATA_END__, BL31_DATA_END);

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

#define UART_PAD_CTRL	(PADRING_IFMUX_EN_MASK | PADRING_GP_EN_MASK | \
			(SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			(SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | \
			(SC_PAD_28FDSOI_PS_PD << PADRING_PULL_SHIFT))

static const mmap_region_t imx_mmap[] = {
	MAP_REGION_FLAT(IMX_REG_BASE, IMX_REG_SIZE, MT_DEVICE | MT_RW),
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

#if DEBUG_CONSOLE_A35
static void lpuart32_serial_setbrg(unsigned int base, int baudrate)
{
	unsigned int sbr, osr, baud_diff, tmp_osr, tmp_sbr;
	unsigned int diff1, diff2, tmp, rate;

	if (baudrate == 0)
		panic();

	sc_pm_get_clock_rate(ipc_handle, SC_R_UART_0, 2, &rate);

	baud_diff = baudrate;
	osr = 0;
	sbr = 0;
	for (tmp_osr = 4; tmp_osr <= 32; tmp_osr++) {
		tmp_sbr = (rate / (baudrate * tmp_osr));
		if (tmp_sbr == 0)
			tmp_sbr = 1;

		/* calculate difference in actual baud w/ current values */
		diff1 = rate / (tmp_osr * tmp_sbr) - baudrate;
		diff2 = rate / (tmp_osr * (tmp_sbr + 1));

		/* select best values between sbr and sbr+1 */
		if (diff1 > (baudrate - diff2)) {
			diff1 = baudrate - diff2;
			tmp_sbr++;
		}

		if (diff1 <= baud_diff) {
			baud_diff = diff1;
			osr = tmp_osr;
			sbr = tmp_sbr;
		}
	}

	tmp = mmio_read_32(IMX_BOOT_UART_BASE + BAUD);

	if ((osr > 3) && (osr < 8))
		tmp |= LPUART_BAUD_BOTHEDGE_MASK;

	tmp &= ~LPUART_BAUD_OSR_MASK;
	tmp |= LPUART_BAUD_OSR(osr - 1);
	tmp &= ~LPUART_BAUD_SBR_MASK;
	tmp |= LPUART_BAUD_SBR(sbr);

	/* explicitly disable 10 bit mode & set 1 stop bit */
	tmp &= ~(LPUART_BAUD_M10_MASK | LPUART_BAUD_SBNS_MASK);

	mmio_write_32(IMX_BOOT_UART_BASE + BAUD, tmp);
}

static int lpuart32_serial_init(unsigned int base)
{
	unsigned int tmp;

	/* disable TX & RX before enabling clocks */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(CTRL_TE | CTRL_RE);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + MODIR, 0);
	mmio_write_32(IMX_BOOT_UART_BASE + FIFO, mmio_read_32(IMX_BOOT_UART_BASE + FIFO) | (FIFO_TXFE | FIFO_RXFE));

	mmio_write_32(IMX_BOOT_UART_BASE + MATCH, 0);

	/* provide data bits, parity, stop bit, etc */
	lpuart32_serial_setbrg(base, IMX_BOOT_UART_BAUDRATE);

	/* eight data bits no parity bit */
	tmp = mmio_read_32(IMX_BOOT_UART_BASE + CTRL);
	tmp &= ~(LPUART_CTRL_PE_MASK | LPUART_CTRL_PT_MASK | LPUART_CTRL_M_MASK);
	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, tmp);

	mmio_write_32(IMX_BOOT_UART_BASE + CTRL, CTRL_RE | CTRL_TE);

	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x55);
	mmio_write_32(IMX_BOOT_UART_BASE + DATA, 0x0A);

	return 0;
}
#endif

void imx8_partition_resources(void)
{
	sc_rm_pt_t secure_part, os_part;
	sc_rm_mr_t mr, mr_record = 64, mr_ocram = 64;
	sc_faddr_t start, end, reg_end;
	sc_err_t err;
	bool owned;
	int i;
#if defined(SPD_opteed) || defined(SPD_trusty)
	sc_rm_mr_t mr_tee = 64;
	bool mr_tee_atf_same = false;
	sc_faddr_t reg_start;
#endif
	uint32_t cpu_id, cpu_rev = 0x1; /* Set Rev B as default */

	if (imx_get_cpu_rev(&cpu_id, &cpu_rev) != 0)
		ERROR("Get CPU id and rev failed\n");

	err = sc_rm_get_partition(ipc_handle, &secure_part);
	if (err)
		ERROR("sc_rm_get_partition failed: %u\n", err);

	err = sc_rm_partition_alloc(ipc_handle, &os_part, false, false,
		false, false, false);
	if (err)
		ERROR("sc_rm_partition_alloc failed: %u\n", err);

	err = sc_rm_set_parent(ipc_handle, os_part, secure_part);
	if (err)
		ERROR("sc_rm_set_parent: %u\n", err);

	/* set secure resources to NOT-movable */
	for (i = 0; i < (ARRAY_SIZE(secure_rsrcs)); i++) {
		err = sc_rm_set_resource_movable(ipc_handle,
			 secure_rsrcs[i], secure_rsrcs[i], false);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				secure_rsrcs[i], err);
	}

	/*
	 * sc_rm_set_peripheral_permissions
	 * sc_rm_set_memreg_permissions
	 * sc_rm_set_pin_movable
	 */
	for (mr = 0; mr < 64; mr++) {
		owned = sc_rm_is_memreg_owned(ipc_handle, mr);
		if (owned) {
			err = sc_rm_get_memreg_info(ipc_handle, mr, &start, &end);
			if (err) {
				ERROR("Memreg get info failed, %u\n", mr);
			} else {
				NOTICE("Memreg %u 0x%" PRIx64 " -- 0x%" PRIx64 "\n", mr, start, end);

				if (BL31_BASE >= start && (BL31_LIMIT - 1) <= end) {
					mr_record = mr; /* Record the mr for ATF running */
				}
#if defined(SPD_opteed) || defined(SPD_trusty)
				else if (BL32_BASE >= start && (BL32_LIMIT -1) <= end) {
					mr_tee = mr;
				}
#endif
				else if (cpu_rev >= 1 && 0 >= start && (OCRAM_BASE + OCRAM_ALIAS_SIZE - 1) <= end) {
					mr_ocram = mr;
				}
				else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 ", err %d\n", start, end, err);
				}
			}
		}
	}

#if defined(SPD_opteed) || defined(SPD_trusty)
	if (mr_tee != 64) {
		err = sc_rm_get_memreg_info(ipc_handle, mr_tee, &start, &end);
		if (err) {
			ERROR("Memreg get info failed, %u\n", mr_tee);
		} else {
			if ((BL32_LIMIT - 1) < end) {
				err = sc_rm_memreg_frag(ipc_handle, &mr, BL32_LIMIT , end);
				if (err) {
					ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)BL32_LIMIT, end);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)BL32_LIMIT, end);
				}
			}

			if (start < (BL32_BASE - 1)) {
				err = sc_rm_memreg_frag(ipc_handle, &mr, start, BL32_BASE - 1);
				if (err) {
					ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", start, (sc_faddr_t)BL32_BASE - 1);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", start, (sc_faddr_t)BL32_BASE - 1);
				}
			}
		}
	}
#endif
	if (mr_record != 64) {
		err = sc_rm_get_memreg_info(ipc_handle, mr_record, &start, &end);

#if defined(SPD_opteed) || defined(SPD_trusty)
		if (BL32_BASE >= start && (BL32_LIMIT - 1) <= end)
			mr_tee_atf_same = true;
#endif
		reg_end = end;
		if (err) {
			ERROR("Memreg get info failed, %u\n", mr_record);
		} else {
			if ((BL31_LIMIT - 1) < end) {
#if defined(SPD_opteed) || defined(SPD_trusty)
				if ((end > BL32_BASE) && mr_tee_atf_same)
					reg_end = BL32_BASE - 1;
#endif
				err = sc_rm_memreg_frag(ipc_handle, &mr, BL31_LIMIT, reg_end);
				if (err) {
					ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)BL31_LIMIT, reg_end);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)BL31_LIMIT, reg_end);
				}
			}
#if defined(SPD_opteed) || defined(SPD_trusty)
			if (mr_tee_atf_same) {
				if ((BL32_LIMIT - 1) < end) {
					reg_start = BL32_LIMIT;
					err = sc_rm_memreg_frag(ipc_handle, &mr, reg_start, end);
					if (err) {
						ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", reg_start, reg_end);
					} else {
						err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
						if (err)
							ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", reg_start, reg_end);
					}
				}
			}
#endif

			if (start < (BL31_BASE - 1)) {
				err = sc_rm_memreg_frag(ipc_handle, &mr, start, BL31_BASE - 1);
				if (err)
					ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n",
						start, (sc_faddr_t)BL31_BASE - 1);
				err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
				if (err)
					ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n",
						start, (sc_faddr_t)BL31_BASE - 1);
			}
		}
	}

	if (mr_ocram != 64) {
		err = sc_rm_get_memreg_info(ipc_handle, mr_ocram, &start, &end);
		reg_end = end;
		if (err) {
			ERROR("Memreg get info failed, %u\n", mr_ocram);
		} else {
			if ((OCRAM_BASE + OCRAM_ALIAS_SIZE - 1) < end) {
				err = sc_rm_memreg_frag(ipc_handle, &mr, OCRAM_BASE + OCRAM_ALIAS_SIZE, reg_end);
				if (err) {
					ERROR("sc_rm_memreg_frag failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)OCRAM_BASE + OCRAM_ALIAS_SIZE, reg_end);
				} else {
					err = sc_rm_assign_memreg(ipc_handle, os_part, mr);
					if (err)
						ERROR("Memreg assign failed, 0x%" PRIx64 " -- 0x%" PRIx64 "\n", (sc_faddr_t)OCRAM_BASE + OCRAM_ALIAS_SIZE, reg_end);
				}
			}
		}
	}

	owned = sc_rm_is_resource_owned(ipc_handle, SC_R_M4_0_PID0);
	if (owned) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, false);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
	}

	/* move all movable resources and pins to non-secure partition */
	err = sc_rm_move_all(ipc_handle, secure_part, os_part, true, true);
	if (err)
		ERROR("sc_rm_move_all: %u\n", err);
	if (owned) {
		err = sc_rm_set_resource_movable(ipc_handle, SC_R_M4_0_PID0,
				SC_R_M4_0_PID0, true);
		if (err)
			ERROR("sc_rm_set_resource_movable: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
		err = sc_rm_assign_resource(ipc_handle, os_part, SC_R_M4_0_PID0);
		if (err)
			ERROR("sc_rm_assign_resource: rsrc %u, ret %u\n",
				SC_R_M4_0_PID0, err);
	}

	/* iterate through peripherals to give NS OS part access */
	for (i = 0; i < ARRAY_SIZE(ns_access_allowed); i++) {
		err = sc_rm_set_peripheral_permissions(ipc_handle,
			ns_access_allowed[i], os_part, SC_RM_PERM_FULL);
		if (err)
			ERROR("sc_rm_set_peripheral_permissions: rsrc %u, \
				ret %u\n", ns_access_allowed[i], err);
	}

	if (err)
		NOTICE("Partitioning Failed\n");
	else
		NOTICE("Non-secure Partitioning Succeeded\n");
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	unsigned int count = (BL31_DATA_END - BL31_DATA_START);
	unsigned char *data = (unsigned char *)(BL31_LIMIT - (BL31_DATA_END - BL31_DATA_START));
	unsigned char *ptr = (unsigned char *)BL31_DATA_START;

	if (data_section_restore_flag == 0x1) {
		data_section_restore_flag = 0x2;
		memcpy(data, ptr, count);
	} else {
		memcpy(ptr, data, count);
	}

#if DEBUG_CONSOLE
	static console_t console;
#endif
	if (sc_ipc_open(&ipc_handle, SC_IPC_BASE) != SC_ERR_NONE)
		panic();

#if DEBUG_CONSOLE_A35
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_UART_0, SC_PM_PW_MODE_ON);
	sc_pm_clock_rate_t rate = 80000000;
	sc_pm_set_clock_rate(ipc_handle, SC_R_UART_0, 2, &rate);
	sc_pm_clock_enable(ipc_handle, SC_R_UART_0, 2, true, false);

	/* Configure UART pads */
	sc_pad_set(ipc_handle, SC_P_UART0_RX, UART_PAD_CTRL);
	sc_pad_set(ipc_handle, SC_P_UART0_TX, UART_PAD_CTRL);
	lpuart32_serial_init(IMX_BOOT_UART_BASE);
#endif

#if DEBUG_CONSOLE
	console_lpuart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		     IMX_CONSOLE_BAUDRATE, &console);
#endif
	/* Turn on MU1 for non-secure OS/Hypervisor */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_MU_1A, SC_PM_PW_MODE_ON);

	/* Turn on GPT_0's power & clock for non-secure OS/Hypervisor */
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_GPT_0, SC_PM_PW_MODE_ON);
	sc_pm_clock_enable(ipc_handle, SC_R_GPT_0, SC_PM_CLK_PER, true, 0);
	mmio_write_32(IMX_GPT0_LPCG_BASE, mmio_read_32(IMX_GPT0_LPCG_BASE) | (1 << 25));

	/*
	 * create new partition for non-secure OS/Hypervisor
	 * uses global structs defined in sec_rsrc.h
	 */
	imx8_partition_resources();

#ifdef SPD_trusty
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_CAAM_JR2, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_CAAM_JR2_OUT, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_CAAM_JR3, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_CAAM_JR3_OUT, SC_PM_PW_MODE_ON);
	sc_pm_set_resource_power_mode(ipc_handle, SC_R_MU_4A, SC_PM_PW_MODE_ON);
#endif

	bl33_image_ep_info.pc = PLAT_NS_IMAGE_OFFSET;
	bl33_image_ep_info.spsr = get_spsr_for_bl33_entry();
#if defined(SPD_opteed) || defined(SPD_trusty)
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = 0;
#ifdef SPD_trusty
	bl32_image_ep_info.args.arg0 = BL32_SIZE;
	bl32_image_ep_info.args.arg1 = BL32_BASE;
#endif
#ifdef SPD_opteed
	bl33_image_ep_info.args.arg1 = BL32_BASE;
	bl33_image_ep_info.args.arg2 = BL32_SIZE;
	/* Make sure memory is clean */
	mmio_write_32(BL32_FDT_OVERLAY_ADDR, 0);
	bl33_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
	bl32_image_ep_info.args.arg3 = BL32_FDT_OVERLAY_ADDR;
#endif
#endif
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

void bl31_plat_arch_setup(void)
{
	unsigned long ro_start = BL_CODE_BASE;
	unsigned long ro_size = BL_CODE_END - BL_CODE_BASE;
	unsigned long rw_start = BL31_RW_START;
	unsigned long rw_size = BL_END - BL31_RW_START;
#if USE_COHERENT_MEM
	unsigned long coh_start = BL_COHERENT_RAM_BASE;
	unsigned long coh_size = BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE;
#endif

	mmap_add_region(ro_start, ro_start, ro_size,
		MT_RO | MT_MEMORY | MT_SECURE);
	mmap_add_region(rw_start, rw_start, rw_size,
		MT_RW | MT_MEMORY | MT_SECURE);
	mmap_add(imx_mmap);

#if defined(SPD_opteed) || defined(SPD_trusty)
	mmap_add_region(BL32_BASE, BL32_BASE, BL32_SIZE, MT_MEMORY | MT_RW);
#endif

#if USE_COHERENT_MEM
	mmap_add_region(coh_start, coh_start, coh_size,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	init_xlat_tables();
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
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
