#
# Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		:=	-Iplat/imx/imx8qx/include		\
				-Iplat/imx/common/include
ifeq (${IMX_ANDROID_BUILD},true)
PLAT_INCLUDES           +=	-Iinclude/drivers/nxp/crypto/caam	\
				-Iinclude/drivers/nxp/timer
endif

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

IMX_GIC_SOURCES	:=		${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/lpuart_console.S	\
				plat/imx/common/imx8_helpers.S		\
				plat/imx/imx8qx/imx8qx_bl31_setup.c	\
				plat/imx/imx8qx/imx8qx_psci.c		\
				plat/imx/common/imx8_topology.c		\
				plat/imx/common/imx8_psci.c		\
				plat/imx/common/imx_sip_svc.c		\
				plat/imx/common/imx_sip_handler.c	\
				plat/common/plat_psci_common.c		\
				lib/cpus/aarch64/cortex_a35.S		\
				${XLAT_TABLES_LIB_SRCS}			\
				${IMX_GIC_SOURCES}
ifeq (${IMX_ANDROID_BUILD},true)
BL31_SOURCES            +=      drivers/nxp/crypto/caam/src/caam.c	\
				drivers/nxp/crypto/caam/src/rng.c	\
				drivers/nxp/crypto/caam/src/jobdesc.c	\
				drivers/nxp/crypto/caam/src/sec_hw_specific.c	\
				drivers/nxp/crypto/caam/src/sec_jr_driver.c	\
				drivers/nxp/timer/nxp_timer.c
endif

include plat/imx/common/sci/sci_api.mk

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1

IMX_DEBUG_UART		?= 	0
$(eval $(call add_define,IMX_USE_UART${IMX_DEBUG_UART}))

DEBUG_CONSOLE		?= 	0
$(eval $(call add_define,DEBUG_CONSOLE))

ENABLE_CPU_DYNAMIC_RETENTION := 1
$(eval $(call add_define,ENABLE_CPU_DYNAMIC_RETENTION))
ENABLE_L2_DYNAMIC_RETENTION := 1
$(eval $(call add_define,ENABLE_L2_DYNAMIC_RETENTION))

ifeq (${PLAT},imx8dx)
BL32_BASE		?=	0x96000000
else
BL32_BASE		?=	0xfe000000
endif
$(eval $(call add_define,BL32_BASE))

BL32_SIZE		?=	0x2000000
$(eval $(call add_define,BL32_SIZE))

ifeq (${SPD},trusty)
IMX_SEPARATE_XLAT_TABLE :=	1

$(eval $(call add_define,IMX_SEPARATE_XLAT_TABLE))

BL31_SOURCES += plat/imx/common/ffa_shared_mem.c
endif

ifeq (${IMX_ANDROID_BUILD},true)
CONFIG_PHYS_64BIT	:=	1
$(eval $(call add_define,CONFIG_PHYS_64BIT))
NXP_SEC_LE		:=	1
$(eval $(call add_define,NXP_SEC_LE))
IMX_CAAM_ENABLE		:=	1
$(eval $(call add_define,IMX_CAAM_ENABLE))
IMX_CAAM_32BIT		:=	1
$(eval $(call add_define,IMX_CAAM_32BIT))
IMX_IMAGE_8Q		:=	1
$(eval $(call add_define,IMX_IMAGE_8Q))
CACHE_WRITEBACK_GRANULE :=	64
$(eval $(call add_define,CACHE_WRITEBACK_GRANULE))
endif
