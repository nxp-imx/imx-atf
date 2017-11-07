PLAT_INCLUDES		:=	-Iplat/imx/imx8mq/include		\
				-Iplat/imx/common/include		\

PLAT_GIC_SOURCES	:=	drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv3.c		\
				plat/imx/common/plat_imx8_gic.c

BL31_SOURCES		+=	plat/imx/common/imx8_helpers.S		\
				plat/imx/common/mxcuart_console.S	\
				plat/imx/common/sip_svc.c		\
				plat/imx/imx8mq/imx8m_bl31_setup.c	\
				plat/imx/imx8mq/src.c			\
				plat/imx/imx8mq/gpc.c			\
				plat/imx/imx8mq/ddrc.c			\
				plat/imx/imx8mq/imx8m_psci.c		\
				plat/imx/imx8mq/imx_csu.c		\
				plat/imx/imx8mq/imx_rdc.c		\
				plat/imx/common/imx8_topology.c		\
				plat/common/plat_psci_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c	\
				lib/xlat_tables/xlat_tables_common.c	\
				lib/cpus/aarch64/cortex_a53.S		\
				drivers/console/aarch64/console.S	\
				${PLAT_GIC_SOURCES}			\
				drivers/arm/tzc/tzc380.c

ENABLE_PLAT_COMPAT	:=	0
USE_COHERENT_MEM	:=	0
MULTI_CONSOLE_API	:=	1
RESET_TO_BL31		:=	1
ERROR_DEPRECATED	:=	1
XLAT_TABLE_IN_OCRAM_S	:=	1
ifneq (${SPD},none)
$(eval $(call add_define,TEE_IMX8))
endif
$(eval $(call add_define,XLAT_TABLE_IN_OCRAM_S))
