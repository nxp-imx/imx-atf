# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Adding SoC specific defines

ifneq (${CACHE_LINE},)
$(eval $(call add_define_val,PLATFORM_CACHE_LINE_SHIFT,${CACHE_LINE}))
$(eval CACHE_WRITEBACK_GRANULE=$(shell echo $$((1 << $(CACHE_LINE)))))
$(eval $(call add_define_val,CACHE_WRITEBACK_GRANULE,$(CACHE_WRITEBACK_GRANULE)))
endif

ifneq (${INTERCONNECT},)
$(eval $(call add_define,NXP_HAS_${INTERCONNECT}))
ifeq (${INTERCONNECT}, CCI400)
ICNNCT_ID := 0x420
$(eval $(call add_define,ICNNCT_ID))
endif
endif

ifneq (${CHASSIS},)
$(info setting chssis)
$(eval $(call add_define,CONFIG_CHASSIS_${CHASSIS}))
endif

ifneq (${PLAT_DDR_PHY},)
$(eval $(call add_define,NXP_DDR_${PLAT_DDR_PHY}))
endif

ifneq (${PHYS_SYS},)
$(eval $(call add_define,CONFIG_PHYS_64BIT))
endif

ifneq (${CSF_HDR_SZ},)
$(eval $(call add_define_val,CSF_HDR_SZ,${CSF_HDR_SZ}))
endif

ifneq (${OCRAM_START_ADDR},)
$(eval $(call add_define_val,NXP_OCRAM_ADDR,${OCRAM_START_ADDR}))
endif

ifneq (${OCRAM_SIZE},)
$(eval $(call add_define_val,NXP_OCRAM_SIZE,${OCRAM_SIZE}))
endif

ifneq (${NXP_ROM_RSVD},)
$(eval $(call add_define_val,NXP_ROM_RSVD,${NXP_ROM_RSVD}))
endif

ifneq (${BL2_BASE},)
$(eval $(call add_define_val,BL2_BASE,${BL2_BASE}))
endif

ifeq (${SEC_MEM_NON_COHERENT},yes)
$(eval $(call add_define,SEC_MEM_NON_COHERENT))
endif

# Module endianness definition
NXP_MODULES := \
  NXP_ESDHC \
  NXP_SFP \
  NXP_GPIO \
  NXP_SNVS \
  NXP_GUR \
  NXP_FSPI \
  NXP_SEC \
  NXP_DDR \
  NXP_QSPI \
  NXP_SCFG \
  NXP_IFC

define add_module_endianness_define
  ifneq ($$($(1)_ENDIANNESS),)
    $$(eval $$(call add_define,$(1)_$$($(1)_ENDIANNESS)))
  else
    $$(eval $$(call add_define,$(1)_BE))
  endif
endef

$(foreach m,$(NXP_MODULES),$(eval $(call add_module_endianness_define,$(m))))
# End Module endianness definition

ifneq (${NXP_SFP_VER},)
$(eval $(call add_define,NXP_SFP_VER_${NXP_SFP_VER}))
endif

ifneq (${NXP_DDR_INTLV_256B},)
$(eval $(call add_define,NXP_DDR_INTLV_256B))
endif

ifneq (${PLAT_XLAT_TABLES_DYNAMIC},)
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
endif

# Use a personal linker file
ifeq (${SEPARATE_RW_AND_NOLOAD}, 1)
$(eval $(call add_define,SEPARATE_RW_AND_NOLOAD))
BL2_LINKERFILE	:=	${PLAT_SOC_PATH}/bl2_el3_${SOC}.ld.S
endif

ifeq (${OCRAM_ECC_EN},yes)
$(eval $(call add_define,CONFIG_OCRAM_ECC_EN))
include ${PLAT_COMMON_PATH}/ocram/ocram.mk
endif
