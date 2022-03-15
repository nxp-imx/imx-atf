#
# Copyright 2018-2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
BOOT_MODE	:=	qspi
BOARD		:=	ls1012afrwy

# DDR Compilation Configs
DDRC_NUM_CS     :=      1

# On-Board Flash Details
QSPI_FLASH_SZ   :=      0x200000

BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c

SUPPORTED_BOOT_MODE	:=	qspi

# Adding platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

# Adding SoC build info
include plat/nxp/soc-ls1012a/soc.mk
