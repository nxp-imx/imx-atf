/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <lib/mmio.h>

#include <plat_imx8.h>

#define PCC_PR	BIT(31)
#define PFD_VALID_MASK	U(0x40404040)

#define S400_MU_BASE	U(0x27020000)
#define S400_MU_RSR	(S400_MU_BASE + 0x12c)
#define S400_MU_TRx(i)	(S400_MU_BASE + 0x200 + (i) * 4)
#define S400_MU_RRx(i)	(S400_MU_BASE + 0x280 + (i) * 4)

/*
 * need to re-init the PLL, CGC1, PCC, CMC, XRDC, SIM, GPIO etc.
 * init the PLL &PFD first, then switch the CA35 clock to PLL for
 * performance consideration, resotre other bus fabric clock.
 */

extern void imx8ulp_caam_init();
extern void upower_wait_resp();

struct plat_gic_ctx imx_gicv3_ctx;
static uint32_t cmc1_pmprot;
static uint32_t cmc1_srie;

/* CGC1 PLL2 */
uint32_t pll2[][2] = {
	{0x292c0510, 0x0}, {0x292c0518, 0x0}, {0x292c051c, 0x0},
	{0x292c0520, 0x0}, {0x292c0500, 0x0},
};

/* CGC1 PLL3 */
uint32_t pll3[][2] = {
	{0x292c0604, 0x0}, {0x292c0608, 0x0}, {0x292c060c, 0x0},
	{0x292c0610, 0x0}, {0x292c0618, 0x0}, {0x292c061c, 0x0},
	{0x292c0620, 0x0}, {0x292c0624, 0x0}, {0x292c0600, 0x0},
	{0x292c0614, 0x0},
};

/* CGC1 others */
uint32_t cgc1[][2] = {
	{0x292c0014, 0x0}, {0x292c0034, 0x0}, {0x292c0038, 0x0},
	{0x292c0108, 0x0}, {0x292c0208, 0x0}, {0x292c0700, 0x0},
	{0x292c0810, 0x0}, {0x292c0900, 0x0}, {0x292c0904, 0x0},
	{0x292c0908, 0x0}, {0x292c090c, 0x0}, {0x292c0a00, 0x0},
};

static uint32_t pcc3[61];
static uint32_t pcc4[32];

static uint32_t pcc5_0[33];
static uint32_t pcc5_1[][2] = {
	{0x2da70084, 0x0}, {0x2da70088, 0x0}, {0x2da7008c, 0x0},
	{0x2da700a0, 0x0}, {0x2da700a4, 0x0}, {0x2da700a8, 0x0},
	{0x2da700ac, 0x0}, {0x2da700b0, 0x0}, {0x2da700b4, 0x0},
	{0x2da700bc, 0x0}, {0x2da700c0, 0x0}, {0x2da700c8, 0x0},
	{0x2da700cc, 0x0}, {0x2da700d0, 0x0}, {0x2da700f0, 0x0},
	{0x2da700f4, 0x0}, {0x2da700f8, 0x0}, {0x2da70108, 0x0},
	{0x2da7010c, 0x0}, {0x2da70110, 0x0}, {0x2da70114, 0x0},
};

static uint32_t cgc2[][2] = {
	{0x2da60014, 0x0}, {0x2da60020, 0x0}, {0x2da6003c, 0x0},
	{0x2da60040, 0x0}, {0x2da60108, 0x0}, {0x2da60208, 0x0},
	{0x2da60900, 0x0}, {0x2da60904, 0x0}, {0x2da60908, 0x0},
	{0x2da60910, 0x0}, {0x2da60a00, 0x0},
};

static uint32_t pll4[][2] = {
	{0x2da60604, 0x0}, {0x2da60608, 0x0}, {0x2da6060c, 0x0},
	{0x2da60610, 0x0}, {0x2da60618, 0x0}, {0x2da6061c, 0x0},
	{0x2da60620, 0x0}, {0x2da60624, 0x0}, {0x2da60600, 0x0},
	{0x2da60614, 0x0},
};

static uint32_t lpav_sim[][2] = {
	{0x2da50000, 0x0}, {0x2da50004, 0x0}, {0x2da50008, 0x0},
	{0x2da5001c, 0x0}, {0x2da50020, 0x0}, {0x2da50024, 0x0},
	{0x2da50034, 0x0},
};

#define GPIO_CTRL_NUM		3
#define GPIO_PIN_MAX_NUM	32
#define GPIO_CTX(addr, num)	\
	{.base = (addr), .pin_num = (num), }

struct gpio_ctx {
	/* gpio base */
	uintptr_t base;
	/* port control */
	uint32_t port_ctrl[7];
	/* GPIO ICR, Max 32 */
	uint32_t pin_num;
	uint32_t gpio_icr[GPIO_PIN_MAX_NUM];
};

static uint32_t gpio_ctrl_offset[7] = { 0xc, 0x10, 0x14, 0x18, 0x1c, 0x54, 0x58 };
static struct gpio_ctx gpio_ctx[GPIO_CTRL_NUM] = {
	GPIO_CTX(IMX_GPIOD_BASE, 24),
	GPIO_CTX(IMX_GPIOE_BASE, 24),
	GPIO_CTX(IMX_GPIOF_BASE, 32),
};

/* iomuxc setting */
#define IOMUXC_SECTION_NUM	8
struct iomuxc_section {
	uint32_t offset;
	uint32_t reg_num;
};

struct iomuxc_section iomuxc_sections[IOMUXC_SECTION_NUM] = {
	{.offset = IOMUXC_PTD_PCR_BASE, .reg_num = 24},
	{.offset = IOMUXC_PTE_PCR_BASE, .reg_num = 24},
	{.offset = IOMUXC_PTF_PCR_BASE, .reg_num = 32},
	{.offset = IOMUXC_PSMI_BASE0, .reg_num = 10},
	{.offset = IOMUXC_PSMI_BASE1, .reg_num = 61},
	{.offset = IOMUXC_PSMI_BASE2, .reg_num = 12},
	{.offset = IOMUXC_PSMI_BASE3, .reg_num = 20},
	{.offset = IOMUXC_PSMI_BASE4, .reg_num = 75},
};
static uint32_t iomuxc_ctx[258];

void iomuxc_save(void)
{
	int i, j;
	unsigned index = 0;

	for (i = 0; i < IOMUXC_SECTION_NUM; i++) {
		for (j = 0; j < iomuxc_sections[i].reg_num; j++) {
			iomuxc_ctx[index++] = mmio_read_32(iomuxc_sections[i].offset + j * 4);
		}
	}
}

void iomuxc_restore(void)
{
	int i, j;
	unsigned index = 0;

	for (i = 0; i < IOMUXC_SECTION_NUM; i++) {
		for (j = 0; j < iomuxc_sections[i].reg_num; j++) {
			mmio_write_32(iomuxc_sections[i].offset + j * 4, iomuxc_ctx[index++]);
		}
	}
}

void gpio_save(void)
{
	unsigned int i, j;
	struct gpio_ctx *ctx;

	for (i = 0; i < GPIO_CTRL_NUM; i++) {
		ctx = &gpio_ctx[i];

		/* save the port control setting */
		for (j = 0; j < 7; j++)
			ctx->port_ctrl[j] = mmio_read_32(ctx->base + gpio_ctrl_offset[j]);
		/*
		 * clear the permission setting to read the GPIO non-secure world setting.
		 */
		for (j = 0; j < 7; j++)
			mmio_write_32(ctx->base + gpio_ctrl_offset[j], 0x0);

		/* save the gpio icr setting */
		for (j = 0; j < ctx->pin_num; j++)
			ctx->gpio_icr[j] = mmio_read_32(ctx->base + 0x80 + j * 4);
	}
}

void gpio_restore(void)
{
	unsigned int i, j;
	struct gpio_ctx *ctx;

	for (i = 0; i < GPIO_CTRL_NUM; i++) {
		ctx = &gpio_ctx[i];
		for (j = 0; j < ctx->pin_num; j++)
			mmio_write_32(ctx->base + 0x80 + j * 4, ctx->gpio_icr[j]);

		for (j = 0; j < 7; j++)
			mmio_write_32( ctx->base + gpio_ctrl_offset[j], ctx->port_ctrl[j]);
	}
}

void cgc1_save(void)
{
	int i;

	/* PLL2 */
	for (i = 0; i < ARRAY_SIZE(pll2); i++)
		pll2[i][1] = mmio_read_32(pll2[i][0]);

	/* PLL3 */
	for (i = 0; i < ARRAY_SIZE(pll3); i++)
		pll3[i][1] = mmio_read_32(pll3[i][0]);

	/* CGC1 others */
	for (i = 0; i < ARRAY_SIZE(cgc1); i++)
		cgc1[i][1] = mmio_read_32(cgc1[i][0]);
}

void cgc1_restore(void)
{
	int i;

	/* PLL2 */
	for (i = 0; i < ARRAY_SIZE(pll2); i++)
		mmio_write_32(pll2[i][0], pll2[i][1]);
	/* wait for PLL2 lock */
	while (!(mmio_read_32(pll2[4][0]) & BIT(24)))
		;

	/* PLL3 */
	for (i = 0; i < 9; i++)
		mmio_write_32(pll3[i][0], pll3[i][1]);

	/* wait for PLL3 lock */
	while (!(mmio_read_32(pll3[4][0]) & BIT(24)))
		;
	/* restore the PFDs */
	mmio_write_32(pll3[9][0], pll3[9][1] & ~(BIT(31) | BIT(23) | BIT(15) | BIT(7)));
	mmio_write_32(pll3[9][0], pll3[9][1]);

	/* wait for the PFD is stable, only need to check the enabled PFDs */
	while (!(mmio_read_32(pll3[9][0]) & PFD_VALID_MASK))
		;

	/* CGC1 others */
	for (i = 0; i < ARRAY_SIZE(cgc1); i++)
		mmio_write_32(cgc1[i][0], cgc1[i][1]);
}

static uint32_t tpm5[3];

void tpm5_save(void)
{
	tpm5[0] = mmio_read_32(0x29340010);
	tpm5[1] = mmio_read_32(0x29340018);
	tpm5[2] = mmio_read_32(0x29340020);
}

void tpm5_restore(void)
{
	mmio_write_32(0x29340010, tpm5[0]);
	mmio_write_32(0x29340018, tpm5[1]);
	mmio_write_32(0x29340020, tpm5[2]);
}

static uint32_t wdog3[2];
void wdog3_save(void)
{
	/* enable wdog3 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xa8, 0xd2800000);

	/* save the CS & TOVAL regiter */
	wdog3[0] = mmio_read_32(IMX_WDOG3_BASE);
	wdog3[1] = mmio_read_32(IMX_WDOG3_BASE + 0x8);
}

void wdog3_restore(void)
{
	/* enable wdog3 clock */
	mmio_write_32(IMX_PCC3_BASE + 0xa8, 0xd2800000);

	/* unlock the wdog */
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0xc520);
	mmio_write_32(IMX_WDOG3_BASE + 0x4, 0xd928);

	dsb();

	/* wait for the unlock status */
	while(!(mmio_read_32(IMX_WDOG3_BASE) & BIT(11)))
		;

	/* set the tiemout value */
	mmio_write_32(IMX_WDOG3_BASE + 0x8, wdog3[1]);
	/* reconfig the CS */
	mmio_write_32(IMX_WDOG3_BASE, wdog3[0]);

	/* wait for the config done */
	while(!(mmio_read_32(IMX_WDOG3_BASE) & BIT(10)))
		;
}

static uint32_t lpuart_regs[4];
#define LPUART_BAUD     0x10
#define LPUART_CTRL     0x18
#define LPUART_FIFO     0x28
#define LPUART_WATER    0x2c

void lpuart_save(void)
{
	lpuart_regs[0] = mmio_read_32(IMX_LPUART5_BASE + LPUART_BAUD);
	lpuart_regs[1] = mmio_read_32(IMX_LPUART5_BASE + LPUART_FIFO);
	lpuart_regs[2] = mmio_read_32(IMX_LPUART5_BASE + LPUART_WATER);
	lpuart_regs[3] = mmio_read_32(IMX_LPUART5_BASE + LPUART_CTRL);
}

void lpuart_restore(void)
{
	mmio_write_32(IMX_LPUART5_BASE + LPUART_BAUD, lpuart_regs[0]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_FIFO, lpuart_regs[1]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_WATER, lpuart_regs[2]);
	mmio_write_32(IMX_LPUART5_BASE + LPUART_CTRL, lpuart_regs[3]);
}


extern void dram_enter_retention(void);
extern void dram_exit_retention(void);

bool is_lpav_owned_by_apd(void)
{
	return (mmio_read_32(0x2802b044) & BIT(7)) ? true : false;
}

void lpav_ctx_save(void)
{
	int i;

	/* CGC2 save */
	for (i = 0; i < ARRAY_SIZE(cgc2); i++)
		cgc2[i][1] = mmio_read_32(cgc2[i][0]);

	/* PLL4 */
	for (i = 0; i <ARRAY_SIZE(pll4); i++)
		pll4[i][1] = mmio_read_32(pll4[i][0]);

	/* PCC5 save */
	for (i = 0; i < ARRAY_SIZE(pcc5_0); i++)
		pcc5_0[i] = mmio_read_32(IMX_PCC5_BASE + i * 4); 

	for (i = 0; i < ARRAY_SIZE(pcc5_1); i++)
		pcc5_1[i][1] = mmio_read_32(pcc5_1[i][0]);

	/* LPAV SIM save */
	for (i = 0; i < ARRAY_SIZE(lpav_sim); i++)
		lpav_sim[i][1] = mmio_read_32(lpav_sim[i][0]);

	/* put DDR into retention */
	dram_enter_retention();
}

void lpav_ctx_restore(void)
{
	int i;

	/* PLL4 */
	for (i = 0; i < 9; i++)
		mmio_write_32(pll4[i][0], pll4[i][1]);

	/* wait for PLL4 lock */
	while (!(mmio_read_32(pll4[8][0]) & BIT(24)))
		;
	/* restore the PLL4 PFDs */
	mmio_write_32(pll4[9][0], pll4[9][1] & ~(BIT(31) | BIT(23) | BIT(15) | BIT(7)));
	mmio_write_32(pll4[9][0], pll4[9][1]);

	/* wait for the PFD is stable */
	while (!(mmio_read_32(pll4[9][0]) & PFD_VALID_MASK))
		;

	/* CGC2 restore */
	for (i = 0; i < ARRAY_SIZE(cgc2); i++)
		mmio_write_32(cgc2[i][0], cgc2[i][1]);

	/* PCC5 restore */
	for (i = 0; i < ARRAY_SIZE(pcc5_0); i++)
		mmio_write_32(IMX_PCC5_BASE + i * 4, pcc5_0[i]);

	for (i = 0; i < ARRAY_SIZE(pcc5_1); i++)
		mmio_write_32(pcc5_1[i][0], pcc5_1[i][1]);

	/* LPAV_SIM */
	for (i = 0; i < ARRAY_SIZE(lpav_sim); i++)
		mmio_write_32(lpav_sim[i][0], lpav_sim[i][1]);

	/* DDR retention exit */
	dram_exit_retention();
}

/* XRDC context */
void xrdc_init_mda(void)
{
	int i;

	for (i = 3; i <= 5; i++) {
		mmio_write_32(0x292f0000 + 0x800 + i * 0x20, 0x200000a1);
		mmio_write_32(0x292f0000 + 0x800 + i * 0x20, 0xa00000a1);
	}

	for (i = 10; i <= 15; i++) {
		mmio_write_32(0x292f0000 + 0x800 + i * 0x20, 0x200000a3);
		mmio_write_32(0x292f0000 + 0x800 + i * 0x20, 0xa00000a3);
	}
}

int xrdc_config_mrc_dx_perm(uint32_t mrc_con, uint32_t region, uint32_t dom, uint32_t dxsel)
{
	uint32_t val = 0;

	val = (mmio_read_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20 + 0x8) & (~(7 << (3 * dom)))) | (dxsel << (3 * dom));
	mmio_write_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20 + 0x8, val);

	return 0;
}

int xrdc_config_mrc_w0_w1(uint32_t mrc_con, uint32_t region, uint32_t w0, uint32_t size)
{
	if ((size % 32) != 0)
		return -1;

	mmio_write_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20, w0 & ~0x1f);
	mmio_write_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20 + 0x4, w0 + size -1);

	return 0;
}

int xrdc_config_mrc_w3_w4(uint32_t mrc_con, uint32_t region, uint32_t w3, uint32_t w4)
{
	mmio_write_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20 + 0xc, w3);
	mmio_write_32(0x292f0000 + 0x2000 + mrc_con * 0x200 + region * 0x20 + 0xc + 0x4, w4);

        return 0;
}

void xrdc_init_mrc(void)
{
	/* The MRC8 is for SRAM1 */
	xrdc_config_mrc_w0_w1(8, 0, 0x21000000, 0x10000);
	/* Allow for all domains: So domain 2/3 (HIFI DSP/LPAV) is ok to access */
	xrdc_config_mrc_dx_perm(8, 0, 0, 1);
	xrdc_config_mrc_dx_perm(8, 0, 1, 1);
	xrdc_config_mrc_dx_perm(8, 0, 2, 1);
	xrdc_config_mrc_dx_perm(8, 0, 3, 1);
	xrdc_config_mrc_dx_perm(8, 0, 4, 1);
	xrdc_config_mrc_dx_perm(8, 0, 5, 1);
	xrdc_config_mrc_dx_perm(8, 0, 6, 1);
	xrdc_config_mrc_dx_perm(8, 0, 7, 1);
	xrdc_config_mrc_w3_w4(8, 0, 0x0, 0x80000FFF);
	
	/* The MRC6 is for video modules to ddr */
	xrdc_config_mrc_w0_w1(6, 0, 0x80000000, 0x80000000);
	xrdc_config_mrc_dx_perm(6, 0, 3, 1); /* allow for domain 3 video */
	xrdc_config_mrc_w3_w4(6, 0, 0x0, 0x80000FFF);
}

void imx_apd_ctx_save(unsigned int proc_num)
{
	unsigned int i;
	uint32_t val;

	/* enable LPUART5's clock by default */
	mmio_setbits_32(IMX_PCC3_BASE + 0xe8, BIT(30));

	/* save the gic config */
	plat_gic_save(proc_num, &imx_gicv3_ctx);

	cmc1_pmprot = mmio_read_32(IMX_CMC1_BASE + 0x18);
	cmc1_srie = mmio_read_32(IMX_CMC1_BASE + 0x8c);

	/* save the PCC3 */
	for (i = 0; i < ARRAY_SIZE(pcc3); i++) {
		/* save the pcc if it is exist */
		val = mmio_read_32(IMX_PCC3_BASE + i * 4);
		if (val & PCC_PR)
			pcc3[i] = val;
	}

	/* save the PCC4 */
	for (i = 0; i < ARRAY_SIZE(pcc4); i++) {
		/* save the pcc if it is exist */
		val = mmio_read_32(IMX_PCC4_BASE + i * 4);
		if (val & PCC_PR)
			pcc4[i] = val;
	}

	/* save the CGC1 */
	cgc1_save();

	wdog3_save();

	gpio_save();

	iomuxc_save();

	tpm5_save();

	lpuart_save();

	/*
	 * save the lpav ctx & put the ddr into retention
	 * if lpav master is assigned to APD domain.
	 */
	if (is_lpav_owned_by_apd()) {
		lpav_ctx_save();
	}
}

void xrdc_reinit(void)
{
	xrdc_init_mda();
	xrdc_init_mrc();
}

void s400_release_caam(void)
{
	uint32_t msg, resp;

	mmio_write_32(S400_MU_TRx(0), 0x17d70206);
	mmio_write_32(S400_MU_TRx(1), 0x7);

	do {
		resp = mmio_read_32(S400_MU_RSR);
	} while ((resp & 0x3) != 0x3);

	msg = mmio_read_32(S400_MU_RRx(0));
	resp = mmio_read_32(S400_MU_RRx(1));

	VERBOSE("resp %x; %x", msg, resp);
}

void imx_apd_ctx_restore(unsigned int proc_num)
{
	unsigned int i;

	/* restore the CCG1 */
	cgc1_restore();

	for (i = 0; i < ARRAY_SIZE(pcc3); i++) {
		/* save the pcc if it is exist */
		if (pcc3[i] & PCC_PR)
			mmio_write_32(IMX_PCC3_BASE + i * 4, pcc3[i]);
	}

	for (i = 0; i < ARRAY_SIZE(pcc4); i++) {
		if (pcc4[i] & PCC_PR)
			mmio_write_32(IMX_PCC4_BASE + i * 4, pcc4[i]);
	}

	wdog3_restore();

	iomuxc_restore();

	gpio_restore();

	tpm5_restore();

//	xrdc_reinit();

	/* restore the gic config */
	plat_gic_restore(proc_num, &imx_gicv3_ctx);

	mmio_write_32(IMX_CMC1_BASE + 0x18, cmc1_pmprot);
	mmio_write_32(IMX_CMC1_BASE + 0x8c, cmc1_srie);

	/* enable LPUART5's clock by default */
	mmio_setbits_32(IMX_PCC3_BASE + 0xe8, BIT(30));

	/* restore the console lpuart */
	lpuart_restore();

	/* FIXME: make uart work for ATF */
	mmio_write_32(0x293a0018, 0xc0000);

	/*
	 * Ask S400 to release caam to APD as it is owned by s400
	 */
	s400_release_caam();

	/* re-init the caam */
	imx8ulp_caam_init();

	/*
	 * ack the upower, seems a necessary steps, otherwise the upower can
	 * not response to the new API service call. put this just before the
	 * ddr retention exit because that the dram retention exit flow need to
	 * communicate with upower.
	 */
	upower_wait_resp();

	/*
	 * restore the lpav ctx & make ddr out of retention
	 * if lpav master is assigned to APD domain.
	 */
	if (is_lpav_owned_by_apd()) {
		lpav_ctx_restore();
	}
}

#define DGO_CTRL1	U(0xc)
#define USB_WAKEUP	U(0x44)
#define USB1_PHY_DPD_WAKEUP_EN	BIT_32(5)
#define USB0_PHY_DPD_WAKEUP_EN	BIT_32(4)
#define USB1_PHY_WAKEUP_ISO_DISABLE	BIT_32(1)
#define USB0_PHY_WAKEUP_ISO_DISABLE	BIT_32(0)

void usb_wakeup_enable(bool enable)
{
	if (enable) {
		mmio_write_32(IMX_SIM1_BASE + USB_WAKEUP, USB1_PHY_DPD_WAKEUP_EN | USB0_PHY_DPD_WAKEUP_EN |
			USB1_PHY_WAKEUP_ISO_DISABLE | USB0_PHY_WAKEUP_ISO_DISABLE);
		mmio_setbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		while (!(mmio_read_32(IMX_SIM1_BASE + DGO_CTRL1) & BIT(1)))
			;
		mmio_clrbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(1));
	} else {
		/*
		 * USBx_PHY_DPD_WAKEUP_EN should be cleared before USB0_PHY_WAKEUP_ISO_DISABLE
		 * to provide the correct the wake-up functionality.
		 */
		mmio_write_32(IMX_SIM1_BASE + USB_WAKEUP, USB1_PHY_WAKEUP_ISO_DISABLE |
			USB0_PHY_WAKEUP_ISO_DISABLE);
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		while (!(mmio_read_32(IMX_SIM1_BASE + DGO_CTRL1) & BIT(1)))
			;
		mmio_clrbits_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(0));
		mmio_write_32(IMX_SIM1_BASE + DGO_CTRL1, BIT(1));
	}
}
