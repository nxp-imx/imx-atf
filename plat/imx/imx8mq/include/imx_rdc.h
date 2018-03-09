/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_RDC_H__
#define __IMX_RDC_H__

/* Masters index */
enum rdc_mda_idx {
	RDC_MDA_A53 = 0,
	RDC_MDA_M4 = 1,
	RDC_MDA_PCIE_CTRL1 = 2,
	RDC_MDA_PCIE_CTRL2 = 3,
	RDC_MDA_VPU_DEC = 4,
	RDC_MDA_LCDIF = 5,
	RDC_MDA_CSI1 = 6,
	RDC_MDA_CSI2 = 7,
	RDC_MDA_Coresight = 8,
	RDC_MDA_DAP = 9,
	RDC_MDA_CAAM = 10,
	RDC_MDA_SDMAp = 11,
	RDC_MDA_SDMAb = 12,
	RDC_MDA_APBHDMA = 13,
	RDC_MDA_RAWNAND = 14,
	RDC_MDA_uSDHC1 = 15,
	RDC_MDA_uSDHC2 = 16,
	RDC_MDA_DCSS = 17,
	RDC_MDA_GPU = 18,
	RDC_MDA_USB1 = 19,
	RDC_MDA_USB2 = 20,
	RDC_MDA_TESTPORT = 21,
	RDC_MDA_ENET1_TX = 22,
	RDC_MDA_ENET1_RX = 23,
	RDC_MDA_SDMA2 = 24,
	RDC_MDA_Reserved = 25,
	RDC_MDA_SDMA1 = 26,
};

/* Peripherals index */
enum rdc_pdap_idx {
	RDC_PDAP_GPIO1 = 0,
	RDC_PDAP_GPIO2 = 1,
	RDC_PDAP_GPIO3 = 2,
	RDC_PDAP_GPIO4 = 3,
	RDC_PDAP_GPIO5 = 4,
	RDC_PDAP_Reserved1 = 5,
	RDC_PDAP_ANA_TSENSOR = 6,
	RDC_PDAP_ANA_OSC = 7,
	RDC_PDAP_WDOG1 = 8,
	RDC_PDAP_WDOG2 = 9,
	RDC_PDAP_WDOG3 = 10,
	RDC_PDAP_Reserved2 = 11,
	RDC_PDAP_SDMA2 = 12,
	RDC_PDAP_GPT1 = 13,
	RDC_PDAP_GPT2 = 14,
	RDC_PDAP_GPT3 = 15,
	RDC_PDAP_Reserved3 = 16,
	RDC_PDAP_ROMCP = 17,
	RDC_PDAP_LCDIF = 18,
	RDC_PDAP_IOMUXC = 19,
	RDC_PDAP_IOMUXC_GPR = 20,
	RDC_PDAP_OCOTP_CTRL = 21,
	RDC_PDAP_ANATOP_PLL = 22,
	RDC_PDAP_SNVS_HP = 23,
	RDC_PDAP_CCM = 24,
	RDC_PDAP_SRC = 25,
	RDC_PDAP_GPC = 26,
	RDC_PDAP_SEMAPHORE1 = 27,
	RDC_PDAP_SEMAPHORE2 = 28,
	RDC_PDAP_RDC = 29,
	RDC_PDAP_CSU = 30,
	RDC_PDAP_Reserved4 = 31,
	RDC_PDAP_MST0 = 32,
	RDC_PDAP_MST1 = 33,
	RDC_PDAP_MST2 = 34,
	RDC_PDAP_MST3 = 35,
	RDC_PDAP_HDMI_SEC = 36,
	RDC_PDAP_Reserved5 = 37,
	RDC_PDAP_PWM1 = 38,
	RDC_PDAP_PWM2 = 39,
	RDC_PDAP_PWM3 = 40,
	RDC_PDAP_PWM4 = 41,
	RDC_PDAP_SysCounter_RD = 42,
	RDC_PDAP_SysCounter_CMP = 43,
	RDC_PDAP_SysCounter_CTRL = 44,
	RDC_PDAP_HDMI_CTRL = 45,
	RDC_PDAP_GPT6 = 46,
	RDC_PDAP_GPT5 = 47,
	RDC_PDAP_GPT4 = 48,
	RDC_PDAP_TZASC = 56,
	RDC_PDAP_MTR = 59,
	RDC_PDAP_PERFMON1 = 60,
	RDC_PDAP_PERFMON2 = 61,
	RDC_PDAP_PLATFORM_CTRL = 62,
	RDC_PDAP_QoSC = 63,
	RDC_PDAP_MIPI_PHY = 64,
	RDC_PDAP_MIPI_DSI = 65,
	RDC_PDAP_I2C1 = 66,
	RDC_PDAP_I2C2 = 67,
	RDC_PDAP_I2C3 = 68,
	RDC_PDAP_I2C4 = 69,
	RDC_PDAP_UART4 = 70,
	RDC_PDAP_MIPI_CSI1 = 71,
	RDC_PDAP_MIPI_CSI_PHY1 = 72,
	RDC_PDAP_CSI1 = 73,
	RDC_PDAP_MU_A = 74,
	RDC_PDAP_MU_B = 75,
	RDC_PDAP_SEMAPHORE_HS = 76,
	RDC_PDAP_Reserved6 = 77,
	RDC_PDAP_SAI1 = 78,
	RDC_PDAP_Reserved7 = 79,
	RDC_PDAP_SAI6 = 80,
	RDC_PDAP_SAI5 = 81,
	RDC_PDAP_SAI4 = 82,
	RDC_PDAP_Reserved8 = 83,
	RDC_PDAP_USDHC1 = 84,
	RDC_PDAP_USDHC2 = 85,
	RDC_PDAP_MIPI_CSI2 = 86,
	RDC_PDAP_MIPI_CSI_PHY2 = 87,
	RDC_PDAP_CSI2 = 88,
	RDC_PDAP_Reserved9 = 89,
	RDC_PDAP_Reserved10 = 90,
	RDC_PDAP_QSPI = 91,
	RDC_PDAP_Reserved11 = 92,
	RDC_PDAP_SDMA1 = 93,
	RDC_PDAP_ENET1 = 94,
	RDC_PDAP_Reserved12 = 95,
	RDC_PDAP_Reserved13 = 96,
	RDC_PDAP_SPDIF1 = 97,
	RDC_PDAP_ECSPI1 = 98,
	RDC_PDAP_ECSPI2 = 99,
	RDC_PDAP_ECSPI3 = 100,
	RDC_PDAP_Reserved14 = 101,
	RDC_PDAP_UART1 = 102,
	RDC_PDAP_Reserved15 = 103,
	RDC_PDAP_UART3 = 104,
	RDC_PDAP_UART2 = 105,
	RDC_PDAP_SPDIF2 = 106,
	RDC_PDAP_SAI2 = 107,
	RDC_PDAP_SAI3 = 108,
	RDC_PDAP_Reserved16 = 109,
	RDC_PDAP_Reserved17 = 110,
	RDC_PDAP_SPBA1 = 111,
	RDC_PDAP_CAAM = 114,
	RDC_PDAP_DDRC_SEC = 115,
	RDC_PDAP_GIC_EXSC = 116,
	RDC_PDAP_USB_EXSC = 117,
	RDC_PDAP_OCRAM_TZ = 118,
	RDC_PDAP_OCRAM_S_TZ = 119,
	RDC_PDAP_VPU_SEC = 120,
	RDC_PDAP_DAP_EXSC = 121,
	RDC_PDAP_ROMCP_SEC = 122,
	RDC_PDAP_APBHDMA_SEC = 123,
	RDC_PDAP_M4_SEC = 124,
	RDC_PDAP_QSPI_SEC = 125,
	RDC_PDAP_GPU_EXSC = 126,
	RDC_PDAP_PCIE = 127,
};

/* RDC registers mapping */
struct imx_rdc_regs {
	uint32_t	vir;		/* Version information */
	uint32_t	reserved1[8];
	uint32_t	stat;		/* Status */
	uint32_t	intctrl;	/* Interrupt and Control */
	uint32_t	intstat;	/* Interrupt Status */
	uint32_t	reserved2[116];
	uint32_t	mda[27];		/* Master Domain Assignment */
	uint32_t	reserved3[101];
	uint32_t	pdap[118];		/* Peripheral Domain Access Permissions */
	uint32_t	reserved4[138];
	struct {
		uint32_t mrsa;		/* Memory Region Start Address */
		uint32_t mrea;		/* Memory Region End Address */
		uint32_t mrc;		/* Memory Region Control */
		uint32_t mrvs;		/* Memory Region Violation Status */
	} mem_region[52];
};

struct rdc_pdap_conf {
	enum rdc_pdap_idx	index;	/* Peripheral index */
	uint8_t	domains;	/* Assigned domains */
	uint8_t	lock;	/* Lock */
};

struct rdc_mda_conf {
	enum rdc_mda_idx	index;	/* Master index */
	uint8_t	domain;	/* Assigned domain */
	uint8_t	lock;	/* Lock */
};

#define RDC_MDA_DID_SHIFT	0
#define RDC_MDA_DID_MASK	(0x3 << RDC_MDA_DID_SHIFT)
#define RDC_MDA_LCK_SHIFT	31
#define RDC_MDA_LCK_MASK	(0x1 << RDC_MDA_LCK_SHIFT)

#define RDC_PDAP_DW_SHIFT(domain)	((domain) << 1)
#define RDC_PDAP_DR_SHIFT(domain)	(1 + RDC_PDAP_DW_SHIFT(domain))
#define RDC_PDAP_DW_MASK(domain)	(1 << RDC_PDAP_DW_SHIFT(domain))
#define RDC_PDAP_DR_MASK(domain)	(1 << RDC_PDAP_DR_SHIFT(domain))
#define RDC_PDAP_DRW_MASK(domain)	(RDC_PDAP_DW_MASK(domain) | \
					 RDC_PDAP_DR_MASK(domain))

#define RDC_PDAP_SREQ_SHIFT	30
#define RDC_PDAP_SREQ_MASK	(0x1 << RDC_PDAP_SREQ_SHIFT)
#define RDC_PDAP_LCK_SHIFT	31
#define RDC_PDAP_LCK_MASK	(0x1 << RDC_PDAP_LCK_SHIFT)

int imx_rdc_get_pdap(struct rdc_pdap_conf *p);
int imx_rdc_set_pdap(struct rdc_pdap_conf *p);
int imx_rdc_set_peripherals(struct rdc_pdap_conf *peripheral_list,
				uint32_t count);
void imx_rdc_set_peripherals_default(void);
int imx_rdc_get_mda(struct rdc_mda_conf *p);
int imx_rdc_set_mda(struct rdc_mda_conf *p);
int imx_rdc_set_masters(struct rdc_mda_conf *masters_list, uint32_t count);
void imx_rdc_set_masters_default(void);

#if defined (CSU_RDC_TEST)
void rdc_test(void);
#endif
#endif	/* __IMX_RDC_H__*/
