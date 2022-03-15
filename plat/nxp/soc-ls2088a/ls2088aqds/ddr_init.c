/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

#include "platform_def.h"

#ifdef CONFIG_STATIC_DDR
#error No static value defined
#endif

static const struct rc_timing rce_1[] = {
	{U(1600), U(8), U(7)},
	{U(1867), U(8), U(7)},
	{U(2134), U(8), U(8)},
	{}
};

static const struct board_timing udimm1[] = {
	{U(0x04), rce_1, U(0x01020407), U(0x090A0B04)},
};

static const struct rc_timing rce_2[] = {
	{U(1600), U(8), U(0xD)},
	{}
};

static const struct board_timing udimm2[] = {
	{U(0x04), rce_2, U(0xFEFCFD00), U(0x000000FC)},
};

static const struct rc_timing rcb[] = {
	{U(1600), U(8), U(7)},
	{U(1867), U(8), U(7)},
	{U(2134), U(8), U(8)},
	{}
};

static const struct board_timing rdimm[] = {
	{U(0x01), rcb, U(0xFEFCFAFA), U(0xFAFCFEF9)},
	{U(0x04), rcb, U(0xFEFCFAFA), U(0xFAFCFEF9)},
};

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;
	struct dimm_params *pdimm = &priv->dimm;
	const struct ddr_conf *conf = &priv->conf;
	const unsigned long ddr_freq = priv->clk / 1000000;
	unsigned int dq_mapping_0, dq_mapping_2, dq_mapping_3;
	int ret;
	int i;
	int is_dpddr = 0;

	if (NXP_DDR3_ADDR == (unsigned long)priv->ddr[0]) { /* DP-DDR */
		is_dpddr = 1;
		if (popts->rdimm) {
			ERROR("RDIMM parameters not set.\n");
			return -EINVAL;
		}
		ret = cal_board_params(priv, udimm2, ARRAY_SIZE(udimm2));
	} else {
		if (popts->rdimm) {
			ret = cal_board_params(priv, rdimm,
					       ARRAY_SIZE(rdimm));
		} else {
			ret = cal_board_params(priv, udimm1,
					       ARRAY_SIZE(udimm1));
		}
	}
	if (ret) {
		return ret;
	}

	popts->cpo_sample = U(0x70);

	if (is_dpddr) {
		/* DPDDR bus width 32 bits */
		popts->data_bus_used = DDR_DBUS_32;
		popts->otf_burst_chop_en = 0;
		popts->burst_length = DDR_BL8;
		popts->bstopre = 0;	/* enable auto precharge */
		/*
		 * Layout optimization results byte mapping
		 * Byte 0 -> Byte ECC
		 * Byte 1 -> Byte 3
		 * Byte 2 -> Byte 2
		 * Byte 3 -> Byte 1
		 * Byte ECC -> Byte 0
		 */
		dq_mapping_0 = pdimm->dq_mapping[0];
		dq_mapping_2 = pdimm->dq_mapping[2];
		dq_mapping_3 = pdimm->dq_mapping[3];
		pdimm->dq_mapping[0] = pdimm->dq_mapping[8];
		pdimm->dq_mapping[1] = pdimm->dq_mapping[9];
		pdimm->dq_mapping[2] = pdimm->dq_mapping[6];
		pdimm->dq_mapping[3] = pdimm->dq_mapping[7];
		pdimm->dq_mapping[6] = dq_mapping_2;
		pdimm->dq_mapping[7] = dq_mapping_3;
		pdimm->dq_mapping[8] = dq_mapping_0;
		for (i = 9; i < 18; i++)
			pdimm->dq_mapping[i] = 0;
		popts->cpo_sample = 0x5a;
	}

	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0x0;	/* 32 clocks */

	if (ddr_freq < 2350) {
		if (conf->cs_in_use == 0xf) {
			popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
					  DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
			popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm);
			popts->twot_en = 1;	/* enable 2T timing */
		} else {
			popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
					  DDR_CDR1_ODT(DDR_CDR_ODT_60ohm);
			popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_60ohm) |
					  DDR_CDR2_VREF_RANGE_2;
		}
	} else {
		popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
				  DDR_CDR1_ODT(DDR_CDR_ODT_100ohm);
		popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_100ohm) |
				  DDR_CDR2_VREF_RANGE_2;
	}

	return 0;
}

long long init_ddr(void)
{
	int spd_addr[] = {0x51, 0x52, 0x53, 0x54};
	int dpddr_spd_addr[] = {0x55};
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;
	long long dp_dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(struct ddr_info));
	/* Set two DDRC here. Unused DDRC will be removed automatically. */
	info.num_ctlrs = NUM_OF_DDRC;
	info.dimm_on_ctlr = DDRC_NUM_DIMM;
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	if (!info.clk) {
		info.clk = get_ddr_freq(&sys, 1);
	}

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
		    );

	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	zeromem(&info, sizeof(info));
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.spd_addr = dpddr_spd_addr;
	info.ddr[0] = (void *)NXP_DDR3_ADDR;
	info.clk = get_ddr_freq(&sys, 2);

	dp_dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
		    );
	if (dp_dram_size < 0) {
		debug("DPDDR init failed.\n");
	}

	return dram_size;
}
