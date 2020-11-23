#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum.
ERRATA_SOC_A008850	?= 0
ERRATA_SOC_A010539	?= 0

# SoC Errata
ifeq (${ERRATA_SOC_A008850},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A008850))
endif

ifeq (${ERRATA_SOC_A010539},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A010539))
endif

ifeq (${INCL_SOC_ERRATA_SOURCES},yes)
BL2_SOURCES	+= 	${PLAT_SOC_PATH}/erratas_soc.c
endif
