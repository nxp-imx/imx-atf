/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include "trdc_config.h"

#define DID_NUM 16
#define MBC_MAX_NUM 4
#define MRC_MAX_NUM 2
#define MBC_NUM(HWCFG) ((HWCFG >> 16) & 0xF)
#define MRC_NUM(HWCFG) ((HWCFG >> 24) & 0x1F)

#define MBC_BLK_NUM(GLBCFG) (GLBCFG & 0x3FF)
#define MRC_RGN_NUM(GLBCFG) (GLBCFG & 0x1F)

struct mbc_mem_dom {
	uint32_t mem_glbcfg[4];
	uint32_t nse_blk_index;
	uint32_t nse_blk_set;
	uint32_t nse_blk_clr;
	uint32_t nsr_blk_clr_all;
	uint32_t memn_glbac[8];
	/* The upper only existed in the beginning of each MBC */
	uint32_t mem0_blk_cfg_w[64];
	uint32_t mem0_blk_nse_w[16];
	uint32_t mem1_blk_cfg_w[8];
	uint32_t mem1_blk_nse_w[2];
	uint32_t mem2_blk_cfg_w[8];
	uint32_t mem2_blk_nse_w[2];
	uint32_t mem3_blk_cfg_w[8];
	uint32_t mem3_blk_nse_w[2]; /*0x1F0, 0x1F4 */
	uint32_t reserved[2];
};

struct mrc_rgn_dom {
	uint32_t mrc_glbcfg[4];
	uint32_t nse_rgn_indirect;
	uint32_t nse_rgn_set;
	uint32_t nse_rgn_clr;
	uint32_t nse_rgn_clr_all;
	uint32_t memn_glbac[8];
	/* The upper only existed in the beginning of each MRC */
	uint32_t rgn_desc_words[16][2]; /* 16 regions at max, 2 words per region */
	uint32_t rgn_nse;
	uint32_t reserved2[15];
};

struct mda_inst {
	uint32_t mda_w[8];
};

struct trdc_mgr {
	uint32_t trdc_cr;
	uint32_t res0[59];
	uint32_t trdc_hwcfg0;
	uint32_t trdc_hwcfg1;
	uint32_t res1[450];
	struct mda_inst mda[8];
	uint32_t res2[15808];
};

struct trdc_mbc {
	struct mbc_mem_dom mem_dom[DID_NUM];
};

struct trdc_mrc {
	struct mrc_rgn_dom mrc_dom[DID_NUM];
};

struct trdc_mgr_info {
	unsigned long trdc_base;
	uint8_t mbc_id;
	uint8_t mbc_mem_id;
	uint8_t blk_mgr;
	uint8_t blk_mc;
};

struct trdc_config_info {
	unsigned long trdc_base;
	struct trdc_glbac_config *mbc_glbac;
	uint32_t num_mbc_glbac;
	struct trdc_mbc_config *mbc_cfg;
	uint32_t num_mbc_cfg;
	struct trdc_glbac_config *mrc_glbac;
	uint32_t num_mrc_glbac;
	struct trdc_mrc_config *mrc_cfg;
	uint32_t num_mrc_cfg;
};

struct trdc_mgr_info trdc_mgr_blks[] = {
	{ 0x44270000, 0, 0, 39, 40 }, /* TRDC_A */
	{ 0x42460000, 0, 0, 70, 71 }, /* TRDC_W */
	{ 0x42460000, 1, 0, 1,  2  }, /* TRDC_M */
	{ 0x49010000, 0, 1, 1,  2  }, /* TRDC_N */
};

struct trdc_config_info trdc_cfg_info[] = {
	{ 	0x44270000,
		trdc_a_mbc_glbac, ARRAY_SIZE(trdc_a_mbc_glbac),
		trdc_a_mbc, ARRAY_SIZE(trdc_a_mbc),
		trdc_a_mrc_glbac, ARRAY_SIZE(trdc_a_mrc_glbac),
		trdc_a_mrc, ARRAY_SIZE(trdc_a_mrc)
	}, /* TRDC_A */
	{ 	0x42460000,
		trdc_w_mbc_glbac, ARRAY_SIZE(trdc_w_mbc_glbac),
		trdc_w_mbc, ARRAY_SIZE(trdc_w_mbc),
		trdc_w_mrc_glbac, ARRAY_SIZE(trdc_w_mrc_glbac),
		trdc_w_mrc, ARRAY_SIZE(trdc_w_mrc)
	}, /* TRDC_W */
	{ 	0x49010000,
		trdc_n_mbc_glbac, ARRAY_SIZE(trdc_n_mbc_glbac),
		trdc_n_mbc, ARRAY_SIZE(trdc_n_mbc),
		trdc_n_mrc_glbac, ARRAY_SIZE(trdc_n_mrc_glbac),
		trdc_n_mrc, ARRAY_SIZE(trdc_n_mrc)
	}, /* TRDC_N */
};

int trdc_mda_set_cpu(unsigned long trdc_reg, uint32_t mda_inst,
		 uint32_t mda_reg, uint8_t sa, uint8_t dids, uint8_t did,
		 uint8_t pe, uint8_t pidm, uint8_t pid)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t *mda_w = &trdc_base->mda[mda_inst].mda_w[mda_reg];
	uint32_t val = mmio_read_32((uintptr_t)mda_w);

	if (val & BIT(29)) /* non-cpu */
		return -EINVAL;

	val = BIT(31) | ((pid & 0x3f) << 16) | ((pidm & 0x3f) << 8) | ((pe & 0x3) << 6) |
		((sa & 0x3) << 14) | ((dids & 0x3) << 4) | (did & 0xf);

	mmio_write_32((uintptr_t)mda_w, val);

	return 0;
}

int trdc_mda_set_noncpu(unsigned long trdc_reg, uint32_t mda_inst,
		 uint32_t mda_reg, bool did_bypass, uint8_t sa,
		 uint8_t pa, uint8_t did)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t *mda_w = &trdc_base->mda[mda_inst].mda_w[mda_reg];
	uint32_t val = mmio_read_32((uintptr_t)mda_w);

	if (!(val & BIT(29))) /* cpu */
		return -EINVAL;

	val = BIT(31) | ((sa & 0x3) << 6) | ((pa & 0x3) << 4) | (did & 0xf);
	if (did_bypass)
		val |= BIT(8);

	mmio_write_32((uintptr_t)mda_w, val);

	return 0;
}

static unsigned long trdc_get_mbc_base(unsigned long trdc_reg, uint32_t mbc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);

	if (mbc_x >= mbc_num)
		return 0;

	return trdc_reg + 0x10000 + 0x2000 * mbc_x;
}

static unsigned long trdc_get_mrc_base(unsigned long trdc_reg, uint32_t mrc_x)
{
	struct trdc_mgr *trdc_base = (struct trdc_mgr *)trdc_reg;
	uint32_t mbc_num = MBC_NUM(trdc_base->trdc_hwcfg0);
	uint32_t mrc_num = MRC_NUM(trdc_base->trdc_hwcfg0);

	if (mrc_x >= mrc_num)
		return 0;

	return trdc_reg + 0x10000 + 0x2000 * mbc_num + 0x1000 * mrc_x;
}

uint32_t trdc_mbc_blk_num(unsigned long trdc_reg, uint32_t mbc_x, uint32_t mem_x)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;
	uint32_t glbcfg;

	if (mbc_base == 0)
		return 0;

	/* only first dom has the glbcfg */
	mbc_dom = &mbc_base->mem_dom[0];
	glbcfg = mmio_read_32((uintptr_t)&mbc_dom->mem_glbcfg[mem_x]);

	return MBC_BLK_NUM(glbcfg);
}

uint32_t trdc_mrc_rgn_num(unsigned long trdc_reg, uint32_t mrc_x)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;
	uint32_t glbcfg;

	if (mrc_base == 0)
		return 0;

	/* only first dom has the glbcfg */
	mrc_dom = &mrc_base->mrc_dom[0];
	glbcfg = mmio_read_32((uintptr_t)&mrc_dom->mrc_glbcfg[0]);

	return MBC_BLK_NUM(glbcfg);
}

int trdc_mbc_set_control(unsigned long trdc_reg, uint32_t mbc_x, uint32_t glbac_id, uint32_t glbac_val)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mbc_dom = &mbc_base->mem_dom[0];

	mmio_write_32((uintptr_t)&mbc_dom->memn_glbac[glbac_id], glbac_val);

	return 0;
}

int trdc_mbc_blk_config(unsigned long trdc_reg, uint32_t mbc_x,
	 uint32_t dom_x, uint32_t mem_x, uint32_t blk_x,
	 bool sec_access, uint32_t glbac_id)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;
	uint32_t *cfg_w;
	uint32_t index, offset, val;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	mbc_dom = &mbc_base->mem_dom[dom_x];

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[blk_x / 8];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[blk_x / 8];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[blk_x / 8];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[blk_x / 8];
		break;
	default:
		return -1;
	};

	index = blk_x % 8;
	offset = index * 4;

	val = mmio_read_32((uintptr_t)cfg_w);

	val &= ~(0xFU << offset);

	/*
	 * MBC0-3
	 * Global 0, 0x7777 secure pri/user read/write/execute, S400 has already set it.
	 * So select MBC0_MEMN_GLBAC0
	 */
	if (sec_access) {
		val |= ((0x0 | (glbac_id & 0x7)) << offset);
		mmio_write_32((uintptr_t)cfg_w, val);
	} else {
		val |= ((0x8 | (glbac_id & 0x7)) << offset); /* nse bit set */
		mmio_write_32((uintptr_t)cfg_w, val);
	}

	return 0;
}

int trdc_mrc_set_control(unsigned long trdc_reg, uint32_t mrc_x,
	uint32_t glbac_id, uint32_t glbac_val)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;

	if (mrc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mrc_dom = &mrc_base->mrc_dom[0];

	mmio_write_32((uintptr_t)&mrc_dom->memn_glbac[glbac_id], glbac_val);

	return 0;
}

int trdc_mrc_rgn_config(unsigned long trdc_reg, uint32_t mrc_x, uint32_t dom_x, uint32_t rgn_id,
	uint32_t addr_start, uint32_t addr_size, bool sec_access, uint32_t glbac_id)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;
	uint32_t *desc_w;
	uint32_t addr_end;

	if (mrc_base == 0 || glbac_id >= 8 || rgn_id >= 16)
		return -EINVAL;

	mrc_dom = &mrc_base->mrc_dom[dom_x];

	addr_end = addr_start + addr_size - 1;
	addr_start &= ~0x3fff;
	addr_end &= ~0x3fff;

	desc_w = &mrc_dom->rgn_desc_words[rgn_id][0];

	if (sec_access) {
		mmio_write_32((uintptr_t)desc_w, addr_start | (glbac_id & 0x7));
		mmio_write_32((uintptr_t)(desc_w + 1), addr_end | 0x1);
	} else {
		mmio_write_32((uintptr_t)desc_w, addr_start | (glbac_id & 0x7));
		mmio_write_32((uintptr_t)(desc_w + 1), (addr_end | 0x1 | 0x10));
	}

	return 0;
}

bool trdc_mrc_enabled(unsigned long trdc_base)
{
	return (!!(mmio_read_32(trdc_base) & 0x8000));
}

bool trdc_mbc_enabled(unsigned long trdc_base)
{
	return (!!(mmio_read_32(trdc_base) & 0x4000));
}

#if DEBUG
int trdc_mbc_control_dump(unsigned long trdc_reg, uint32_t mbc_x, uint32_t glbac_id)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;

	if (mbc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mbc_dom = &mbc_base->mem_dom[0];

	NOTICE("mbc_dom %u glbac %u: 0x%x\n", mbc_x, glbac_id,
		mmio_read_32((uintptr_t)&mbc_dom->memn_glbac[glbac_id]));

	return 0;
}

int trdc_mbc_mem_dump(unsigned long trdc_reg, uint32_t mbc_x, uint32_t dom_x, uint32_t mem_x, uint32_t word)
{
	struct trdc_mbc *mbc_base = (struct trdc_mbc *)trdc_get_mbc_base(trdc_reg, mbc_x);
	struct mbc_mem_dom *mbc_dom;
	uint32_t *cfg_w;

	if (mbc_base == 0)
		return -EINVAL;

	mbc_dom = &mbc_base->mem_dom[dom_x];

	switch (mem_x) {
	case 0:
		cfg_w = &mbc_dom->mem0_blk_cfg_w[word];
		break;
	case 1:
		cfg_w = &mbc_dom->mem1_blk_cfg_w[word];
		break;
	case 2:
		cfg_w = &mbc_dom->mem2_blk_cfg_w[word];
		break;
	case 3:
		cfg_w = &mbc_dom->mem3_blk_cfg_w[word];
		break;
	default:
		return -1;
	};

	NOTICE("mbc_dom %u dom %u mem %u word %u: 0x%x\n", mbc_x, dom_x, mem_x, word,
		mmio_read_32((uintptr_t)cfg_w));

	return 0;
}

int trdc_mrc_control_dump(unsigned long trdc_reg, uint32_t mrc_x, uint32_t glbac_id)
{
	struct trdc_mrc *mrc_base = (struct trdc_mrc *)trdc_get_mrc_base(trdc_reg, mrc_x);
	struct mrc_rgn_dom *mrc_dom;

	if (mrc_base == 0 || glbac_id >= 8)
		return -EINVAL;

	/* only first dom has the glbac */
	mrc_dom = &mrc_base->mrc_dom[0];

	NOTICE("mrc_dom %u glbac %u: 0x%x\n", mrc_x, glbac_id,
		mmio_read_32((uintptr_t)&mrc_dom->memn_glbac[glbac_id]));

	return 0;
}

void trdc_dump(void)
{
	uint32_t i;
	NOTICE("TRDC AONMIX MBC\n");

	trdc_mbc_control_dump(0x44270000, 0, 0);
	trdc_mbc_control_dump(0x44270000, 1, 0);

	for (i = 0; i < 11; i++) {
		trdc_mbc_mem_dump(0x44270000, 0, 3, 0, i);
	}
	for (i = 0; i < 1; i++) {
		trdc_mbc_mem_dump(0x44270000, 0, 3, 1, i);
	}

	for (i = 0; i < 4; i++) {
		trdc_mbc_mem_dump(0x44270000, 1, 3, 0, i);
	}
	for (i = 0; i < 4; i++) {
		trdc_mbc_mem_dump(0x44270000, 1, 3, 1, i);
	}

	NOTICE("TRDC WAKEUP MBC\n");

	trdc_mbc_control_dump(0x42460000, 0, 0);
	trdc_mbc_control_dump(0x42460000, 1, 0);

	for (i = 0; i < 15; i++) {
		trdc_mbc_mem_dump(0x42460000, 0, 3, 0, i);
	}
	trdc_mbc_mem_dump(0x42460000, 0, 3, 1, 0);
	trdc_mbc_mem_dump(0x42460000, 0, 3, 2, 0);

	for (i = 0; i < 2; i++) {
		trdc_mbc_mem_dump(0x42460000, 1, 3, 0, i);
	}
	trdc_mbc_mem_dump(0x42460000, 1, 3, 1, 0);
	trdc_mbc_mem_dump(0x42460000, 1, 3, 2, 0);
	trdc_mbc_mem_dump(0x42460000, 1, 3, 3, 0);

	NOTICE("TRDC NICMIX MBC\n");

	trdc_mbc_control_dump(0x49010000, 0, 0);
	trdc_mbc_control_dump(0x49010000, 1, 0);
	trdc_mbc_control_dump(0x49010000, 2, 0);
	trdc_mbc_control_dump(0x49010000, 3, 0);

	for (i = 0; i < 7; i++) {
		trdc_mbc_mem_dump(0x49010000, 0, 3, 0, i);
	}

	for (i = 0; i < 2; i++) {
		trdc_mbc_mem_dump(0x49010000, 0, 3, 1, i);
	}

	for (i = 0; i < 5; i++) {
		trdc_mbc_mem_dump(0x49010000, 0, 3, 2, i);
	}

	for (i = 0; i < 6; i++) {
		trdc_mbc_mem_dump(0x49010000, 0, 3, 3, i);
	}

	for (i = 0; i < 1; i++) {
		trdc_mbc_mem_dump(0x49010000, 1, 3, 0, i);
	}

	for (i = 0; i < 1; i++) {
		trdc_mbc_mem_dump(0x49010000, 1, 3, 1, i);
	}

	for (i = 0; i < 3; i++) {
		trdc_mbc_mem_dump(0x49010000, 1, 3, 2, i);
	}

	for (i = 0; i < 3; i++) {
		trdc_mbc_mem_dump(0x49010000, 1, 3, 3, i);
	}

	for (i = 0; i < 2; i++) {
		trdc_mbc_mem_dump(0x49010000, 2, 3, 0, i);
	}

	for (i = 0; i < 2; i++) {
		trdc_mbc_mem_dump(0x49010000, 2, 3, 1, i);
	}

	for (i = 0; i < 5; i++) {
		trdc_mbc_mem_dump(0x49010000, 3, 3, 0, i);
	}

	for (i = 0; i < 5; i++) {
		trdc_mbc_mem_dump(0x49010000, 3, 3, 1, i);
	}
}
#endif

static bool is_trdc_mgr_slot(unsigned long trdc_base, uint8_t mbc_id, uint8_t mem_id, uint16_t blk_id)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(trdc_mgr_blks); i++) {
		if (trdc_mgr_blks[i].trdc_base == trdc_base) {
			if (mbc_id == trdc_mgr_blks[i].mbc_id
				&& mem_id == trdc_mgr_blks[i].mbc_mem_id
				&& (blk_id == trdc_mgr_blks[i].blk_mgr
				|| blk_id == trdc_mgr_blks[i].blk_mc))
				return true;
		}
	}

	return false;
}

static void trdc_mgr_mbc_setup(struct trdc_mgr_info *mgr)
{
	uint32_t i;

	if (trdc_mbc_enabled(mgr->trdc_base)) {
		trdc_mbc_set_control(mgr->trdc_base, mgr->mbc_id, 7, 0x6000); /* ONLY secure privilige can access */
		for (i = 0; i < 16; i++) {
			trdc_mbc_blk_config(mgr->trdc_base, mgr->mbc_id, i, mgr->mbc_mem_id, mgr->blk_mgr, true, 7);
			trdc_mbc_blk_config(mgr->trdc_base, mgr->mbc_id, i, mgr->mbc_mem_id, mgr->blk_mc, true, 7);
		}
	}
}

static void trdc_setup(struct trdc_config_info *cfg)
{
	uint32_t i, j, num;
	bool is_mgr;

	if (trdc_mrc_enabled(cfg->trdc_base)) {
		for (i = 0; i < cfg->num_mrc_glbac; i++) {
			trdc_mrc_set_control(cfg->trdc_base, cfg->mrc_glbac[i].mbc_mrc_id,
				cfg->mrc_glbac[i].glbac_id, cfg->mrc_glbac[i].glbac_val);
		}

		for (i = 0; i < cfg->num_mrc_cfg; i++) {
			trdc_mrc_rgn_config(cfg->trdc_base, cfg->mrc_cfg[i].mrc_id, cfg->mrc_cfg[i].dom_id,
				cfg->mrc_cfg[i].region_id, cfg->mrc_cfg[i].region_start, cfg->mrc_cfg[i].region_size,
				cfg->mrc_cfg[i].secure, cfg->mrc_cfg[i].glbac_id);
		}
	}

	if (trdc_mbc_enabled(cfg->trdc_base)) {
		for (i = 0; i < cfg->num_mbc_glbac; i++) {
			trdc_mbc_set_control(cfg->trdc_base, cfg->mbc_glbac[i].mbc_mrc_id,
				cfg->mbc_glbac[i].glbac_id, cfg->mbc_glbac[i].glbac_val);
		}

		for (i = 0; i < cfg->num_mbc_cfg; i++) {
			if (cfg->mbc_cfg[i].blk_id == MBC_BLK_ALL) {
				num = trdc_mbc_blk_num(cfg->trdc_base, cfg->mbc_cfg[i].mbc_id, cfg->mbc_cfg[i].mem_id);
				for (j = 0; j < num; j++) {
					/* Skip mgr and mc */
					is_mgr = is_trdc_mgr_slot(cfg->trdc_base, cfg->mbc_cfg[i].mbc_id, cfg->mbc_cfg[i].mem_id, j);
					if (is_mgr)
						continue;

					trdc_mbc_blk_config(cfg->trdc_base, cfg->mbc_cfg[i].mbc_id, cfg->mbc_cfg[i].dom_id,
						cfg->mbc_cfg[i].mem_id, j, cfg->mbc_cfg[i].secure,
						cfg->mbc_cfg[i].glbac_id);
				}
			} else {
				trdc_mbc_blk_config(cfg->trdc_base, cfg->mbc_cfg[i].mbc_id, cfg->mbc_cfg[i].dom_id,
						cfg->mbc_cfg[i].mem_id, cfg->mbc_cfg[i].blk_id, cfg->mbc_cfg[i].secure,
						cfg->mbc_cfg[i].glbac_id);
			}
		}
	}
}

void trdc_config(void)
{
	int i;

	/* Set MTR to DID1 */
	trdc_mda_set_noncpu(0x44270000, 4, 0, false, 0x2, 0x2, 0x1);

	/* Set M33 to DID2*/
	trdc_mda_set_cpu(0x44270000, 1, 0, 0x2, 0x0, 0x2, 0x0, 0x0, 0x0);

	/* Configure the access permission for TRDC MGR and MC slots */
	for (i = 0; i < ARRAY_SIZE(trdc_mgr_blks); i++) {
		trdc_mgr_mbc_setup(&trdc_mgr_blks[i]);
	}

	/* Configure TRDC user settings from config table */
	for (i = 0; i < ARRAY_SIZE(trdc_cfg_info); i++) {
		trdc_setup(&trdc_cfg_info[i]);
	}
}

/*wakeup mix TRDC init */
void trdc_w_reinit(void)
{
	/* config the access permission for the TRDC_W MGR and MC slot */
	trdc_mgr_mbc_setup(&trdc_mgr_blks[1]);

	trdc_mgr_mbc_setup(&trdc_mgr_blks[2]);

	/* config the TRDC user settting from the config table */
	trdc_setup(&trdc_cfg_info[1]);
}

/*nic mix TRDC init */
void trdc_n_reinit(void)
{
	/* config the access permission for the TRDC_N MGR and MC slot */
	trdc_mgr_mbc_setup(&trdc_mgr_blks[3]);

	/* config the TRDC user settting from the config table */
	trdc_setup(&trdc_cfg_info[2]);
}
