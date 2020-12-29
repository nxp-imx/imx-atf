#
# Copyright 2018, 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 8850 workaround during reset.
ERRATA_SOC_A008850	?=0

ifeq (${ERRATA_SOC_A008850},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A008850))
endif

# Flag to apply erratum 9660 workaround during reset.
ERRATA_SOC_A009660	?=0

ifeq (${ERRATA_SOC_A009660},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A009660))
endif

# Flag to apply erratum 010539 workaround during reset.
ERRATA_SOC_A010539	?=0

ifeq (${ERRATA_SOC_A010539},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A010539))
endif

# Flag to apply erratum 50426 workaround during reset.
ERRATA_SOC_A050426	?=0

ifeq (${ERRATA_SOC_A050426},1)
INCL_SOC_ERRATA_SOURCES := yes
$(eval $(call add_define,ERRATA_SOC_A050426))
endif

ifeq (${INCL_SOC_ERRATA_SOURCES},yes)
BL2_SOURCES	+= 	${PLAT_COMMON_PATH}/soc_errata/errata.c
endif
