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

#include <types.h>

#define MU_ATR0_OFFSET1		0x0
#define MU_ARR0_OFFSET1		0x10
#define MU_ASR_OFFSET1		0x20
#define MU_ACR_OFFSET1		0x24
#define MU_TR_COUNT1		4
#define MU_RR_COUNT1		4

#define MU_CR_GIEn_MASK1	(0xF << 28)
#define MU_CR_RIEn_MASK1	(0xF << 24)
#define MU_CR_TIEn_MASK1	(0xF << 20)
#define MU_CR_GIRn_MASK1	(0xF << 16)
#define MU_CR_NMI_MASK1		(1 << 3)
#define MU_CR_Fn_MASK1		0x7

#define MU_SR_TE0_MASK1		(1 << 23)
#define MU_SR_RF0_MASK1		(1 << 27)
#define MU_CR_RIE0_MASK1	(1 << 27)
#define MU_CR_GIE0_MASK1	(1 << 31)

#define MU_TR_COUNT			4
#define MU_RR_COUNT			4

void MU_Init(uint32_t base);
void MU_SendMessage(uint32_t base, uint32_t regIndex, uint32_t msg);
void MU_ReceiveMsg(uint32_t base, uint32_t regIndex, uint32_t *msg);
void MU_EnableGeneralInt(uint32_t base, uint32_t index);
void MU_EnableRxFullInt(uint32_t base, uint32_t index);
