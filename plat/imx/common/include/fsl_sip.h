/*
 * Copyright 2017 NXP
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SOC_FSL_SIP_H
#define __SOC_FSL_SIP_H

#define FSL_SIP_GPC			0xC2000000
#define FSL_SIP_CONFIG_GPC_MASK		0x00
#define FSL_SIP_CONFIG_GPC_UNMASK	0x01
#define FSL_SIP_CONFIG_GPC_SET_WAKE	0x02
#define FSL_SIP_CONFIG_GPC_PM_DOMAIN	0x03

#define FSL_SIP_CPUFREQ			0xC2000001
#define FSL_SIP_SET_CPUFREQ		0x00

#define FSL_SIP_SRTC			0xC2000002
#define FSL_SIP_SRTC_SET_TIME		0x00

#endif
