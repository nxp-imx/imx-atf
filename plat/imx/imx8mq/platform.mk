PLAT_INCLUDES		:=	-Iplat/imx/imx8mq/include		\
				-Iplat/imx/common/include		\

PLAT_GIC_SOURCES	:=	drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/common/gic_common.c	\
				plat/common/plat_gicv3.c		\
				plat/imx/common/plat_imx8_gic.c

PLAT_DRAM_SOURCES	:=	plat/imx/common/imx8m/dram.c		\
				plat/imx/common/imx8m/clock.c		\
				plat/imx/common/imx8m/lpddr4_retention.c \
				plat/imx/common/imx8m/ddr4_retention.c \
				plat/imx/common/imx8m/lpddr4_helper.c	\
				plat/imx/common/imx8m/lpddr4_dvfs.c	\
				plat/imx/common/imx8m/ddr4_dvfs.c

BL31_SOURCES		+=	plat/imx/common/imx8_helpers.S		\
				plat/imx/common/mxcuart_console.S	\
				plat/imx/imx8mq/imx8mq_bl31_setup.c	\
				plat/imx/imx8mq/src.c			\
				plat/imx/common/imx8m/hab.c		\
				plat/imx/imx8mq/gpc.c			\
				plat/imx/common/imx8_sip_svc.c		\
				plat/imx/common/misc.c			\
				plat/imx/imx8mq/imx8mq_psci.c		\
				plat/imx/imx8mq/imx8mq_gpio.c		\
				plat/imx/common/imx8m/imx_csu.c		\
				plat/imx/common/imx8m/imx_rdc.c		\
				plat/imx/common/imx8_topology.c		\
				plat/common/plat_psci_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c	\
				lib/xlat_tables/xlat_tables_common.c	\
				lib/cpus/aarch64/cortex_a53.S		\
				drivers/console/aarch64/console.S	\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				${PLAT_GIC_SOURCES}			\
				${PLAT_DRAM_SOURCES}				\
				drivers/arm/tzc/tzc380.c

ENABLE_PLAT_COMPAT	:=	0
USE_COHERENT_MEM	:=	0
MULTI_CONSOLE_API	:=	1
RESET_TO_BL31		:=	1
ERROR_DEPRECATED	:=	1
XLAT_TABLE_IN_OCRAM_S	:=	1
STACK_IN_OCRAM_S       :=      1
ifneq (${SPD},none)
$(eval $(call add_define,TEE_IMX8))
endif
$(eval $(call add_define,XLAT_TABLE_IN_OCRAM_S))
$(eval $(call add_define,STACK_IN_OCRAM_S))

A53_DISABLE_NON_TEMPORAL_HINT := 0
ERRATA_A53_835769	:=	1
ERRATA_A53_843419	:=	1
ERRATA_A53_855873	:=	1

ifneq (${DECRYPTED_BUFFER_START},)
$(eval $(call add_define,DECRYPTED_BUFFER_START))

ifneq (${DECRYPTED_BUFFER_LEN},)
$(eval $(call add_define,DECRYPTED_BUFFER_LEN))
endif
endif

ifneq (${DECODED_BUFFER_START},)
$(eval $(call add_define,DECODED_BUFFER_START))

ifneq (${DECODED_BUFFER_LEN},)
$(eval $(call add_define,DECODED_BUFFER_LEN))
endif
endif
