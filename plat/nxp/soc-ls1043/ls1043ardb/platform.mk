#
# Copyright 2018-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters

BOOT_MODE		?=	nor
BOARD			:=	ls1043ardb
POVDD_ENABLE		:=	no

# DDR Compilation Configs
CONFIG_DDR_NODIMM	:=	1
NUM_OF_DDRC		:=	1
DDRC_NUM_DIMM		:=	0
DDRC_NUM_CS		:=	4
DDR_ECC_EN		:=	no
CONFIG_STATIC_DDR 	:=	0

# On-Board Flash Details
# 128MB IFC NOR Flash
NOR_FLASH_SZ		:=	0x8000000

# Platform specific features.
WARM_BOOT		:=	no

 # Adding platform specific defines
$(eval $(call add_define_val,BOARD,'"${BOARD}"'))

ifeq (${POVDD_ENABLE},yes)
$(eval $(call add_define,CONFIG_POVDD_ENABLE))
endif

ifneq (${FLASH_TYPE},)
$(eval $(call add_define,CONFIG_${FLASH_TYPE}))
endif

ifneq (${NOR_FLASH_SZ},)
$(eval $(call add_define_val,NXP_NOR_FLASH_SIZE,${NOR_FLASH_SZ}))
endif

ifneq (${FSPI_ERASE_4K},)
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${FSPI_ERASE_4K}))
endif

ifneq (${NUM_OF_DDRC},)
$(eval $(call add_define_val,NUM_OF_DDRC,${NUM_OF_DDRC}))
endif

ifeq (${CONFIG_DDR_NODIMM},1)
$(eval $(call add_define,CONFIG_DDR_NODIMM))
DDRC_NUM_DIMM := 1
endif

ifneq (${DDRC_NUM_DIMM},)
$(eval $(call add_define_val,DDRC_NUM_DIMM,${DDRC_NUM_DIMM}))
endif

ifneq (${DDRC_NUM_CS},)
$(eval $(call add_define_val,DDRC_NUM_CS,${DDRC_NUM_CS}))
endif

ifeq (${DDR_ADDR_DEC},yes)
$(eval $(call add_define,CONFIG_DDR_ADDR_DEC))
endif

ifeq (${DDR_ECC_EN},yes)
$(eval $(call add_define,CONFIG_DDR_ECC_EN))
endif

ifeq (${CONFIG_STATIC_DDR},1)
$(eval $(call add_define,CONFIG_STATIC_DDR))
endif


# Platform can control the base address for non-volatile storage.
#$(eval $(call add_define_val,NV_STORAGE_BASE_ADDR,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - 2 * ${NXP_XSPI_NOR_UNIT_SIZE}'))

ifeq (${WARM_BOOT},yes)
$(eval $(call add_define_val,PHY_TRAINING_REGS_ON_FLASH,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - ${NXP_XSPI_NOR_UNIT_SIZE}'))
endif


 # Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c\
			${BOARD_PATH}/platform.c


# Adding SoC build info
include plat/nxp/soc-ls1043/soc.mk
