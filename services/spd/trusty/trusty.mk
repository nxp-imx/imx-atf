#
# Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TRUSTY_SPD_WITH_SHARED_MEM ?= 1

SPD_INCLUDES		:=

SPD_SOURCES		:=	services/spd/trusty/trusty.c		\
				services/spd/trusty/trusty_helpers.S

ifeq (${TRUSTY_SPD_WITH_GENERIC_SERVICES},1)
SPD_SOURCES		+=	services/spd/trusty/generic-arm64-smcall.c
endif

ifeq (${TRUSTY_SPD_WITH_SHARED_MEM},1)
BL31_CFLAGS		+=	-DPLAT_XLAT_TABLES_DYNAMIC=1 \
				-DTRUSTY_SPM=1
SPD_SOURCES		+=	services/spd/trusty/shared-mem-smcall.c
endif

# On Tegra, BL2 does not map us into memory before our spd is initialized.
# Setting LATE_MAPPED_BL32 indicates that we need to dynamically map in
# our first page for bittage detection.
ifeq (${PLAT},tegra)
CFLAGS                  +=      -DLATE_MAPPED_BL32
endif

NEED_BL32		:=	yes

CTX_INCLUDE_FPREGS	:=	1
