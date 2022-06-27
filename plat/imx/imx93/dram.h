#include <lib/utils_def.h>
#include <arch_helpers.h>
#include <assert.h>

#define DDR_CTL_BASE		0x4E300000
#define DDR_PHY_BASE		0x4E100000
#define IP2APB_DDRPHY_IPS_BASE_ADDR(X)		(DDR_PHY_BASE + (X * 0x2000000))

#define dwc_ddrphy_apb_wr(addr, data) \
        mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr), data)
#define dwc_ddrphy_apb_rd(addr) \
        mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + ddrphy_addr_remap(addr))

/* reg & config param */
struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_timing_info {
	/* umctl2 config */
	struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
	/* ddrphy config */
	struct dram_cfg_param *ddrphy_cfg;
	unsigned int ddrphy_cfg_num;
	/* ddr fsp train info */
	struct dram_fsp_msg *fsp_msg;
	unsigned int fsp_msg_num;
	/* ddr phy trained CSR */
	struct dram_cfg_param *ddrphy_trained_csr;
	unsigned int ddrphy_trained_csr_num;
	/* ddr phy PIE */
	struct dram_cfg_param *ddrphy_pie;
	unsigned int ddrphy_pie_num;
	/* initialized fsp table */
	unsigned int fsp_table[4];
};

void dram_enter_retention(void);
void dram_exit_retention(void);

