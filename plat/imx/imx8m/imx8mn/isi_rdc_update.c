/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <imx_rdc.h>
#include <common/debug.h>
#include <imx8m_ccm.h>

/* The ISI buffers are expected to be less than or equal to 5 */
#define ISI_MAX_REGIONS (5)
#define ENABLE_ISI_BUFFER_ISOLATION (1)
#define DISABLE_ISI_BUFFER_ISOLATION (0)
#define RDC_DRAM_START_REGION (1)

struct isi_region {
	unsigned long start;
	unsigned long end;
};

static struct isi_region regions[ISI_MAX_REGIONS] = {0};
static int isi_region_num = 0;

/* Commit the ISI buffers into an array */
int isi_commit_mem_region(u_register_t x1, u_register_t x2) {
	int i = 0;

	if (x1 == 0 || x2 == 0 || x1 >= x2) {
		NOTICE("Invalid start and end address!\n");
		return 1;
	}

	if (x1 < IMX_DRAM_BASE ||
	    x2 > (IMX_DRAM_BASE + IMX_DRAM_SIZE - 1)) {
		NOTICE("ISI region out of DRAM range!\n");
		return 1;
	}

	if (isi_region_num >= ISI_MAX_REGIONS) {
		NOTICE("ISI region table is full!\n");
		return 1;
	}

	/* The caller should make sure no overlap, here we do some sanity check */
	for (i = 0; i < isi_region_num; i++) {
		if (regions[i].start <= x1 && regions[i].end >= x2) {
			return 0;
		} else if (regions[i].start >= x1 && regions[i].end <= x2) {
			regions[i].start = x1;
			regions[i].end = x2;
			return 0;
		}
	}

	/* Check if we can merge memory regions */
	for (i = 0; i < isi_region_num; i++) {
		if (regions[i].end >= x1 && regions[i].end <= x2) {
			regions[i].end = x2;
			return 0;
		} else if (regions[i].start <= x2 && regions[i].end >= x1) {
			regions[i].start = x1;
			return 0;
		}
	}

	/* A new region */
	regions[isi_region_num].start = x1;
	regions[isi_region_num].end  = x2;
	isi_region_num++;

	return 0;
}

/* Sort the regions in ascending order */
void isi_sort_regions() {
	struct isi_region tmp_region;
	int i = 0, j = 0;;

	for (i = 0; i < isi_region_num; i++) {
		for (j = 0; j < isi_region_num - i - 1; j++) {
			/* We assume no overlap between these memory regions now */
			if (regions[j].start > regions[j + 1].start) {
				tmp_region.start = regions[j].start;
				tmp_region.end  = regions[j].end;
				regions[j].start = regions[j + 1].start;
				regions[j].end  = regions[j + 1].end;
				regions[j + 1].start = tmp_region.start;
				regions[j + 1].end = tmp_region.end;
			}
		}
	}

}

#define ISI_RDC_MEM_REGIONn(rdc, i, msa, mea, mrc)	\
	{						\
		rdc.type = RDC_MEM_REGION;		\
		rdc.index = i;				\
		rdc.setting.rdc_mem_region[0] = (msa);	\
		rdc.setting.rdc_mem_region[1] = (mea);	\
		rdc.setting.rdc_mem_region[2] = (mrc);	\
	}

/* Sort the committed buffers and setup the rdc policy */
int isi_setup_rdc_policy(u_register_t x1) {
	unsigned long rdc_start = 0;
	unsigned long rdc_end = 0;
	struct imx_rdc_cfg rdc_cfg[ISI_MAX_REGIONS + 2] = {0};
	int i = 0, rdc_index = 0;

	if (x1 == ENABLE_ISI_BUFFER_ISOLATION) {
		/* Setup the rdc policy */

		/* Avoid regions[0]/regions[num-1] underflow on empty table */
		if (isi_region_num == 0) {
			NOTICE("No ISI region committed!\n");
			return 1;
		}

		/* Sort the memory regions which should already be committed. */
		isi_sort_regions();

		/* Construct the rdc policy. The first one and the last
		 * one region are special.
		 */
		rdc_start = 0;
		rdc_end = (regions[0].start - 0x40000000) / 2;
		ISI_RDC_MEM_REGIONn(rdc_cfg[rdc_index], RDC_DRAM_START_REGION, rdc_start,
					rdc_end, ENA | D0R | D0W | D1R | D1W);
		rdc_index++;
		for (i = 0; i < isi_region_num - 1; i++) {
			rdc_start = (regions[i].end - 0x40000000) / 2;
			rdc_end = (regions[i + 1].start - 0x40000000) / 2;
			/* Make sure no empty rdc configure */
			if (rdc_start != rdc_end) {
				ISI_RDC_MEM_REGIONn(rdc_cfg[rdc_index], RDC_DRAM_START_REGION + rdc_index,
							rdc_start, rdc_end, ENA | D0R | D0W | D1R | D1W);
				rdc_index++;
			}
		}
		rdc_start = (regions[isi_region_num - 1].end - 0x40000000) / 2;
		rdc_end = 0xFFFFFFFF;
		ISI_RDC_MEM_REGIONn(rdc_cfg[rdc_index], RDC_DRAM_START_REGION + rdc_index,
					rdc_start, rdc_end, ENA | D0R | D0W | D1R | D1W);
	} else if (x1 == DISABLE_ISI_BUFFER_ISOLATION){
		/* Disable and clear the policy */
		for (i = 0; i < ISI_MAX_REGIONS + 1; i++) {
			ISI_RDC_MEM_REGIONn(rdc_cfg[i], RDC_DRAM_START_REGION + i, 0x0, 0x0, 0xff);
		}
		memset(regions, 0, sizeof(regions));
		isi_region_num = 0;
	} else {
		NOTICE("Invalid command!\n");
		return 1;
	}

	imx_rdc_init(rdc_cfg, 0);

	return 0;
}
