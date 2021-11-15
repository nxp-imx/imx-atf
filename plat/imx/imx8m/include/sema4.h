/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

uint8_t sema4_trylock(uint8_t id);


void sema4_lock(uint8_t id);


void sema4_unlock(uint8_t id);


void sema4_init(void);


