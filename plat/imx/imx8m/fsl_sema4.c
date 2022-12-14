/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sema4.h"
#include "sema4.h"

/******************************************************************************
 * Definitions
 *****************************************************************************/

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.sema4"
#endif

/* The first number write to RSTGDP when reset SEMA4 gate. */
#define SEMA4_GATE_RESET_PATTERN_1 (0xE2U)
/* The second number write to RSTGDP when reset SEMA4 gate. */
#define SEMA4_GATE_RESET_PATTERN_2 (0x1DU)

/* The first number write to RSTGDP when reset SEMA4 gate IRQ notification. */
#define SEMA4_GATE_IRQ_RESET_PATTERN_1 (0x47U)
/* The second number write to RSTGDP when reset SEMA4 gate IRQ notification. */
#define SEMA4_GATE_IRQ_RESET_PATTERN_2 (0xB8U)

#define SEMA4_RSTGT_RSTNSM_MASK (0x30U)

#define SEMA4_RSTNTF_RSTNSM_MASK (0x30U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(SEMA4_CLOCKS)
/*!
 * @brief Get instance number for SEMA4 module.
 *
 * @param base SEMA4 peripheral base address.
 */
uint32_t SEMA4_GetInstance(SEMA4_Type *base);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(SEMA4_CLOCKS)
/*! @brief Pointers to sema4 bases for each instance. */
static SEMA4_Type *const s_sema4Bases[] = SEMA4_BASE_PTRS;
#endif

#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
#if defined(SEMA4_CLOCKS)
/*! @brief Pointers to sema4 clocks for each instance. */
static const clock_ip_name_t s_sema4Clocks[] = SEMA4_CLOCKS;
#endif
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

/******************************************************************************
 * CODE
 *****************************************************************************/

#if defined(SEMA4_CLOCKS)
uint32_t SEMA4_GetInstance(SEMA4_Type *base)
{
    uint32_t instance;

    /* Find the instance index from base address mappings. */
    for (instance = 0; instance < ARRAY_SIZE(s_sema4Bases); instance++)
    {
        if (s_sema4Bases[instance] == base)
        {
            break;
        }
    }

    assert(instance < ARRAY_SIZE(s_sema4Bases));

    return instance;
}
#endif

void SEMA4_Init(SEMA4_Type *base)
{
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
#if defined(SEMA4_CLOCKS)
    CLOCK_EnableClock(s_sema4Clocks[SEMA4_GetInstance(base)]);
#endif
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
}

void SEMA4_Deinit(SEMA4_Type *base)
{
#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
#if defined(SEMA4_CLOCKS)
    CLOCK_DisableClock(s_sema4Clocks[SEMA4_GetInstance(base)]);
#endif
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
}

status_t SEMA4_TryLock(SEMA4_Type *base, uint8_t gateNum, uint8_t procNum)
{
    assert(gateNum < FSL_FEATURE_SEMA4_GATE_COUNT);

    ++procNum;

    /* Try to lock. */
    SEMA4_GATEn(base, gateNum) = procNum;

    /* Check locked or not. */
    if (procNum != SEMA4_GATEn(base, gateNum))
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

void SEMA4_Lock(SEMA4_Type *base, uint8_t gateNum, uint8_t procNum)
{
    assert(gateNum < FSL_FEATURE_SEMA4_GATE_COUNT);

    ++procNum;

    while (procNum != SEMA4_GATEn(base, gateNum))
    {
        /* Wait for unlocked status. */
        while (SEMA4_GATEn(base, gateNum))
        {
        }

        /* Lock the gate. */
        SEMA4_GATEn(base, gateNum) = procNum;
    }
}

status_t SEMA4_ResetGate(SEMA4_Type *base, uint8_t gateNum)
{
    /*
     * Reset all gates if gateNum >= SEMA4_GATE_NUM_RESET_ALL
     * Reset specific gate if gateNum < FSL_FEATURE_SEMA4_GATE_COUNT
     */
    assert(!((gateNum < SEMA4_GATE_NUM_RESET_ALL) && (gateNum >= FSL_FEATURE_SEMA4_GATE_COUNT)));

    /* Check whether some reset is ongoing. */
    if (base->RSTGT & SEMA4_RSTGT_RSTNSM_MASK)
    {
        return kStatus_Fail;
    }

    /* First step. */
    base->RSTGT = SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP(SEMA4_GATE_RESET_PATTERN_1);
    /* Second step. */
    base->RSTGT = SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP(SEMA4_GATE_RESET_PATTERN_2) | SEMA4_RSTGT_RSTGTN(gateNum);

    return kStatus_Success;
}
