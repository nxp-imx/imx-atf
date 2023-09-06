/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

/*
 * Subset of Arm PSA Firmware Framework for Arm v8-A 1.0 EAC
 * (https://developer.arm.com/docs/den0077/a) needed for shared memory.
 */

#include <services/ffa_svc.h>
#include "smcall.h"

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(e) _Static_assert(e, #e)
#endif

#define TRUSTY_FFA_CURRENT_VERSION_MAJOR (1U)
#define TRUSTY_FFA_CURRENT_VERSION_MINOR (0U)

#define FFA_VERSION_TO_MAJOR(V) ((V) >> FFA_VERSION_MAJOR_SHIFT)
#define MAKE_TRUSTY_FFA_CURRENT_VERSION	\
	MAKE_FFA_VERSION(TRUSTY_FFA_CURRENT_VERSION_MAJOR, \
			 TRUSTY_FFA_CURRENT_VERSION_MINOR)

#define FFA_NWLD_ID_BASE	0x0
#define FFA_SWLD_ID_BASE	0x8000

#define SMC_ENTITY_SHARED_MEMORY 4

#define SMC_FASTCALL_NR_SHARED_MEMORY(nr) \
    SMC_FASTCALL_NR(SMC_ENTITY_SHARED_MEMORY, nr)
#define SMC_FASTCALL64_NR_SHARED_MEMORY(nr) \
    SMC_FASTCALL64_NR(SMC_ENTITY_SHARED_MEMORY, nr)

#define FFA_PAGE_SIZE (4096)

/**
 * typedef ffa_endpoint_id16_t - Endpoint ID
 *
 * Current implementation only supports VMIDs. FFA spec also support stream
 * endpoint ids.
 */
typedef uint16_t ffa_endpoint_id16_t;

/**
 * struct ffa_cons_mrd - Constituent memory region descriptor
 * @address:
 *         Start address of contiguous memory region. Must be 4K page aligned.
 * @page_count:
 *         Number of 4K pages in region.
 * @reserved_12_15:
 *         Reserve bytes 12-15 to pad struct size to 16 bytes.
 */
struct ffa_cons_mrd {
    uint64_t address;
    uint32_t page_count;
    uint32_t reserved_12_15;
};
STATIC_ASSERT(sizeof(struct ffa_cons_mrd) == 16);

/**
 * struct ffa_comp_mrd - Composite memory region descriptor
 * @total_page_count:
 *         Number of 4k pages in memory region. Must match sum of
 *         @address_range_array[].page_count.
 * @address_range_count:
 *         Number of entries in @address_range_array.
 * @reserved_8_15:
 *         Reserve bytes 8-15 to pad struct size to 16 byte alignment and
 *         make @address_range_array 16 byte aligned.
 * @address_range_array:
 *         Array of &struct ffa_cons_mrd entries.
 */
struct ffa_comp_mrd {
    uint32_t total_page_count;
    uint32_t address_range_count;
    uint64_t reserved_8_15;
    struct ffa_cons_mrd address_range_array[];
};
STATIC_ASSERT(sizeof(struct ffa_comp_mrd) == 16);

/**
 * typedef ffa_mem_attr8_t - Memory region attributes
 *
 * * @FFA_MEM_ATTR_DEVICE_NGNRNE:
 *     Device-nGnRnE.
 * * @FFA_MEM_ATTR_DEVICE_NGNRE:
 *     Device-nGnRE.
 * * @FFA_MEM_ATTR_DEVICE_NGRE:
 *     Device-nGRE.
 * * @FFA_MEM_ATTR_DEVICE_GRE:
 *     Device-GRE.
 * * @FFA_MEM_ATTR_NORMAL_MEMORY_UNCACHED
 *     Normal memory. Non-cacheable.
 * * @FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB
 *     Normal memory. Write-back cached.
 * * @FFA_MEM_ATTR_NON_SHAREABLE
 *     Non-shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 * * @FFA_MEM_ATTR_OUTER_SHAREABLE
 *     Outer Shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 * * @FFA_MEM_ATTR_INNER_SHAREABLE
 *     Inner Shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 * * @FFA_MEM_ATTR_NONSECURE
 *     Set on retrieve if memory is nonsecure and client requested it be set.
 */
typedef uint8_t ffa_mem_attr8_t;
#define FFA_MEM_ATTR_DEVICE_NGNRNE ((1U << 4) | (0x0U << 2))
#define FFA_MEM_ATTR_DEVICE_NGNRE ((1U << 4) | (0x1U << 2))
#define FFA_MEM_ATTR_DEVICE_NGRE ((1U << 4) | (0x2U << 2))
#define FFA_MEM_ATTR_DEVICE_GRE ((1U << 4) | (0x3U << 2))
#define FFA_MEM_ATTR_NORMAL_MEMORY_UNCACHED ((2U << 4) | (0x1U << 2))
#define FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB ((2U << 4) | (0x3U << 2))
#define FFA_MEM_ATTR_NON_SHAREABLE (0x0U << 0)
#define FFA_MEM_ATTR_OUTER_SHAREABLE (0x2U << 0)
#define FFA_MEM_ATTR_INNER_SHAREABLE (0x3U << 0)
#define FFA_MEM_ATTR_NONSECURE (1U << 6)

/**
 * typedef ffa_mem_perm8_t - Memory access permissions
 *
 * * @FFA_MEM_ATTR_RO
 *     Request or specify read-only mapping.
 * * @FFA_MEM_ATTR_RW
 *     Request or allow read-write mapping.
 * * @FFA_MEM_PERM_NX
 *     Deny executable mapping.
 * * @FFA_MEM_PERM_X
 *     Request executable mapping.
 */
typedef uint8_t ffa_mem_perm8_t;
#define FFA_MEM_PERM_RO (1U << 0)
#define FFA_MEM_PERM_RW (1U << 1)
#define FFA_MEM_PERM_NX (1U << 2)
#define FFA_MEM_PERM_X (1U << 3)

/**
 * typedef ffa_mem_flag8_t - Endpoint memory flags
 *
 * * @FFA_MEM_FLAG_NON_RETRIEVAL_BORROWER
 *     Non-retrieval Borrower. Memory region must not be or was not retrieved on
 *     behalf of this endpoint.
 */
typedef uint8_t ffa_mem_flag8_t;
#define FFA_MEM_FLAG_NON_RETRIEVAL_BORROWER (1U << 0)

/**
 * typedef ffa_mtd_flag32_t - Memory transaction descriptor flags
 *
 * * @FFA_MTD_FLAG_ZERO_MEMORY
 *     Zero memory after unmapping from sender (must be 0 for share).
 * * @FFA_MTD_FLAG_TIME_SLICING
 *     Not supported by this implementation.
 * * @FFA_MTD_FLAG_ZERO_MEMORY_AFTER_RELINQUISH
 *     Zero memory after unmapping from borrowers (must be 0 for share).
 * * @FFA_MTD_FLAG_TYPE_MASK
 *     Bit-mask to extract memory management transaction type from flags.
 * * @FFA_MTD_FLAG_TYPE_SHARE_MEMORY
 *     Share memory transaction flag.
 *     Used by @SMC_FC_FFA_MEM_RETRIEVE_RESP to indicate that memory came from
 *     @SMC_FC_FFA_MEM_SHARE and by @SMC_FC_FFA_MEM_RETRIEVE_REQ to specify that
 *     it must have.
 * * @FFA_MTD_FLAG_TYPE_LEND_MEMORY
 *     Lend memory transaction flag.
 *     Used by @SMC_FC_FFA_MEM_RETRIEVE_RESP to indicate that memory came from
 *     @SMC_FC_FFA_MEM_LEND and by @SMC_FC_FFA_MEM_RETRIEVE_REQ to specify that
 *     it must have.
 * * @FFA_MTD_FLAG_ADDRESS_RANGE_ALIGNMENT_HINT_MASK
 *     Not supported by this implementation.
 */
typedef uint32_t ffa_mtd_flag32_t;
#define FFA_MTD_FLAG_ZERO_MEMORY (1U << 0)
#define FFA_MTD_FLAG_TIME_SLICING (1U << 1)
#define FFA_MTD_FLAG_ZERO_MEMORY_AFTER_RELINQUISH (1U << 2)
#define FFA_MTD_FLAG_TYPE_MASK (3U << 3)
#define FFA_MTD_FLAG_TYPE_SHARE_MEMORY (1U << 3)
#define FFA_MTD_FLAG_TYPE_LEND_MEMORY (2U << 3)
#define FFA_MTD_FLAG_ADDRESS_RANGE_ALIGNMENT_HINT_MASK (0x1FU << 5)

/**
 * struct ffa_mapd - Memory access permissions descriptor
 * @endpoint_id:
 *         Endpoint id that @memory_access_permissions and @flags apply to.
 *         (&typedef ffa_endpoint_id16_t).
 * @memory_access_permissions:
 *         FFA_MEM_PERM_* values or'ed together (&typedef ffa_mem_perm8_t).
 * @flags:
 *         FFA_MEM_FLAG_* values or'ed together (&typedef ffa_mem_flag8_t).
 */
struct ffa_mapd {
    ffa_endpoint_id16_t endpoint_id;
    ffa_mem_perm8_t memory_access_permissions;
    ffa_mem_flag8_t flags;
};
STATIC_ASSERT(sizeof(struct ffa_mapd) == 4);

/**
 * struct ffa_emad - Endpoint memory access descriptor.
 * @mapd:  &struct ffa_mapd.
 * @comp_mrd_offset:
 *         Offset of &struct ffa_comp_mrd form start of &struct ffa_mtd.
 * @reserved_8_15:
 *         Reserved bytes 8-15. Must be 0.
 */
struct ffa_emad {
    struct ffa_mapd mapd;
    uint32_t comp_mrd_offset;
    uint64_t reserved_8_15;
};
STATIC_ASSERT(sizeof(struct ffa_emad) == 16);

/**
 * struct ffa_mtd - Memory transaction descriptor.
 * @sender_id:
 *         Sender endpoint id.
 * @memory_region_attributes:
 *         FFA_MEM_ATTR_* values or'ed together (&typedef ffa_mem_attr8_t).
 * @reserved_3:
 *         Reserved bytes 3. Must be 0.
 * @flags:
 *         FFA_MTD_FLAG_* values or'ed together (&typedef ffa_mtd_flag32_t).
 * @handle:
 *         Id of shared memory object. Most be 0 for MEM_SHARE.
 * @tag:   Client allocated tag. Must match original value.
 * @reserved_24_27:
 *         Reserved bytes 24-27. Must be 0.
 * @emad_count:
 *         Number of entries in @emad. Must be 1 in current implementation.
 *         FFA spec allows more entries.
 * @emad:
 *         Endpoint memory access descriptor array (see @struct ffa_emad).
 */
struct ffa_mtd {
    ffa_endpoint_id16_t sender_id;
    ffa_mem_attr8_t memory_region_attributes;
    uint8_t reserved_3;
    ffa_mtd_flag32_t flags;
    uint64_t handle;
    uint64_t tag;
    uint32_t reserved_24_27;
    uint32_t emad_count;
    struct ffa_emad emad[];
};
STATIC_ASSERT(sizeof(struct ffa_mtd) == 32);

/**
 * struct ffa_mem_relinquish_descriptor - Relinquish request descriptor.
 * @handle:
 *         Id of shared memory object to relinquish.
 * @flags:
 *         If bit 0 is set clear memory after unmapping from borrower. Must be 0
 *         for share. Bit[1]: Time slicing. Not supported, must be 0. All other
 *         bits are reserved 0.
 * @endpoint_count:
 *         Number of entries in @endpoint_array.
 * @endpoint_array:
 *         Array of endpoint ids.
 */
struct ffa_mem_relinquish_descriptor {
    uint64_t handle;
    uint32_t flags;
    uint32_t endpoint_count;
    ffa_endpoint_id16_t endpoint_array[];
};
STATIC_ASSERT(sizeof(struct ffa_mem_relinquish_descriptor) == 16);

/**
 * struct ffa_partition_info - FFA partition info descriptor.
 * @id:
 *         16-bit ID of the partition
 * @execution_ctx_count:
 *         Number of execution contexts implemented by this partition
 * @properties:
 *         Flags to determine partition properties. Like direct/indirect
 *         messages send/receive capabilities.
 */
struct ffa_partition_info {
	uint16_t id;
	uint16_t execution_ctx_count;
#define FFA_PART_PROP_RECV_DIRECT (1U)
#define FFA_PART_PROP_SEND_DIRECT (1U << 1)
#define FFA_PART_PROP_VM_MSGS (1U << 6)
	uint32_t properties;
};

/**
 * typedef ffa_features2_t - FFA_FEATURES values returned in w2
 *
 * * @FFA_FEATURES2_RXTX_MAP_BUF_SIZE_MASK
 *     For RXTX_MAP: min buffer size and alignment boundary mask.
 * * @FFA_FEATURES2_RXTX_MAP_BUF_SIZE_4K
 *     For RXTX_MAP: min buffer size and alignment boundary is 4K.
 * * @FFA_FEATURES2_RXTX_MAP_BUF_SIZE_64K
 *     For RXTX_MAP: min buffer size and alignment boundary is 64K.
 * * @FFA_FEATURES2_RXTX_MAP_BUF_SIZE_16K
 *     For RXTX_MAP: min buffer size and alignment boundary is 16K.
 * * @FFA_FEATURES2_MEM_DYNAMIC_BUFFER
 *     Supports custom buffers for memory transactions.
 * * @FFA_FEATURES2_MEM_RETRIEVE_REQ_NS_BIT
 *     Supports setting the NS bit on retrieved descriptors.
 *
 * For all other bits and commands: must be 0.
 */
typedef uint32_t ffa_features2_t;
#define FFA_FEATURES2_RXTX_MAP_BUF_SIZE_MASK 0x3U
#define FFA_FEATURES2_RXTX_MAP_BUF_SIZE_4K 0x0U
#define FFA_FEATURES2_RXTX_MAP_BUF_SIZE_64K 0x1U
#define FFA_FEATURES2_RXTX_MAP_BUF_SIZE_16K 0x2U
#define FFA_FEATURES2_MEM_DYNAMIC_BUFFER 0x1U
#define FFA_FEATURES2_MEM_RETRIEVE_REQ_NS_BIT 0x2U

/**
 * typedef ffa_features3_t - FFA_FEATURES values returned in w3
 *
 * * @FFA_FEATURES3_MEM_RETRIEVE_REQ_REFCOUNT_MASK
 *     For FFA_MEM_RETRIEVE_REQ, bit[7-0]: Number of times receiver can
 *     retrieve each memory region before relinquishing it specified as
 *     ((1U << (value + 1)) - 1 (or value = bits in reference count - 1).
 *
 * For all other bits and commands: must be 0.
 */
typedef uint32_t ffa_features3_t;
#define FFA_FEATURES3_MEM_RETRIEVE_REQ_REFCOUNT_MASK 0xffU

/**
 * SMC_FC32_FFA_MIN - First 32 bit SMC opcode reserved for FFA
 */
#define SMC_FC32_FFA_MIN FFA_FID(SMC_32, FFA_FNUM_ERROR)

/**
 * SMC_FC32_FFA_MAX - Last 32 bit SMC opcode reserved for FFA
 * For FFA version 1.0
 */
#define SMC_FC32_FFA_MAX FFA_FID(SMC_32, FFA_FNUM_MEM_FRAG_TX)

/**
 * SMC_FC64_FFA_MIN - First 64 bit SMC opcode reserved for FFA
 */
#define SMC_FC64_FFA_MIN FFA_FID(SMC_64, FFA_FNUM_ERROR)

/**
 * SMC_FC64_FFA_MAX - Last 64 bit SMC opcode reserved for FFA
 * For FFA version 1.0
 */
#define SMC_FC64_FFA_MAX FFA_FID(SMC_64, FFA_FNUM_MEM_FRAG_TX)
