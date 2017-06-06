/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMX8_PINS_H__
#define __IMX8_PINS_H__

#if PLAT_IMX8QM
#include "imx8qm_pads.h"
#elif PLAT_IMX8QXP
#include "imx8qxp_pads.h"
#else
#error "No pin header"
#endif
#endif	/* __IMX8_PINS_H__ */
