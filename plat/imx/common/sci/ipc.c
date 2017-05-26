/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sci/scfw.h>
#include <sci/ipc.h>
#include <sci/rpc.h>
#include <stdlib.h>

#include "mx8_mu.h"

void sc_call_rpc(sc_ipc_t ipc, sc_rpc_msg_t *msg, bool no_resp)
{
	sc_ipc_write(ipc, msg);
	if (!no_resp)
		sc_ipc_read(ipc, msg);
}

sc_err_t sc_ipc_open(sc_ipc_t *ipc, sc_ipc_id_t id)
{
	uint32_t base = id;
	uint32_t i;
	
	/* Get MU base associated with IPC channel */
	if ((ipc == NULL) || (base == 0))
		return SC_ERR_IPC;

	/* Init MU */
	MU_Init(base);

	/* Enable all RX interrupts */
	for (i = 0; i < MU_RR_COUNT; i++) {
		MU_EnableRxFullInt(base, i);
	}

	/* Return MU address as handle */
	*ipc = (sc_ipc_t) id;

	return SC_ERR_NONE;
}

void sc_ipc_close(sc_ipc_t ipc)
{
	uint32_t base = ipc;

	if (base != 0)
		MU_Init(base);
}

void sc_ipc_read(sc_ipc_t ipc, void *data)
{
	uint32_t base = ipc;
	sc_rpc_msg_t *msg = (sc_rpc_msg_t*) data;
	uint8_t count = 0;

	/* Check parms */
	if ((base == 0) || (msg == NULL))
		return;

	/* Read first word */
	MU_ReceiveMsg(base, 0, (uint32_t*) msg);   
	count++;

	/* Check size */
	if (msg->size > SC_RPC_MAX_MSG) {
		*((uint32_t*) msg) = 0;
		return;
	}
	
	/* Read remaining words */
	while (count < msg->size) {
		MU_ReceiveMsg(base, count % MU_RR_COUNT,
			&(msg->DATA.u32[count - 1]));   
		count++;
	}
}

void sc_ipc_write(sc_ipc_t ipc, void *data)
{
	sc_rpc_msg_t *msg = (sc_rpc_msg_t*) data;
	uint32_t base = ipc;
	uint8_t count = 0;

	/* Check parms */
	if ((base == 0) || (msg == NULL))
		return;

	/* Check size */
	if (msg->size > SC_RPC_MAX_MSG)
		return;

	/* Write first word */
	MU_SendMessage(base, 0, *((uint32_t*) msg));   
	count++;
	
	/* Write remaining words */
	while (count < msg->size) {
		MU_SendMessage(base, count % MU_TR_COUNT,
			msg->DATA.u32[count - 1]);   
		count++;
	}
}

