#include <lib/utils_def.h>

#include <lib/mmio.h>
#include <drivers/delay_timer.h>

#include <platform_def.h>
#include "dram.h"

#define DDRC_BASE		U(0x4E300000)
#define DDR_SDRAM_CFG		(DDRC_BASE + 0x110)
#define DDR_SDRAM_CFG_2		(DDRC_BASE + 0x114)
#define DDR_SDRAM_MD_CNTL	(DDRC_BASE + 0x120)
#define DDR_SDRAM_INTERVAL	(DDRC_BASE + 0x124)
#define DDR_ZQ_CNTL		(DDRC_BASE + 0x170)
#define DDR_SR_CNTR		(DDRC_BASE + 0x17c)
#define DDR_SDRAM_CFG_3		(DDRC_BASE + 0x260)
#define DDR_SDRAM_CFG_4		(DDRC_BASE + 0x264)
#define DDRDSR_2		(DDRC_BASE + 0xB24)
#define DEBUG_26		(DDRC_BASE + 0xF64)

/* DDRMIX_BLK_CTRL */
#define DDRC_STOP_CTRL		(0x4E01000C)
#define AUTO_CG_CTRL		(0x4E010010)

#define DRAM_PLL_BASE		U(0x44481300)

#define PLL_CTRL	U(0x0)
#define PLL_CTRL_SET	U(0x4)
#define PLL_CTRL_CLR	U(0x8)
#define PLL_CTRL_TOG	U(0xC)

#define PLL_SPREAD_SPECTRUM		U(0x30)
#define PLL_SPREAD_SPECTRUM_SET		U(0x34)
#define PLL_SPREAD_SPECTRUM_CLR		U(0x38)
#define PLL_SPREAD_SPECTRUM_TOG		U(0x3C)

#define PLL_NUMERATOR			U(0x40)
#define PLL_DENOMINATOR			U(0x50)

#define PLL_DIV				U(0x60)
#define PLL_STATUS			U(0xF0)

#define SRC_DDRC_SW_CTRL		U(0x44461020)
#define SRC_DDRPHY_SW_CTRL		U(0x44461420)
#define SRC_DDRPHY_SINGLE_RESET_SW_CTRL	U(0x44461424)

void dram_disable_bypass(void)
{
        /* Set DRAM APB to 133Mhz */
	mmio_write_32(0x44452680, 0x202);
        /* Switch from DRAM clock root from CCM to PLL */
	mmio_write_32(0x44454840, 0x0);
}

void dram_pll_init(void)
{
	/* powerup and enable */
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x1);

	/* wait lock */
	while (!(mmio_read_32(DRAM_PLL_BASE + PLL_STATUS) & 0x1)) {
		;
	}

	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_CLR, 0x4);
	mmio_write_32(DRAM_PLL_BASE + PLL_CTRL_SET, 0x2);
}

unsigned long ddrphy_addr_remap(uint32_t paddr_apb_from_ctlr)
{
	uint32_t paddr_apb_qual;
	uint32_t paddr_apb_unqual_dec_22_13;
	uint32_t paddr_apb_unqual_dec_19_13;
	uint32_t paddr_apb_unqual_dec_12_1;
	uint32_t paddr_apb_unqual;
	uint32_t paddr_apb_phy;

	paddr_apb_qual             = (paddr_apb_from_ctlr << 1);
	paddr_apb_unqual_dec_22_13 = ((paddr_apb_qual & 0x7fe000) >> 13);
	paddr_apb_unqual_dec_12_1  = ((paddr_apb_qual & 0x1ffe) >> 1);

	switch (paddr_apb_unqual_dec_22_13) {
	case 0x000: paddr_apb_unqual_dec_19_13 = 0x00; break;
	case 0x001: paddr_apb_unqual_dec_19_13 = 0x01; break;
	case 0x002: paddr_apb_unqual_dec_19_13 = 0x02; break;
	case 0x003: paddr_apb_unqual_dec_19_13 = 0x03; break;
	case 0x004: paddr_apb_unqual_dec_19_13 = 0x04; break;
	case 0x005: paddr_apb_unqual_dec_19_13 = 0x05; break;
	case 0x006: paddr_apb_unqual_dec_19_13 = 0x06; break;
	case 0x007: paddr_apb_unqual_dec_19_13 = 0x07; break;
	case 0x008: paddr_apb_unqual_dec_19_13 = 0x08; break;
	case 0x009: paddr_apb_unqual_dec_19_13 = 0x09; break;
	case 0x00a: paddr_apb_unqual_dec_19_13 = 0x0a; break;
	case 0x00b: paddr_apb_unqual_dec_19_13 = 0x0b; break;
	case 0x100: paddr_apb_unqual_dec_19_13 = 0x0c; break;
	case 0x101: paddr_apb_unqual_dec_19_13 = 0x0d; break;
	case 0x102: paddr_apb_unqual_dec_19_13 = 0x0e; break;
	case 0x103: paddr_apb_unqual_dec_19_13 = 0x0f; break;
	case 0x104: paddr_apb_unqual_dec_19_13 = 0x10; break;
	case 0x105: paddr_apb_unqual_dec_19_13 = 0x11; break;
	case 0x106: paddr_apb_unqual_dec_19_13 = 0x12; break;
	case 0x107: paddr_apb_unqual_dec_19_13 = 0x13; break;
	case 0x108: paddr_apb_unqual_dec_19_13 = 0x14; break;
	case 0x109: paddr_apb_unqual_dec_19_13 = 0x15; break;
	case 0x10a: paddr_apb_unqual_dec_19_13 = 0x16; break;
	case 0x10b: paddr_apb_unqual_dec_19_13 = 0x17; break;
	case 0x200: paddr_apb_unqual_dec_19_13 = 0x18; break;
	case 0x201: paddr_apb_unqual_dec_19_13 = 0x19; break;
	case 0x202: paddr_apb_unqual_dec_19_13 = 0x1a; break;
	case 0x203: paddr_apb_unqual_dec_19_13 = 0x1b; break;
	case 0x204: paddr_apb_unqual_dec_19_13 = 0x1c; break;
	case 0x205: paddr_apb_unqual_dec_19_13 = 0x1d; break;
	case 0x206: paddr_apb_unqual_dec_19_13 = 0x1e; break;
	case 0x207: paddr_apb_unqual_dec_19_13 = 0x1f; break;
	case 0x208: paddr_apb_unqual_dec_19_13 = 0x20; break;
	case 0x209: paddr_apb_unqual_dec_19_13 = 0x21; break;
	case 0x20a: paddr_apb_unqual_dec_19_13 = 0x22; break;
	case 0x20b: paddr_apb_unqual_dec_19_13 = 0x23; break;
	case 0x300: paddr_apb_unqual_dec_19_13 = 0x24; break;
	case 0x301: paddr_apb_unqual_dec_19_13 = 0x25; break;
	case 0x302: paddr_apb_unqual_dec_19_13 = 0x26; break;
	case 0x303: paddr_apb_unqual_dec_19_13 = 0x27; break;
	case 0x304: paddr_apb_unqual_dec_19_13 = 0x28; break;
	case 0x305: paddr_apb_unqual_dec_19_13 = 0x29; break;
	case 0x306: paddr_apb_unqual_dec_19_13 = 0x2a; break;
	case 0x307: paddr_apb_unqual_dec_19_13 = 0x2b; break;
	case 0x308: paddr_apb_unqual_dec_19_13 = 0x2c; break;
	case 0x309: paddr_apb_unqual_dec_19_13 = 0x2d; break;
	case 0x30a: paddr_apb_unqual_dec_19_13 = 0x2e; break;
	case 0x30b: paddr_apb_unqual_dec_19_13 = 0x2f; break;
	case 0x010: paddr_apb_unqual_dec_19_13 = 0x30; break;
	case 0x011: paddr_apb_unqual_dec_19_13 = 0x31; break;
	case 0x012: paddr_apb_unqual_dec_19_13 = 0x32; break;
	case 0x013: paddr_apb_unqual_dec_19_13 = 0x33; break;
	case 0x014: paddr_apb_unqual_dec_19_13 = 0x34; break;
	case 0x015: paddr_apb_unqual_dec_19_13 = 0x35; break;
	case 0x016: paddr_apb_unqual_dec_19_13 = 0x36; break;
	case 0x017: paddr_apb_unqual_dec_19_13 = 0x37; break;
	case 0x018: paddr_apb_unqual_dec_19_13 = 0x38; break;
	case 0x019: paddr_apb_unqual_dec_19_13 = 0x39; break;
	case 0x110: paddr_apb_unqual_dec_19_13 = 0x3a; break;
	case 0x111: paddr_apb_unqual_dec_19_13 = 0x3b; break;
	case 0x112: paddr_apb_unqual_dec_19_13 = 0x3c; break;
	case 0x113: paddr_apb_unqual_dec_19_13 = 0x3d; break;
	case 0x114: paddr_apb_unqual_dec_19_13 = 0x3e; break;
	case 0x115: paddr_apb_unqual_dec_19_13 = 0x3f; break;
	case 0x116: paddr_apb_unqual_dec_19_13 = 0x40; break;
	case 0x117: paddr_apb_unqual_dec_19_13 = 0x41; break;
	case 0x118: paddr_apb_unqual_dec_19_13 = 0x42; break;
	case 0x119: paddr_apb_unqual_dec_19_13 = 0x43; break;
	case 0x210: paddr_apb_unqual_dec_19_13 = 0x44; break;
	case 0x211: paddr_apb_unqual_dec_19_13 = 0x45; break;
	case 0x212: paddr_apb_unqual_dec_19_13 = 0x46; break;
	case 0x213: paddr_apb_unqual_dec_19_13 = 0x47; break;
	case 0x214: paddr_apb_unqual_dec_19_13 = 0x48; break;
	case 0x215: paddr_apb_unqual_dec_19_13 = 0x49; break;
	case 0x216: paddr_apb_unqual_dec_19_13 = 0x4a; break;
	case 0x217: paddr_apb_unqual_dec_19_13 = 0x4b; break;
	case 0x218: paddr_apb_unqual_dec_19_13 = 0x4c; break;
	case 0x219: paddr_apb_unqual_dec_19_13 = 0x4d; break;
	case 0x310: paddr_apb_unqual_dec_19_13 = 0x4e; break;
	case 0x311: paddr_apb_unqual_dec_19_13 = 0x4f; break;
	case 0x312: paddr_apb_unqual_dec_19_13 = 0x50; break;
	case 0x313: paddr_apb_unqual_dec_19_13 = 0x51; break;
	case 0x314: paddr_apb_unqual_dec_19_13 = 0x52; break;
	case 0x315: paddr_apb_unqual_dec_19_13 = 0x53; break;
	case 0x316: paddr_apb_unqual_dec_19_13 = 0x54; break;
	case 0x317: paddr_apb_unqual_dec_19_13 = 0x55; break;
	case 0x318: paddr_apb_unqual_dec_19_13 = 0x56; break;
	case 0x319: paddr_apb_unqual_dec_19_13 = 0x57; break;
	case 0x020: paddr_apb_unqual_dec_19_13 = 0x58; break;
	case 0x120: paddr_apb_unqual_dec_19_13 = 0x59; break;
	case 0x220: paddr_apb_unqual_dec_19_13 = 0x5a; break;
	case 0x320: paddr_apb_unqual_dec_19_13 = 0x5b; break;
	case 0x040: paddr_apb_unqual_dec_19_13 = 0x5c; break;
	case 0x140: paddr_apb_unqual_dec_19_13 = 0x5d; break;
	case 0x240: paddr_apb_unqual_dec_19_13 = 0x5e; break;
	case 0x340: paddr_apb_unqual_dec_19_13 = 0x5f; break;
	case 0x050: paddr_apb_unqual_dec_19_13 = 0x60; break;
	case 0x051: paddr_apb_unqual_dec_19_13 = 0x61; break;
	case 0x052: paddr_apb_unqual_dec_19_13 = 0x62; break;
	case 0x053: paddr_apb_unqual_dec_19_13 = 0x63; break;
	case 0x054: paddr_apb_unqual_dec_19_13 = 0x64; break;
	case 0x055: paddr_apb_unqual_dec_19_13 = 0x65; break;
	case 0x056: paddr_apb_unqual_dec_19_13 = 0x66; break;
	case 0x057: paddr_apb_unqual_dec_19_13 = 0x67; break;
	case 0x070: paddr_apb_unqual_dec_19_13 = 0x68; break;
	case 0x090: paddr_apb_unqual_dec_19_13 = 0x69; break;
	case 0x190: paddr_apb_unqual_dec_19_13 = 0x6a; break;
	case 0x290: paddr_apb_unqual_dec_19_13 = 0x6b; break;
	case 0x390: paddr_apb_unqual_dec_19_13 = 0x6c; break;
	case 0x0c0: paddr_apb_unqual_dec_19_13 = 0x6d; break;
	case 0x0d0: paddr_apb_unqual_dec_19_13 = 0x6e; break;
	default: paddr_apb_unqual_dec_19_13 = 0x00;
	}
	paddr_apb_unqual = ((paddr_apb_unqual_dec_19_13 << 13) | (paddr_apb_unqual_dec_12_1 << 1));
	paddr_apb_phy = (paddr_apb_unqual << 1);
	return paddr_apb_phy;
}

void self_refresh_enter()
{
	mmio_setbits_32(DDR_SDRAM_CFG_2, BIT(31));
}

void self_refresh_exit()
{
	mmio_clrbits_32(DDR_SDRAM_CFG_2,BIT(31));
}

static void ipg_stop_ack_wait(uint32_t expect_value)
{
	uint32_t read_data;
	read_data = mmio_read_32(DDRC_STOP_CTRL);
	if (expect_value == 0x1) {
		while ((read_data & BIT(1)) == 0x0){
			read_data= mmio_read_32(DDRC_STOP_CTRL);
		}
	} else if (expect_value == 0x0){
		while ((read_data & BIT(1)) != 0x0){
			read_data = mmio_read_32(DDRC_STOP_CTRL);
		}
	}
}

void check_dfi_init_complete(void)
{
	uint32_t regval;

	do {
		regval = mmio_read_32(DDRDSR_2);
 		if (regval & 0x04) {
			break;
		}
	} while (1);

	mmio_setbits_32(DDRDSR_2, BIT(2));
}

#define DDRC_IDLE	BIT(31)
void check_ddrc_idle(void)
{
	uint32_t val;

	do {
		val = mmio_read_32(DDRDSR_2);
		if (val & DDRC_IDLE) {
			break;
		}
	} while (1);
}

void dram_enter_retention(void)
{
	uint32_t val;

	/* 2. Polling for DDRDSR_2[IDLE] to be set */
	do {
		val = mmio_read_32(DDRDSR_2);
		if (val & DDRC_IDLE) {
			break;
		}
	} while (1);

	/* HALT the ddrc axi port */
	mmio_setbits_32(DDR_SDRAM_CFG, BIT(1));

	/* 3. Set DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_setbits_32(DDR_SDRAM_CFG_3, BIT(1));
	if ((mmio_read_32(DDR_SDRAM_CFG_3) & 0x2) != 0) {
		NOTICE("SR_FAST_WK_EN bit set success\n");
	} else {
		NOTICE("SR_FAST_WK_EN bit set failed\n");
	}

	/* 4. Program DDR_SDRAM_CFG_4[DFI_FREQ] to 2b'1111 */
	val = mmio_read_32(DDR_SDRAM_CFG_4);
	mmio_write_32(DDR_SDRAM_CFG_4, val | 0x1f000);
	if ((mmio_read_32(DDR_SDRAM_CFG_4) & 0x1f000) != 0x1f000) {
		NOTICE("DFI_FREQ bits set success\n");
	} else {
		NOTICE("DFI_FREQ bit set failed\n");
	}

	/* 5. Clear DDR_ZQ_CNTL register */
	mmio_write_32(DDR_ZQ_CNTL, 0x0);

	/* 6. Set DEBUG_26[DIS_CTRLUPD_REQ */
	mmio_setbits_32(DEBUG_26, (0x1f << 12));

	/* 7. Force the DDRC to enter self refresh */
	self_refresh_enter();
	mmio_setbits_32(AUTO_CG_CTRL, BIT(17)); /* Enable ipg_stop_reg while auto cg */
	mmio_setbits_32(DDR_SDRAM_CFG, BIT(30)); /* enable SR during sleep */
	mmio_setbits_32(DDR_SDRAM_CFG_3, BIT(11)); /* drain for self refresh */

	/* 8. Set stop and poll stop ack */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(DDRC_STOP_CTRL, BIT(0));
	ipg_stop_ack_wait(0x1);

	/* 9. Clear DDR_INTERVAL(this disables refreshes */
	mmio_write_32(DDR_SDRAM_INTERVAL, 0x0);

	/* 10. Set DDR_SDRAM_MD_CNTL to 0x00100000(this forces CKE to remain low */
	mmio_write_32(DDR_SDRAM_MD_CNTL, 0x00100000);

	/* 11. Remove Stop request via DDRMIX register */
	mmio_clrbits_32(DDRC_STOP_CTRL, BIT(0));
	check_dfi_init_complete();

	/* 12. Clear DDR_SDRAM_CFG[SREN] */
	mmio_clrbits_32(DDR_SDRAM_CFG, BIT(30));

	/* 13. Clear DDR_SDRAM_CFG_3[SR_FAST_WK_EN] */
	mmio_clrbits_32(DDR_SDRAM_CFG_3, BIT(1));

	/* 14. Set stop request again via DDRMIX register */
	ipg_stop_ack_wait(0x0);
	mmio_setbits_32(DDRC_STOP_CTRL, BIT(0));

	/* 15. Poll for STOP_ACK to be asserted in DDRMIX register */
	ipg_stop_ack_wait(0x1);

	/* 16. Set STOP request again via DDRMIX register */
	mmio_clrbits_32(DDRC_STOP_CTRL, BIT(0));

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if (!(dwc_ddrphy_apb_rd(0x90028) & 0x1))
		INFO("PhyInLP3 = 1\n");
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* 17. Clear PwrOkIn via DDRMIX regsiter */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));

	/* 18. Power off the DDRMIX */
	NOTICE("Enable ddr power off\r\n");
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(31));
	NOTICE("enter retention done\n");
}

/* Restore the dram PHY config */
void dram_phy_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *cfg = timing->ddrphy_cfg;
	unsigned int i;

	/* Restore the PHY init config */
	cfg = timing->ddrphy_cfg;
	for (i = 0U; i < timing->ddrphy_cfg_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_wr(0xc0080, 0x3);
	/* Restore the DDR PHY CSRs */
	cfg = timing->ddrphy_trained_csr;
	for (i = 0U; i < timing->ddrphy_trained_csr_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	dwc_ddrphy_apb_wr(0xc0080, 0x2);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* Load the PIE image */
	cfg = timing->ddrphy_pie;
	for (i = 0U; i < timing->ddrphy_pie_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}
}

void ddrc_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *ddrc_cfg = timing->ddrc_cfg;
	unsigned int i;
	uint32_t val;

	for (i = 0;  i < timing->ddrc_cfg_num; i++) {
		mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}

	check_dfi_init_complete();

	val = mmio_read_32(DDR_SDRAM_CFG);
	mmio_write_32(DDR_SDRAM_CFG, val | BIT(31));

	check_ddrc_idle();
}

void ddrphy_coldreset(void) {
	/*
	 * dramphy_apb_n default 1 , assert -> 0, de_assert -> 1
	 * dramphy_reset_n default 0 , assert -> 0, de_assert -> 1
	 * dramphy_PwrOKIn default 0 , assert -> 1, de_assert -> 0
	 */
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	/* src_gen_dphy_PwrOKIn_sw_rst_de_assert() */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);

	/* src_gen_dphy_apb_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_assert */
	mmio_setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
	udelay(10);

	/* src_gen_dphy_PwrOKIn_sw_rst_assert */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
	udelay(10);
	/* src_gen_dphy_apb_sw_rst_de_assert */
	mmio_clrbits_32(SRC_DDRPHY_SW_CTRL, BIT(0));
	/* src_gen_dphy_sw_rst_de_assert() */
	mmio_clrbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(2));
}

void dram_exit_retention(void)
{
	struct dram_timing_info *timing = (struct dram_timing_info *)SAVED_DRAM_TIMING_BASE;

	/* 1. Power up the DDRMIX */
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(31));

	/* additional step to make sure DDR exit retenton works */
	mmio_setbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10000);
	mmio_clrbits_32(SRC_DDRC_SW_CTRL, BIT(0));
	udelay(10000);

	/* 2. Cold reset the DDRPHY */
	ddrphy_coldreset();

	/* 3. Config the DRAM PLL to FSP0 */
	dram_pll_init();
	dram_disable_bypass();

	/* 4. Reload the DDRPHY config */
	dram_phy_init(timing);
	printf("phy reload done\n");

	/* 5. Reload the ddrc config */
	ddrc_init(timing);

	NOTICE("exit retention done\n");
}
