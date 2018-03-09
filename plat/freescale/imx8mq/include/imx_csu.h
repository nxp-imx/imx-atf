/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_CSU_H__
#define __IMX_CSU_H__

enum csu_mode {
	CSU_NSR = 0x08,
	CSU_NSW = 0x80,
	CSU_NSRW = 0x88,
	CSU_NUR = 0x04,
	CSU_NUW = 0x40,
	CSU_NURW = 0x44,
	CSU_SSR = 0x02,
	CSU_SSW = 0x20,
	CSU_SSRW = 0x22,
	CSU_SUR = 0x01,
	CSU_SUW = 0x10,
	CSU_SURW = 0x11,
	CSU_RW = 0xff,
};

enum csu_csln_idx {
	CSU_CSLn_GPIO1 = 0,
	CSU_CSLn_GPIO2 = 1,
	CSU_CSLn_GPIO3 = 2,
	CSU_CSLn_GPIO4 = 3,
	CSU_CSLn_GPIO5 = 4,
	CSU_CSLn_Reserved1 = 5,
	CSU_CSLn_ANA_TSENSOR = 6,
	CSU_CSLn_ANA_OSC = 7,
	CSU_CSLn_WDOG1 = 8,
	CSU_CSLn_WDOG2 = 9,
	CSU_CSLn_WDOG3 = 10,
	CSU_CSLn_Reserved2 = 11,
	CSU_CSLn_SDMA2 = 12,
	CSU_CSLn_GPT1 = 13,
	CSU_CSLn_GPT2 = 14,
	CSU_CSLn_GPT3 = 15,
	CSU_CSLn_Reserved3 = 16,
	CSU_CSLn_ROMCP = 17,
	CSU_CSLn_LCDIF = 18,
	CSU_CSLn_IOMUXC = 19,
	CSU_CSLn_IOMUXC_GPR = 20,
	CSU_CSLn_OCOTP_CTRL = 21,
	CSU_CSLn_ANATOP_PLL = 22,
	CSU_CSLn_SNVS_HP = 23,
	CSU_CSLn_CCM = 24,
	CSU_CSLn_SRC = 25,
	CSU_CSLn_GPC = 26,
	CSU_CSLn_SEMAPHORE1 = 27,
	CSU_CSLn_SEMAPHORE2 = 28,
	CSU_CSLn_RDC = 29,
	CSU_CSLn_CSU = 30,
	CSU_CSLn_Reserved4 = 31,
	CSU_CSLn_MST0 = 32,
	CSU_CSLn_MST1 = 33,
	CSU_CSLn_MST2 = 34,
	CSU_CSLn_MST3 = 35,
	CSU_CSLn_HDMI_SEC = 36,
	CSU_CSLn_Reserved5 = 37,
	CSU_CSLn_PWM1 = 38,
	CSU_CSLn_PWM2 = 39,
	CSU_CSLn_PWM3 = 40,
	CSU_CSLn_PWM4 = 41,
	CSU_CSLn_SysCounter_RD = 42,
	CSU_CSLn_SysCounter_CMP = 43,
	CSU_CSLn_SysCounter_CTRL = 44,
	CSU_CSLn_HDMI_CTRL = 45,
	CSU_CSLn_GPT6 = 46,
	CSU_CSLn_GPT5 = 47,
	CSU_CSLn_GPT4 = 48,
	CSU_CSLn_TZASC = 56,
	CSU_CSLn_MTR = 59,
	CSU_CSLn_PERFMON1 = 60,
	CSU_CSLn_PERFMON2 = 61,
	CSU_CSLn_PLATFORM_CTRL = 62,
	CSU_CSLn_QoSC = 63,
	CSU_CSLn_MIPI_PHY = 64,
	CSU_CSLn_MIPI_DSI = 65,
	CSU_CSLn_I2C1 = 66,
	CSU_CSLn_I2C2 = 67,
	CSU_CSLn_I2C3 = 68,
	CSU_CSLn_I2C4 = 69,
	CSU_CSLn_UART4 = 70,
	CSU_CSLn_MIPI_CSI1 = 71,
	CSU_CSLn_MIPI_CSI_PHY1 = 72,
	CSU_CSLn_CSI1 = 73,
	CSU_CSLn_MU_A = 74,
	CSU_CSLn_MU_B = 75,
	CSU_CSLn_SEMAPHORE_HS = 76,
	CSU_CSLn_Internal1 = 77,
	CSU_CSLn_SAI1 = 78,
	CSU_CSLn_Reserved7 = 79,
	CSU_CSLn_SAI6 = 80,
	CSU_CSLn_SAI5 = 81,
	CSU_CSLn_SAI4 = 82,
	CSU_CSLn_Internal2 = 83,
	CSU_CSLn_USDHC1 = 84,
	CSU_CSLn_USDHC2 = 85,
	CSU_CSLn_MIPI_CSI2 = 86,
	CSU_CSLn_MIPI_CSI_PHY2 = 87,
	CSU_CSLn_CSI2 = 88,
	CSU_CSLn_Internal3 = 89,
	CSU_CSLn_Reserved10 = 90,
	CSU_CSLn_QSPI = 91,
	CSU_CSLn_Reserved11 = 92,
	CSU_CSLn_SDMA1 = 93,
	CSU_CSLn_ENET1 = 94,
	CSU_CSLn_Reserved12 = 95,
	CSU_CSLn_Internal4 = 96,
	CSU_CSLn_SPDIF1 = 97,
	CSU_CSLn_ECSPI1 = 98,
	CSU_CSLn_ECSPI2 = 99,
	CSU_CSLn_ECSPI3 = 100,
	CSU_CSLn_Reserved14 = 101,
	CSU_CSLn_UART1 = 102,
	CSU_CSLn_Internal5 = 103,
	CSU_CSLn_UART3 = 104,
	CSU_CSLn_UART2 = 105,
	CSU_CSLn_SPDIF2 = 106,
	CSU_CSLn_SAI2 = 107,
	CSU_CSLn_SAI3 = 108,
	CSU_CSLn_Reserved16 = 109,
	CSU_CSLn_Internal6 = 110,
	CSU_CSLn_SPBA1 = 111,
	CSU_CSLn_MOD_EN3 = 112,
	CSU_CSLn_MOD_EN0 = 113,
	CSU_CSLn_CAAM = 114,
	CSU_CSLn_DDRC_SEC = 115,
	CSU_CSLn_GIC_EXSC = 116,
	CSU_CSLn_USB_EXSC = 117,
	CSU_CSLn_OCRAM_TZ = 118,
	CSU_CSLn_OCRAM_S_TZ = 119,
	CSU_CSLn_VPU_SEC = 120,
	CSU_CSLn_DAP_EXSC = 121,
	CSU_CSLn_ROMCP_SEC = 122,
	CSU_CSLn_APBHDMA_SEC = 123,
	CSU_CSLn_M4_SEC = 124,
	CSU_CSLn_QSPI_SEC = 125,
	CSU_CSLn_GPU_EXSC = 126,
	CSU_CSLn_PCIE = 127,
};

enum csu_hp_idx {
	CSU_HP_A53,
	CSU_HP_M4,
	CSU_HP_SDMA1,
	CSU_HP_CSI,
	CSU_HP_USB,
	CSU_HP_PCIE,
	CSU_HP_VPU,
	CSU_HP_GPU,
	CSU_HP_APBHDMA,
	CSU_HP_ENET,
	CSU_HP_USDHC1,
	CSU_HP_USDHC2,
	CSU_HP_DCSS,
	CSU_HP_HUGO,
	CSU_HP_DAP,
	CSU_HP_SDMA2,
	CSU_HP_CAAM,
};

enum csu_sa_idx {
	CSU_SA_M4,
	CSU_SA_SDMA1,
	CSU_SA_CSI,
	CSU_SA_USB,
	CSU_SA_PCIE,
	CSU_SA_VPU,
	CSU_SA_GPU,
	CSU_SA_APBHDMA,
	CSU_SA_ENET,
	CSU_SA_USDHC1,
	CSU_SA_USDHC2,
	CSU_SA_DCSS,
	CSU_SA_HUGO,
	CSU_SA_DAP,
	CSU_SA_SDMA2,
	CSU_SA_CAAM,
};

struct csu_slave_conf {
	enum csu_csln_idx index;
	uint16_t mode;
	uint16_t lock;
};

struct csu_sa_conf {
	enum csu_sa_idx index;
	uint8_t enable;
	uint8_t lock;
};

void csu_set_slave_index_mode(enum csu_csln_idx index,
			uint16_t mode, uint8_t lock);
void csu_get_slave_index_mode(enum csu_csln_idx index,
			uint16_t *mode, uint8_t *lock);
void csu_set_slaves_modes(struct csu_slave_conf *csu_config, uint32_t count);
void csu_set_default_slaves_modes(void);
void csu_set_hp_index_config(enum csu_hp_idx index, uint8_t enable,
			uint8_t set_control, uint8_t lock);
void csu_set_sa_index_config(enum csu_sa_idx index, uint8_t enable,
			uint8_t lock);
void csu_get_sa_index_config(enum csu_sa_idx index, uint8_t *enable,
			uint8_t *lock);
void csu_set_sa_configs(struct csu_sa_conf *sa_configs,  uint32_t count);
void csu_set_default_secure_configs(void);

#if defined (CSU_RDC_TEST)
void csu_test(void);
#endif
#endif /* __IMX_CSU_H__ */
