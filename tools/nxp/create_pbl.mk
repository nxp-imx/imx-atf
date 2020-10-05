#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

CREATE_PBL	?=	${PLAT_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${PLAT_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

#SWAP is required for Chassis 2 platforms - LS102, ls1043 and ls1046 for QSPI
ifeq (${SOC},ls1046)
SOC_NUM :=	1046
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1043)
SOC_NUM :=	1043
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1012)
SOC_NUM :=	1012
SWAP	= 	1
CH	=	2
else ifeq (${SOC},ls1088)
SOC_NUM :=	1088
CH	=	3
else ifeq (${SOC},ls2088)
SOC_NUM :=	2088
CH	=	3
else ifeq (${SOC},lx2160)
SOC_NUM :=	2160
CH	=	3
else ifeq (${SOC},ls1028)
SOC_NUM :=	1028
CH	=	3
else
$(error "Check SOC Not defined in create_pbl.mk.")
endif

ifeq (${CH},2)

include ${PLAT_TOOL_PATH}/pbl_ch2.mk

endif #CH2

ifeq (${CH},3)

include ${PLAT_TOOL_PATH}/pbl_ch3.mk

endif #CH3


