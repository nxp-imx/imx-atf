#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters

BOOT_MODE	?=	qspi
BOARD		:=	ls1046ardb
POVDD_ENABLE	:=	no

 # DDR Compilation Configs
NUM_OF_DDRC	:=	1
DDRC_NUM_DIMM	:=	1
DDRC_NUM_CS	:=	4
DDR_ECC_EN	:=	yes
CONFIG_STATIC_DDR := 0

# DDR Errata
ERRATA_DDR_A008511		:=	1
ERRATA_DDR_A009803		:=	1
ERRATA_DDR_A009942		:=	1
ERRATA_DDR_A010165		:=	1

 # On-Board Flash Details
QSPI_FLASH_SZ	:=	0x20000000
NOR_FLASH_SIZE	:=	0x20000000

 # Platform specific features.
WARM_BOOT	:=	no

 # Adding platform specific defines
$(eval $(call add_define_val,BOARD,'"${BOARD}"'))

ifeq (${POVDD_ENABLE},yes)
$(eval $(call add_define,CONFIG_POVDD_ENABLE))
endif

ifneq (${FLASH_TYPE},)
$(eval $(call add_define,CONFIG_${FLASH_TYPE}))
endif

ifneq (${QSPI_FLASH_SZ},)
$(eval $(call add_define_val,NXP_QSPI_FLASH_SIZE,${QSPI_FLASH_SZ}))
endif

ifneq (${NOR_FLASH_SZ},)
$(eval $(call add_define_val,NXP_QSPI_FLASH_SIZE,${NOR_FLASH_SZ}))
endif


ifneq (${FSPI_ERASE_4K},)
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${FSPI_ERASE_4K}))
endif

ifneq (${NUM_OF_DDRC},)
$(eval $(call add_define_val,NUM_OF_DDRC,${NUM_OF_DDRC}))
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
include plat/nxp/soc-ls1046/soc.mk
