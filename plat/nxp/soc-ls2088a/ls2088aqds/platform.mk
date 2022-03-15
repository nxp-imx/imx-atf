#
# Copyright 2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

# board-specific build parameters
BOOT_MODE		?=	nor
BOARD			:=	ls2088aqds

# DDR Compilation Configs
CONFIG_DDR_NODIMM	:=	0
NUM_OF_DDRC		:=	2
DDRC_NUM_DIMM		:=	2
DDRC_NUM_CS		:=	4
DDR_ECC_EN		:=	yes
CONFIG_STATIC_DDR 	:=	0

# On-Board Flash Details
# 128MB IFC NOR Flash
NOR_FLASH_SZ		:=	0x8000000

# Platform specific features.
WARM_BOOT		:=	no

# Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c\
			${BOARD_PATH}/platform.c

SUPPORTED_BOOT_MODE	:=	nor

# Adding platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

# Adding SoC build info
include plat/nxp/soc-ls2088a/soc.mk
