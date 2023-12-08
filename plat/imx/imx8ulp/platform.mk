#
# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Translation tables library
include lib/xlat_tables_v2/xlat_tables.mk

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

PLAT_INCLUDES		:=	-Iplat/imx/imx8ulp/include		\
				-Iplat/imx/common/include		\
				-Iplat/imx/imx8ulp/upower
ifeq (${IMX_ANDROID_BUILD},true)
PLAT_INCLUDES           +=	-Iinclude/drivers/nxp/crypto/caam	\
				-Iinclude/drivers/nxp/timer
endif

IMX_GIC_SOURCES		:=	${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/lpuart_console.S	\
				plat/imx/common/imx8_helpers.S		\
				plat/imx/imx8ulp/imx8ulp_bl31_setup.c	\
				plat/imx/imx8ulp/imx8ulp_psci.c		\
				plat/imx/imx8ulp/apd_context.c		\
				plat/imx/common/imx8_topology.c		\
				plat/imx/common/imx_sip_svc.c		\
				plat/imx/common/imx_sip_handler.c	\
				plat/imx/common/imx_bl31_common.c	\
				plat/common/plat_psci_common.c		\
				lib/cpus/aarch64/cortex_a35.S		\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				plat/imx/imx8ulp/xrdc/xrdc_core.c		\
				plat/imx/imx8ulp/imx8ulp_caam.c         \
				plat/imx/imx8ulp/dram.c 	        \
				drivers/scmi-msg/base.c			\
				drivers/scmi-msg/entry.c		\
				drivers/scmi-msg/smt.c			\
				drivers/scmi-msg/power_domain.c		\
				drivers/scmi-msg/sensor.c		\
				plat/imx/imx8ulp/scmi/scmi.c		\
				plat/imx/imx8ulp/scmi/scmi_pd.c		\
				plat/imx/imx8ulp/scmi/scmi_sensor.c	\
				plat/imx/imx8ulp/upower/upower_api.c	\
				plat/imx/imx8ulp/upower/upower_hal.c	\
				${XLAT_TABLES_LIB_SRCS}			\
				${IMX_GIC_SOURCES}
ifeq (${IMX_ANDROID_BUILD},true)
BL31_SOURCES            +=      drivers/nxp/crypto/caam/src/caam.c	\
				drivers/nxp/crypto/caam/src/rng.c	\
				drivers/nxp/crypto/caam/src/jobdesc.c	\
				drivers/nxp/crypto/caam/src/sec_hw_specific.c	\
				drivers/nxp/crypto/caam/src/sec_jr_driver.c	\
				drivers/nxp/timer/nxp_timer.c
endif

ifeq (${SPD},trusty)
	BL31_SOURCES += plat/imx/common/ffa_shared_mem.c
endif

ifeq ($(findstring clang,$(notdir $(CC))),)
    TF_CFLAGS_aarch64	+=	-fno-strict-aliasing
endif

USE_COHERENT_MEM	:=	1
RESET_TO_BL31		:=	1
SEPARATE_NOBITS_REGION	:=	1
SEPARATE_RWDATA_REGION	:=	1
PROGRAMMABLE_RESET_ADDRESS	:=	1
COLD_BOOT_SINGLE_CPU := 1
WARMBOOT_ENABLE_DCACHE_EARLY	:=	1
BL32_BASE		?=	0xa6000000
BL32_SIZE		?=	0x2000000
$(eval $(call add_define,BL32_BASE))
$(eval $(call add_define,BL32_SIZE))

ifdef IMX8ULP_DSL_SUPPORT
$(eval $(call add_define,IMX8ULP_DSL_SUPPORT))
endif

ifdef IMX8ULP_TPM_TIMERS
$(eval $(call add_define,IMX8ULP_TPM_TIMERS))
endif

ifeq (${SPD},trusty)
	BL31_CFLAGS    +=      -DPLAT_XLAT_TABLES_DYNAMIC=1
endif

ifeq (${IMX_ANDROID_BUILD},true)
CONFIG_PHYS_64BIT	:=	1
$(eval $(call add_define,CONFIG_PHYS_64BIT))
NXP_SEC_LE		:=	1
$(eval $(call add_define,NXP_SEC_LE))
IMX_CAAM_ENABLE		:=	1
$(eval $(call add_define,IMX_CAAM_ENABLE))
IMX_CAAM_32BIT		:=	1
$(eval $(call add_define,IMX_CAAM_32BIT))
IMX_IMAGE_8ULP		:=	1
$(eval $(call add_define,IMX_IMAGE_8ULP))
CACHE_WRITEBACK_GRANULE :=	64
$(eval $(call add_define,CACHE_WRITEBACK_GRANULE))
endif
