/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <lib/mmio.h>
#include <lib/bakery_lock.h>

#include <platform_def.h>

#include <drivers/scmi.h>

#include <drivers/arm/css/scmi.h>


void *imx95_scmi_handle;

/* The SCMI channel global object */
static scmi_channel_t channel;

/* TODO: ?? */
//DEFINE_BAKERY_LOCK(imx95_scmi_lock);
spinlock_t imx95_scmi_lock;
#define IMX95_SCMI_LOCK_GET_INSTANCE	(&imx95_scmi_lock)

static void mu_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	uint32_t db = mmio_read_32(plat_info->db_reg_addr) & (plat_info->db_preserve_mask);
	mmio_write_32(plat_info->db_reg_addr, db | plat_info->db_modify_mask);

	return;
}

static scmi_channel_plat_info_t sq_scmi_plat_info = {
		.scmi_mbx_mem = IMX95_SCMI_PAYLOAD_BASE,
		.db_reg_addr = IMX95_MU1_BASE + MU_GCR_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mu_ring_doorbell,
};

#define SCMI_CORE_PROTO_ID			0x82

static int scmi_ap_core_init(scmi_channel_t *ch)
{
	uint32_t version;
	int ret;

	ret = scmi_proto_version(ch, SCMI_CORE_PROTO_ID, &version);
	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI AP core protocol version message failed\n");
		return -1;
	}

	INFO("SCMI AP core protocol version 0x%x detected\n", version);

	return 0;
}

void plat_imx95_setup(void)
{
	channel.info = &sq_scmi_plat_info;
	channel.lock = IMX95_SCMI_LOCK_GET_INSTANCE;
	imx95_scmi_handle = scmi_init(&channel);
	if (imx95_scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
	if (scmi_ap_core_init(&channel) < 0) {
		ERROR("SCMI AP core protocol initialization failed\n");
		panic();
	}
}
