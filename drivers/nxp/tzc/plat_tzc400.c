/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>

#include <plat_tzc400.h>

#pragma weak populate_tzc400_reg_list

#ifdef DEFAULT_TZASC_CONFIG
/*
 * Typical Memory map of DRAM0
 *    |-----------NXP_NS_DRAM_ADDR ( = NXP_DRAM0_ADDR)----------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |								|
 *    |------- (NXP_NS_DRAM_ADDR + NXP_NS_DRAM_SIZE - 1) -------|
 *    |-----------------NXP_SECURE_DRAM_ADDR--------------------|
 *    |								|
 *    |								|
 *    |								|
 *    |			SECURE REGION (= 64MB)			|
 *    |								|
 *    |								|
 *    |								|
 *    |--- (NXP_SECURE_DRAM_ADDR + NXP_SECURE_DRAM_SIZE - 1)----|
 *    |-----------------NXP_SP_SHRD_DRAM_ADDR-------------------|
 *    |								|
 *    |	       Secure EL1 Payload SHARED REGION (= 2MB)         |
 *    |								|
 *    |-----------(NXP_DRAM0_ADDR + NXP_DRAM0_SIZE - 1)---------|
 *
 *
 *
 * Typical Memory map of DRAM1
 *    |---------------------NXP_DRAM1_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM1_ADDR + Dynamically calculated Size - 1) ---|
 *
 *
 * Typical Memory map of DRAM2
 *    |---------------------NXP_DRAM2_ADDR----------------------|
 *    |								|
 *    |								|
 *    |			Non-SECURE REGION			|
 *    |								|
 *    |								|
 *    |---(NXP_DRAM2_ADDR + Dynamically calculated Size - 1) ---|
 */

/*****************************************************************************
 * This function sets up access permissions on memory regions
 ****************************************************************************/
int populate_tzc400_reg_list(struct tzc400_reg *tzc400_reg_list,
					int dram_idx, int list_idx,
					uint64_t dram_start_addr,
					uint64_t dram_size,
					uint32_t secure_dram_sz,
					uint32_t shrd_dram_sz)
{
	/* TZC Region 0 Configuration*/
	if (list_idx == 0) {
		tzc400_reg_list[list_idx].reg_filter_en = 1;
		tzc400_reg_list[list_idx].start_addr = 0x00000000;
		tzc400_reg_list[list_idx].end_addr = 0xFFFFFFFF;
		tzc400_reg_list[list_idx].sec_attr = TZC_REGION_S_NONE;
		tzc400_reg_list[list_idx].nsaid_permissions = TZC_REGION_NS_NONE;
		list_idx++;
	}
	if (dram_idx == 0) {
		/* TZC Region 1 on DRAM0 for Secure Memory*/
		tzc400_reg_list[list_idx].reg_filter_en = 1;
		tzc400_reg_list[list_idx].start_addr = dram_start_addr + dram_size;
		tzc400_reg_list[list_idx].end_addr = dram_start_addr + dram_size
						+ secure_dram_sz - 1;
		tzc400_reg_list[list_idx].sec_attr = TZC_REGION_S_RDWR;
		tzc400_reg_list[list_idx].nsaid_permissions = TZC_REGION_NS_NONE;
		list_idx++;

		/* TZC Region 2 on DRAM0 for Shared Memory*/
		tzc400_reg_list[list_idx].reg_filter_en = 1;
		tzc400_reg_list[list_idx].start_addr = dram_start_addr + dram_size
							+ secure_dram_sz;
		tzc400_reg_list[list_idx].end_addr = dram_start_addr + dram_size
							+ secure_dram_sz
							+ shrd_dram_sz
							- 1;
		tzc400_reg_list[list_idx].sec_attr = TZC_REGION_S_RDWR;
		tzc400_reg_list[list_idx].nsaid_permissions = TZC_NS_ACCESS_ID;
		list_idx++;

		/* TZC Region 3 on DRAM0 for Non-Secure Memory*/
		tzc400_reg_list[list_idx].reg_filter_en = 1;
		tzc400_reg_list[list_idx].start_addr = dram_start_addr;
		tzc400_reg_list[list_idx].end_addr = dram_start_addr + dram_size
							- 1;
		tzc400_reg_list[list_idx].sec_attr = TZC_REGION_S_RDWR;
		tzc400_reg_list[list_idx].nsaid_permissions = TZC_NS_ACCESS_ID;
		list_idx++;
	} else {
		/* TZC Region 3+i on DRAM(> 0) for Non-Secure Memory*/
		tzc400_reg_list[list_idx].reg_filter_en = 1;
		tzc400_reg_list[list_idx].start_addr = dram_start_addr;
		tzc400_reg_list[list_idx].end_addr = dram_start_addr + dram_size
							- 1;
		tzc400_reg_list[list_idx].sec_attr = TZC_REGION_S_RDWR;
		tzc400_reg_list[list_idx].nsaid_permissions = TZC_NS_ACCESS_ID;
		list_idx++;
	}

	return list_idx;
}
#else
int populate_tzc400_reg_list(struct tzc400_reg *tzc400_reg_list,
					int dram_idx, int list_idx,
					uint64_t dram_start_addr,
					uint64_t dram_size,
					uint32_t secure_dram_sz,
					uint32_t shrd_dram_sz)
{
	ERROR("tzc400_reg_list used is not a default list\n");
	ERROR("%s needs to be over-written.\n", __func__);
}
#endif	/* DEFAULT_TZASC_CONFIG */

/*******************************************************************************
 * Configure memory access permissions
 *   - Region 0 with no access;
 *   - Region 1 to 4 with ?????
 ******************************************************************************/
void mem_access_setup(uintptr_t base, uint32_t total_regions,
			struct tzc400_reg *tzc400_reg_list)
{
	uint32_t list_indx = 0;

	INFO("Configuring TrustZone Controller\n");

	tzc400_init(base);

	/* Disable filters. */
	tzc400_disable_filters();

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	for (list_indx = 1; list_indx < total_regions; list_indx++) {
		tzc400_configure_region(
			tzc400_reg_list[list_indx].reg_filter_en,
			list_indx,
			tzc400_reg_list[list_indx].start_addr,
			tzc400_reg_list[list_indx].end_addr,
			tzc400_reg_list[list_indx].sec_attr,
			tzc400_reg_list[list_indx].nsaid_permissions);
	}

	/*
	 * Raise an exception if a NS device tries to access secure memory
	 * TODO: Add interrupt handling support.
	 */
	tzc400_set_action(TZC_ACTION_ERR);

	/* Enable filters. */
	tzc400_enable_filters();
}
