/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "../scmi_private.h"
#include "scmi_imx9.h"

/*
 * API to set the SCMI AP core reset address and attributes
 */
int scmi_core_set_reset_addr(void *p, uint64_t reset_addr, uint32_t cpu_id, uint32_t attr)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_RESET_ADDR_SET_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_RESET_ADDR_SET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG4(mbx_mem->payload, cpu_id, attr, reset_addr & 0xffffffff,
		reset_addr >> 32);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_RESET_ADDR_SET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_core_start(void *p, uint32_t cpu_id)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_START_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_START_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, cpu_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_START_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_core_stop(void *p, uint32_t cpu_id)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_STOP_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_STOP_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, cpu_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_STOP_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_core_set_sleep_mode(void *p, uint32_t cpu_id, uint32_t wakeup, uint32_t mode)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_SETSLEEPMODE_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_SETSLEEPMODE_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, cpu_id, wakeup, mode);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_SETSLEEPMODE_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_core_Irq_wake_set(void *p, uint32_t cpu_id, uint32_t mask_idx,
		uint32_t num_mask, uint32_t *mask)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_SETIRQWAKESET_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_SETIRQWAKESET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG3(mbx_mem->payload, cpu_id, mask_idx, num_mask);

	for (int i = 0; i < num_mask; i++)
		mbx_mem->payload[3 + i] = mask[i];

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_SETIRQWAKESET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_core_lpm_mode_set(void *p, uint32_t cpu_id, uint32_t num_configs,
		struct scmi_lpm_config *cfg)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_CORE_PROTO_ID,
			IMX9_SCMI_CORE_LPMMODESET_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_LPMMODESET_MSG_LEN + (num_configs * sizeof(struct scmi_lpm_config));
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, cpu_id, num_configs);

	int j = 2;
	for (int i = 0; i < num_configs; i++){
		mmio_write_32((uintptr_t)&mbx_mem->payload[j++], cfg[i].power_domain);
		mmio_write_32((uintptr_t)&mbx_mem->payload[j++], cfg[i].lpmsetting);
		mmio_write_32((uintptr_t)&mbx_mem->payload[j++], cfg[i].retentionmask);
	}
	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_SETIRQWAKESET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

int scmi_perf_mode_set(void *p, uint32_t domain_id, uint32_t perf_level)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(IMX9_SCMI_PERF_PROTO_ID,
			IMX9_SCMI_CORE_PERFLEVELSET_MSG, token);
	mbx_mem->len = IMX9_SCMI_CORE_PERFLEVELSET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG2(mbx_mem->payload, domain_id, perf_level);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);
	assert(mbx_mem->len == IMX9_SCMI_CORE_PERFLEVELSET_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;

}

