#
# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${NAND_ADDED},)

NAND_ADDED		:= 1

NAND_DRIVERS_PATH	:=  ${PLAT_DRIVERS_PATH}/ifc/nand

NAND_SOURCES		:=  $(NAND_DRIVERS_PATH)/ifc_nand.c \
			    drivers/io/io_block.c

PLAT_INCLUDES		+= -I$(NAND_DRIVERS_PATH)

ifeq (${BL_COMM_IFC_NAND_NEEDED},yes)
BL_COMMON_SOURCES	+= ${NAND_SOURCES}
else
ifeq (${BL2_IFC_NAND_NEEDED},yes)
BL2_SOURCES		+= ${NAND_SOURCES}
endif
ifeq (${BL31_IFC_NAND_NEEDED},yes)
BL31_SOURCES		+= ${NAND_SOURCES}
endif
endif

endif
