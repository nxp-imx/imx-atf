#
# Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
# Copyright 2017 NXP
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

PLAT_INCLUDES		:=	-Iplat/freescale/imx8qm/include		\
				-Iplat/freescale/common/include		\

PLAT_GIC_SOURCES	:=	drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv3.c		\
				plat/freescale/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/freescale/common/lpuart_console.S		\
				plat/freescale/common/imx8_helpers.S		\
				plat/freescale/imx8qm/imx8qm_bl31_setup.c	\
				plat/freescale/imx8qm/imx8qm_psci.c		\
				plat/freescale/common/imx8_topology.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				drivers/console/aarch64/console.S		\
				drivers/arm/cci/cci.c				\
				${PLAT_GIC_SOURCES}				\

include plat/freescale/common/sci/sci_api.mk

ENABLE_PLAT_COMPAT	:=	0
USE_COHERENT_MEM	:=	0
RESET_TO_BL31		:=	1
ERROR_DEPRECATED	:=	1
A53_DISABLE_NON_TEMPORAL_HINT := 0
