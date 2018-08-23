/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <stdlib.h>
#include <stdint.h>
#include <platform_def.h>
#include <utils_def.h>
#include <mmio.h>
#include <imx_csu.h>

#define CSU_HP0_OFFSET		(0x200)
#define CSU_HP1_OFFSET		(0x204)
#define CSU_SA_OFFSET		(0x218)
#define CSU_HPC0_OFFSET		(0x358)
#define CSU_HPC1_OFFSET		(0x35C)


/* Default CSU slaves CSLn settings */
static struct csu_slave_conf csu_def_csl_conf[] = {
	{CSU_CSLn_GPIO1, CSU_RW, 0},
	{CSU_CSLn_GPIO2, CSU_RW, 0},
	{CSU_CSLn_GPIO3, CSU_RW, 0},
	{CSU_CSLn_GPIO4, CSU_RW, 0},
	{CSU_CSLn_GPIO5, CSU_RW, 0},
	{CSU_CSLn_Reserved1, CSU_RW, 0},
	{CSU_CSLn_ANA_TSENSOR, CSU_RW, 0},
	{CSU_CSLn_ANA_OSC, CSU_RW, 0},
	{CSU_CSLn_WDOG1, CSU_RW, 0},
	{CSU_CSLn_WDOG2, CSU_RW, 0},
	{CSU_CSLn_WDOG3, CSU_RW, 0},
	{CSU_CSLn_SDMA2, CSU_RW, 0},
	{CSU_CSLn_GPT1, CSU_RW, 0},
	{CSU_CSLn_GPT2, CSU_RW, 0},
	{CSU_CSLn_GPT3, CSU_RW, 0},
	{CSU_CSLn_ROMCP, CSU_RW, 0},
	{CSU_CSLn_LCDIF, CSU_RW, 0},
	{CSU_CSLn_IOMUXC, CSU_RW, 0},
	{CSU_CSLn_IOMUXC_GPR, CSU_RW, 0},
	{CSU_CSLn_OCOTP_CTRL, CSU_RW, 0},
	{CSU_CSLn_ANATOP_PLL, CSU_RW, 0},
	{CSU_CSLn_SNVS_HP, CSU_RW, 0},
	{CSU_CSLn_CCM, CSU_RW, 0},
	{CSU_CSLn_SRC, CSU_RW, 0},
	{CSU_CSLn_GPC, CSU_RW, 0},
	{CSU_CSLn_SEMAPHORE1, CSU_RW, 0},
	{CSU_CSLn_SEMAPHORE2, CSU_RW, 0},
#if defined(CSU_RDC_TEST)
	{CSU_CSLn_RDC, CSU_SSRW, 0},
#else
	{CSU_CSLn_RDC, CSU_RW, 0},
#endif
	{CSU_CSLn_CSU, CSU_RW, 0},
	{CSU_CSLn_MST0, CSU_RW, 0},
	{CSU_CSLn_MST1, CSU_RW, 0},
	{CSU_CSLn_MST2, CSU_RW, 0},
	{CSU_CSLn_MST3, CSU_RW, 0},
	{CSU_CSLn_HDMI_SEC, CSU_RW, 0},
	{CSU_CSLn_PWM1, CSU_RW, 0},
	{CSU_CSLn_PWM2, CSU_RW, 0},
	{CSU_CSLn_PWM3, CSU_RW, 0},
	{CSU_CSLn_PWM4, CSU_RW, 0},
	{CSU_CSLn_SysCounter_RD, CSU_RW, 0},
	{CSU_CSLn_SysCounter_CMP, CSU_RW, 0},
	{CSU_CSLn_SysCounter_CTRL, CSU_RW, 0},
	{CSU_CSLn_HDMI_CTRL, CSU_RW, 0},
	{CSU_CSLn_GPT6, CSU_RW, 0},
	{CSU_CSLn_GPT5, CSU_RW, 0},
	{CSU_CSLn_GPT4, CSU_RW, 0},
	{CSU_CSLn_TZASC, CSU_RW, 0},
	{CSU_CSLn_MTR, CSU_RW, 0},
	{CSU_CSLn_PERFMON1, CSU_RW, 0},
	{CSU_CSLn_PERFMON2, CSU_RW, 0},
	{CSU_CSLn_PLATFORM_CTRL, CSU_RW, 0},
	{CSU_CSLn_QoSC, CSU_RW, 0},
	{CSU_CSLn_MIPI_PHY, CSU_RW, 0},
	{CSU_CSLn_MIPI_DSI, CSU_RW, 0},
	{CSU_CSLn_I2C1, CSU_RW, 0},
	{CSU_CSLn_I2C2, CSU_RW, 0},
	{CSU_CSLn_I2C3, CSU_RW, 0},
	{CSU_CSLn_I2C4, CSU_RW, 0},
	{CSU_CSLn_UART4, CSU_RW, 0},
	{CSU_CSLn_MIPI_CSI1, CSU_RW, 0},
	{CSU_CSLn_MIPI_CSI_PHY1, CSU_RW, 0},
	{CSU_CSLn_CSI1, CSU_RW, 0},
	{CSU_CSLn_MU_A, CSU_RW, 0},
	{CSU_CSLn_MU_B, CSU_RW, 0},
	{CSU_CSLn_SEMAPHORE_HS, CSU_RW, 0},
	{CSU_CSLn_SAI1, CSU_RW, 0},
	{CSU_CSLn_SAI6, CSU_RW, 0},
	{CSU_CSLn_SAI5, CSU_RW, 0},
	{CSU_CSLn_SAI4, CSU_RW, 0},
	{CSU_CSLn_USDHC1, CSU_RW, 0},
	{CSU_CSLn_USDHC2, CSU_RW, 0},
	{CSU_CSLn_MIPI_CSI2, CSU_RW, 0},
	{CSU_CSLn_MIPI_CSI_PHY2, CSU_RW, 0},
	{CSU_CSLn_CSI2, CSU_RW, 0},
	{CSU_CSLn_QSPI, CSU_RW, 0},
	{CSU_CSLn_SDMA1, CSU_RW, 0},
	{CSU_CSLn_ENET1, CSU_RW, 0},
	{CSU_CSLn_SPDIF1, CSU_RW, 0},
	{CSU_CSLn_ECSPI1, CSU_RW, 0},
	{CSU_CSLn_ECSPI2, CSU_RW, 0},
	{CSU_CSLn_ECSPI3, CSU_RW, 0},
	{CSU_CSLn_UART1, CSU_RW, 0},
	{CSU_CSLn_UART3, CSU_RW, 0},
	{CSU_CSLn_UART2, CSU_RW, 0},
	{CSU_CSLn_SPDIF2, CSU_RW, 0},
	{CSU_CSLn_SAI2, CSU_RW, 0},
	{CSU_CSLn_SAI3, CSU_RW, 0},
	{CSU_CSLn_SPBA1, CSU_RW, 0},
	{CSU_CSLn_MOD_EN3, CSU_RW, 0},
	{CSU_CSLn_MOD_EN0, CSU_RW, 0},
	{CSU_CSLn_CAAM, CSU_RW, 0},
	{CSU_CSLn_DDRC_SEC, CSU_RW, 0},
	{CSU_CSLn_GIC_EXSC, CSU_RW, 0},
	{CSU_CSLn_USB_EXSC, CSU_RW, 0},
	{CSU_CSLn_OCRAM_TZ, CSU_RW, 0},
	{CSU_CSLn_OCRAM_S_TZ, CSU_RW, 0},
	{CSU_CSLn_VPU_SEC, CSU_RW, 0},
	{CSU_CSLn_DAP_EXSC, CSU_RW, 0},
	{CSU_CSLn_ROMCP_SEC, CSU_RW, 0},
	{CSU_CSLn_APBHDMA_SEC, CSU_RW, 0},
	{CSU_CSLn_M4_SEC, CSU_RW, 0},
	{CSU_CSLn_QSPI_SEC, CSU_RW, 0},
	{CSU_CSLn_GPU_EXSC, CSU_RW, 0},
	{CSU_CSLn_Internal1, CSU_RW, 0},
	{CSU_CSLn_Internal2, CSU_RW, 0},
	{CSU_CSLn_Internal3, CSU_RW, 0},
	{CSU_CSLn_Internal4, CSU_RW, 0},
	{CSU_CSLn_Internal5, CSU_RW, 0},
	{CSU_CSLn_Internal6, CSU_RW, 0},
};

/* Default Secure Access configuration */
static struct csu_sa_conf sa_def_configs[] = {
	{CSU_SA_VPU, 1, 1},
	{CSU_SA_GPU, 1, 1},
	{CSU_SA_DCSS, 1, 1},
};

void csu_set_slave_index_mode(enum csu_csln_idx index,
			uint16_t mode, uint8_t lock)
{
	uintptr_t reg;
	uint32_t tmp;
	uint16_t read_mode;
	uint8_t read_lock = 0;

	/* Check if CSLn is locked or the value is same as written */
	csu_get_slave_index_mode(index, &read_mode, &read_lock);
	if (read_lock) {
		NOTICE("CSU CSLn(%d) already locked with mode:0x%x\n", index, read_mode);
		return;
	}
	if (read_mode == mode) {
		NOTICE("CSU CSLn(%d) mode 0x%x already written\n", index, read_mode);
		return;
	}
	reg = (uintptr_t)(IMX_CSU_BASE + (index / 2) * 4);
	tmp = mmio_read_32(reg);

	if (lock)
		mode |= 1 << 8;

	if (index % 2) {
		tmp &= 0x0000ffff;
		tmp |= mode << 16;
	} else {
		tmp &= 0xffff0000;
		tmp |= mode;
	}
	mmio_write_32(reg, tmp);
}

void csu_get_slave_index_mode(enum csu_csln_idx index,
			uint16_t *mode, uint8_t *lock)
{
	uintptr_t reg;
	uint32_t tmp;

	reg = (uintptr_t)(IMX_CSU_BASE + (index / 2) * 4);
	tmp = mmio_read_32(reg);
	if (index % 2)
		tmp = tmp >> 16;

	tmp &= 0x1ff;

	*mode = tmp & 0xff;
	*lock = tmp >> 8;	
}

void csu_set_slaves_modes(struct csu_slave_conf *csu_config, uint32_t count)
{
	int i;

	for (i = 0; i < count; i++) {
		csu_set_slave_index_mode(csu_config[i].index, csu_config[i].mode, csu_config[i].lock);
	}
}

void csu_set_default_slaves_modes(void)
{
	NOTICE("csu_set_default_slaves_modes: count = %d \n", (int)ARRAY_SIZE(csu_def_csl_conf));
	csu_set_slaves_modes(csu_def_csl_conf, (uint32_t)ARRAY_SIZE(csu_def_csl_conf));
}

void csu_set_hp_index_config(enum csu_hp_idx index, uint8_t enable,
			uint8_t set_control, uint8_t lock)
{
	uint32_t tmp, value;
	uintptr_t reg;

	if (index < 16){
		reg = (uintptr_t)(IMX_CSU_BASE + CSU_HP0_OFFSET);
		tmp = mmio_read_32(reg);
		value = 0x3 << (index * 2);
		tmp &= ~value;
		value = (lock * 2 | enable) << (index * 2);
		tmp |= value;
		mmio_write_32(reg, tmp);
		if (set_control) {
			reg = (uintptr_t)(IMX_CSU_BASE + CSU_HPC0_OFFSET);
			tmp = mmio_read_32(reg);
			value = (lock * 2 | set_control) << (index * 2);
			tmp &= ~value;
			tmp |= value;
			mmio_write_32(reg, tmp);
		}
	} else {
		reg = (uintptr_t)(IMX_CSU_BASE + CSU_HP1_OFFSET);
		mmio_write_32(reg,lock * 2 | enable);
		if (set_control) {
			reg = (uintptr_t)(IMX_CSU_BASE + CSU_HPC1_OFFSET);
			mmio_write_32(reg,lock * 2 | set_control);
		}
	}
}

void csu_set_sa_index_config(enum csu_sa_idx index,
			uint8_t enable, uint8_t lock)
{
	uint32_t tmp, value;
	uintptr_t reg;

	reg = (uintptr_t)(IMX_CSU_BASE + CSU_SA_OFFSET);
	tmp = mmio_read_32((uintptr_t)reg);
	value = 0x3 << (index * 2);
	tmp &= ~value;
	value = (lock * 2 | enable) << (index * 2);
	tmp |= value;
	mmio_write_32(reg, tmp);
}

void csu_get_sa_index_config(enum csu_sa_idx index,
			uint8_t *enable, uint8_t *lock)
{
	uint32_t tmp;
	uintptr_t reg;

	reg = (uintptr_t)(IMX_CSU_BASE + CSU_SA_OFFSET);
	tmp = mmio_read_32((uintptr_t)reg);
	*enable = (tmp >> (index * 2)) & 1;
	*lock = (tmp >> (index * 2 + 1)) & 1;
}

void csu_set_sa_configs(struct csu_sa_conf *sa_conf,  uint32_t count)
{
	int i;

	for (i = 0; i < count; i++)
		csu_set_sa_index_config(sa_conf[i].index,
			sa_conf[i].enable, sa_conf[i].lock);
}

void csu_set_default_secure_configs(void)
{
	csu_set_sa_configs(sa_def_configs, (uint32_t)ARRAY_SIZE(sa_def_configs));
}

#if defined (CSU_RDC_TEST)
void csu_test(void)
{
	csu_set_default_slaves_modes();
}
#endif
