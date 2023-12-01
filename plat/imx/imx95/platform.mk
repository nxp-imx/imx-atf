#
# Copyright 2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ENABLE_SCMI_SM := 1

PLAT_INCLUDES		:=	-Iplat/imx/common/include		\
				-Iplat/imx/imx95/include		\
				-Idrivers/arm/css/scmi			\
				-Idrivers/arm/css/scmi/vendor

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

GICV3_SUPPORT_GIC600  :=      1

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

IMX_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	drivers/arm/css/scmi/scmi_common.c		\
				drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
				drivers/arm/css/scmi/scmi_sys_pwr_proto.c	\
				drivers/arm/css/scmi/vendor/scmi_imx9.c		\
				plat/imx/imx95/imx95_psci.c			\
				plat/imx/imx95/scmi/scmi_client.c		\
				plat/common/aarch64/crash_console_helpers.S     \
				plat/imx/imx95/aarch64/plat_helpers.S		\
				plat/imx/imx95/plat_topology.c			\
				plat/imx/common/lpuart_console.S		\
				plat/imx/imx95/imx95_bl31_setup.c		\
				lib/cpus/aarch64/cortex_a55.S			\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				plat/imx/common/imx_sip_handler.c		\
				plat/imx/common/imx_sip_svc.c			\
				plat/imx/common/ele_api.c			\
				${IMX_GIC_SOURCES}				\
				${XLAT_TABLES_LIB_SRCS}

ifeq (${SPD},trusty)
	BL31_SOURCES += plat/imx/common/ffa_shared_mem.c
endif

RESET_TO_BL31		:=	1
HW_ASSISTED_COHERENCY	:= 	1
USE_COHERENT_MEM	:=	0
PROGRAMMABLE_RESET_ADDRESS := 1
COLD_BOOT_SINGLE_CPU := 1
ERRATA_A55_1530923 := 1

BL32_BASE               ?=      0x8C000000
BL32_SIZE               ?=      0x02000000
$(eval $(call add_define,BL32_BASE))
$(eval $(call add_define,BL32_SIZE))
