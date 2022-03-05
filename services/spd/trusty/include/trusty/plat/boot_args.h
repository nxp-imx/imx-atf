/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

/* ep_info_exp.h requires stdint.h, but doesn't include it itself */
#include <stdint.h>
#include <export/common/ep_info_exp.h>

/**
 * plat_trusty_set_boot_args - Provide custom boot parameters to Trusty.
 * @args:   Boot arguments to provide to Trusty. args->arg0 must be set
 *          to the memory size allocated to Trusty. args->arg1 can be
 *          set to a platform-specific parameter block, with args->arg2
 *          indicating the size of that parameter block.
 *          If TSP_SEC_MEM_SIZE or BL32_MEM_SIZE is defined, a default
 *          implementation that passes that size in args->arg0 will be provided.
 */
void plat_trusty_set_boot_args(aapcs64_params_t *args);
