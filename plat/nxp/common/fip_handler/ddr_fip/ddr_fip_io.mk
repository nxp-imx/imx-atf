#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${DDR_FIP_IO_STORAGE_ADDED},)

$(eval $(call SET_FLAG,IMG_LOADR_NEEDED,BL2))

DDR_FIP_IO_STORAGE_ADDED	:= 1
$(eval $(call add_define,CONFIG_DDR_FIP_IMAGE))

DDR_FIP_IO_STORAGE_PATH	:=  ${PLAT_COMMON_PATH}/fip_handler/ddr_fip

PLAT_INCLUDES		+= -I$(DDR_FIP_IO_STORAGE_PATH)

DDR_FIP_IO_SOURCES	+= $(DDR_FIP_IO_STORAGE_PATH)/ddr_io_storage.c

ifeq (${BL_COMM_DDR_FIP_IO_NEEDED},yes)
BL_COMMON_SOURCES	+= ${DDR_FIP_IO_SOURCES}
else
ifeq (${BL2_DDR_FIP_IO_NEEDED},yes)
BL2_SOURCES		+= ${DDR_FIP_IO_SOURCES}
endif
ifeq (${BL31_DDR_FIP_IO_NEEDED},yes)
BL31_SOURCES		+= ${DDR_FIP_IO_SOURCES}
endif
endif
endif
#------------------------------------------------
