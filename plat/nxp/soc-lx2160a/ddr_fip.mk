#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

DDR_PHY_BIN_PATH	?=	./ddr-phy-binary/lx2160a

ifeq (${DDR_IMEM_UDIMM_1D},)
    DDR_IMEM_UDIMM_1D	:=	${DDR_PHY_BIN_PATH}/ddr4_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_UDIMM_2D},)
    DDR_IMEM_UDIMM_2D	:=	${DDR_PHY_BIN_PATH}/ddr4_2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_UDIMM_1D},)
    DDR_DMEM_UDIMM_1D	:=	${DDR_PHY_BIN_PATH}/ddr4_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_UDIMM_2D},)
    DDR_DMEM_UDIMM_2D	:=	${DDR_PHY_BIN_PATH}/ddr4_2d_pmu_train_dmem.bin
endif

ifeq (${DDR_IMEM_RDIMM_1D},)
    DDR_IMEM_RDIMM_1D	:=	${DDR_PHY_BIN_PATH}/ddr4_rdimm_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_RDIMM_2D},)
    DDR_IMEM_RDIMM_2D	:=	${DDR_PHY_BIN_PATH}/ddr4_rdimm2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_RDIMM_1D},)
    DDR_DMEM_RDIMM_1D	:=	${DDR_PHY_BIN_PATH}/ddr4_rdimm_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_RDIMM_2D},)
    DDR_DMEM_RDIMM_2D	:=	${DDR_PHY_BIN_PATH}/ddr4_rdimm2d_pmu_train_dmem.bin
endif

$(shell mkdir -p '${BUILD_PLAT}')

ifeq (${DDR_FIP_NAME},)
ifeq (${TRUSTED_BOARD_BOOT},1)
	DDR_FIP_NAME	:= ddr_fip_sec.bin
else
	DDR_FIP_NAME	:= ddr_fip.bin
endif
endif

ifneq (${TRUSTED_BOARD_BOOT},1)

DDR_FIP_ARGS += --ddr-immem-udimm-1d ${DDR_IMEM_UDIMM_1D} \
		--ddr-immem-udimm-2d ${DDR_IMEM_UDIMM_2D} \
		--ddr-dmmem-udimm-1d ${DDR_DMEM_UDIMM_1D} \
		--ddr-dmmem-udimm-2d ${DDR_DMEM_UDIMM_2D} \
		--ddr-immem-rdimm-1d ${DDR_IMEM_RDIMM_1D} \
		--ddr-immem-rdimm-2d ${DDR_IMEM_RDIMM_2D} \
		--ddr-dmmem-rdimm-1d ${DDR_DMEM_RDIMM_1D} \
		--ddr-dmmem-rdimm-2d ${DDR_DMEM_RDIMM_2D}
endif


ifeq (${TRUSTED_BOARD_BOOT},1)
ifeq (${MBEDTLS_DIR},)
include plat/nxp/soc-lx2160/ddr_sb.mk
else
include plat/nxp/soc-lx2160/ddr_tbbr.mk
endif
endif
