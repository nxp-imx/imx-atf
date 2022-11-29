/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "trdc_config.h"

struct trdc_mgr_info trdc_mgr_blks[] = {
	{ TRDC_A_BASE, 0, 0, 39, 40 },
	{ TRDC_W_BASE, 0, 0, 70, 71 },
	{ TRDC_W_BASE, 1, 0, 1, 2 },
	{ TRDC_N_BASE, 0, 1, 1, 2 },
};

unsigned int trdc_mgr_num = ARRAY_SIZE(trdc_mgr_blks);

struct trdc_config_info trdc_cfg_info[] = {
	{	TRDC_A_BASE,
		trdc_a_mbc_glbac, ARRAY_SIZE(trdc_a_mbc_glbac),
		trdc_a_mbc, ARRAY_SIZE(trdc_a_mbc),
		trdc_a_mrc_glbac, ARRAY_SIZE(trdc_a_mrc_glbac),
		trdc_a_mrc, ARRAY_SIZE(trdc_a_mrc)
	}, /* TRDC_A */
	{	TRDC_W_BASE,
		trdc_w_mbc_glbac, ARRAY_SIZE(trdc_w_mbc_glbac),
		trdc_w_mbc, ARRAY_SIZE(trdc_w_mbc),
		trdc_w_mrc_glbac, ARRAY_SIZE(trdc_w_mrc_glbac),
		trdc_w_mrc, ARRAY_SIZE(trdc_w_mrc)
	}, /* TRDC_W */
	{	TRDC_N_BASE,
		trdc_n_mbc_glbac, ARRAY_SIZE(trdc_n_mbc_glbac),
		trdc_n_mbc, ARRAY_SIZE(trdc_n_mbc),
		trdc_n_mrc_glbac, ARRAY_SIZE(trdc_n_mrc_glbac),
		trdc_n_mrc, ARRAY_SIZE(trdc_n_mrc)
	}, /* TRDC_N */
};

struct trdc_fused_module_info fuse_info[] = {
	{ 0x49010000, 19, 13, 1, 2, 16, 1 }, /* NPU, NICMIX, MBC1, MEM2, slot 16 */
	{ 0x49010000, 19, 13, 1, 3, 16, 1 }, /* NPU, NICMIX, MBC1, MEM3, slot 16 */
	{ 0x44270000, 19, 30, 0, 0, 58, 1 }, /* FLEXCAN1, AONMIX, MBC0, MEM0, slot 58 */
	{ 0x42460000, 19, 31, 0, 0, 91, 1 }, /* FLEXCAN2, WAKEUPMIX, MBC0, MEM0, slot 91 */
	{ 0x49010000, 20, 4, 0, 3, 16, 16 }, /* USB1, NICMIX, MBC0, MEM3, slot 16-31 */
	{ 0x49010000, 20, 4, 0, 3, 32, 16 }, /* USB2, NICMIX, MBC0, MEM3, slot 32-47 */
	{ 0x42460000, 20, 5, 1, 0, 9, 1   }, /* ENET1 (FEC), WAKEUPMIX, MBC1, MEM0, slot 9 */
	{ 0x42460000, 20, 5, 1, 0, 10, 1  }, /* ENET2 (eQOS), WAKEUPMIX, MBC1, MEM0, slot 10 */
	{ 0x49010000, 20, 10, 0, 2, 34, 1 }, /* PXP, NICMIX, MBC0, MEM2, slot 34 */
	{ 0x49010000, 20, 17, 0, 2, 32, 1 }, /* MIPI CSI NICMIX, MBC0, MEM2, slot 32 */
	{ 0x49010000, 20, 19, 0, 2, 33, 1 }, /* MIPI DSI NICMIX, MBC0, MEM2, slot 33 */
	{ 0x44270000, 21, 7, 0, 0, 83, 1  }, /* ADC1 AONMIX, MBC0, MEM0, slot 83 */
};

void trdc_config(void)
{
	unsigned int i;

	/* Set MTR to DID1 */
	trdc_mda_set_noncpu(TRDC_A_BASE, 4, false, 0x2, 0x2, 0x1);

	/* Set M33 to DID2*/
	trdc_mda_set_cpu(TRDC_A_BASE, 1, 0, 0x2, 0x0, 0x2, 0x0, 0x0, 0x0);

	/* Configure the access permission for TRDC MGR and MC slots */
	for (i = 0U; i < ARRAY_SIZE(trdc_mgr_blks); i++) {
		trdc_mgr_mbc_setup(&trdc_mgr_blks[i]);
	}

	/* Configure TRDC user settings from config table */
	for (i = 0U; i < ARRAY_SIZE(trdc_cfg_info); i++) {
		trdc_setup(&trdc_cfg_info[i]);
	}

	/* Configure the access permission for fused slots */
	for (i = 0; i < ARRAY_SIZE(fuse_info); i++) {
		trdc_mgr_fused_slot_setup(&fuse_info[i]);
	}

	NOTICE("TRDC init done\n");
}

/*wakeup mix TRDC init */
void trdc_w_reinit(void)
{
	unsigned int i;

	/* config the access permission for the TRDC_W MGR and MC slot */
	trdc_mgr_mbc_setup(&trdc_mgr_blks[1]);

	trdc_mgr_mbc_setup(&trdc_mgr_blks[2]);

	/* config the TRDC user settting from the config table */
	trdc_setup(&trdc_cfg_info[1]);

	/* Configure the access permission for fused slots in wakeupmix TRDC */
	for (i = 0; i < ARRAY_SIZE(fuse_info); i++) {
		if (fuse_info[i].trdc_base == 0x42460000)
			trdc_mgr_fused_slot_setup(&fuse_info[i]);
	}
}

/*nic mix TRDC init */
void trdc_n_reinit(void)
{
	unsigned int i;

	/* config the access permission for the TRDC_N MGR and MC slot */
	trdc_mgr_mbc_setup(&trdc_mgr_blks[3]);

	/* config the TRDC user settting from the config table */
	trdc_setup(&trdc_cfg_info[2]);

	/* Configure the access permission for fused slots in nicmix TRDC */
	for (i = 0; i < ARRAY_SIZE(fuse_info); i++) {
		if (fuse_info[i].trdc_base == 0x49010000)
			trdc_mgr_fused_slot_setup(&fuse_info[i]);
	}
}
