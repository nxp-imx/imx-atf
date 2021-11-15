/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SEMA4_H_
#define _FSL_SEMA4_H_


/*******************************************************************************
 * #include "fsl_common.h"
 ******************************************************************************/
#define assert(condition) ((void)0) //FSY to update

/*! @brief Construct a status code value from a group and code number. */
#define MAKE_STATUS(group, code) ((((group)*100) + (code)))


typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

/*! @brief Status group numbers. */
enum _status_groups
{
    kStatusGroup_Generic = 0,                 /*!< Group number for generic status codes. */
};

/*! @brief Generic status return codes. */
enum _generic_status
{
    kStatus_Success = MAKE_STATUS(kStatusGroup_Generic, 0),
    kStatus_Fail = MAKE_STATUS(kStatusGroup_Generic, 1),
};

/*! @brief Type used for all status and error return values. */
typedef int32_t status_t;

/* IO definitions (access restrictions to peripheral registers) */
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/** SEMA4 - Register Layout Typedef */
typedef struct {
  __IO uint8_t Gate00;                             /**< Semaphores Gate 0 Register, offset: 0x0 */
  __IO uint8_t Gate01;                             /**< Semaphores Gate 1 Register, offset: 0x1 */
  __IO uint8_t Gate02;                             /**< Semaphores Gate 2 Register, offset: 0x2 */
  __IO uint8_t Gate03;                             /**< Semaphores Gate 3 Register, offset: 0x3 */
  __IO uint8_t Gate04;                             /**< Semaphores Gate 4 Register, offset: 0x4 */
  __IO uint8_t Gate05;                             /**< Semaphores Gate 5 Register, offset: 0x5 */
  __IO uint8_t Gate06;                             /**< Semaphores Gate 6 Register, offset: 0x6 */
  __IO uint8_t Gate07;                             /**< Semaphores Gate 7 Register, offset: 0x7 */
  __IO uint8_t Gate08;                             /**< Semaphores Gate 8 Register, offset: 0x8 */
  __IO uint8_t Gate09;                             /**< Semaphores Gate 9 Register, offset: 0x9 */
  __IO uint8_t Gate10;                             /**< Semaphores Gate 10 Register, offset: 0xA */
  __IO uint8_t Gate11;                             /**< Semaphores Gate 11 Register, offset: 0xB */
  __IO uint8_t Gate12;                             /**< Semaphores Gate 12 Register, offset: 0xC */
  __IO uint8_t Gate13;                             /**< Semaphores Gate 13 Register, offset: 0xD */
  __IO uint8_t Gate14;                             /**< Semaphores Gate 14 Register, offset: 0xE */
  __IO uint8_t Gate15;                             /**< Semaphores Gate 15 Register, offset: 0xF */
       uint8_t RESERVED_0[48];
  struct {                                         /* offset: 0x40, array step: 0x8 */
    __IO uint16_t CPINE;                             /**< Semaphores Processor n IRQ Notification Enable, array offset: 0x40, array step: 0x8 */
         uint8_t RESERVED_0[6];
  } CPINE[2];
       uint8_t RESERVED_1[48];
  struct {                                         /* offset: 0x80, array step: 0x8 */
    __IO uint16_t CPNTF;                             /**< Semaphores Processor n IRQ Notification, array offset: 0x80, array step: 0x8 */
         uint8_t RESERVED_0[6];
  } CPNTF[2];
       uint8_t RESERVED_2[112];
  __IO uint16_t RSTGT;                             /**< Semaphores (Secure) Reset Gate n, offset: 0x100 */
       uint8_t RESERVED_3[2];
  __IO uint16_t RSTNTF;                            /**< Semaphores (Secure) Reset IRQ Notification, offset: 0x104 */
} SEMA4_Type;


/*! @name RSTGT - Semaphores (Secure) Reset Gate n */
/*! @{ */
#define SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP_MASK    (0xFFU)
#define SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP_SHIFT   (0U)
#define SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP(x)      (((uint16_t)(((uint16_t)(x)) << SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP_SHIFT)) & SEMA4_RSTGT_RSTGSM_RSTGMS_RSTGDP_MASK)
#define SEMA4_RSTGT_RSTGTN_MASK                  (0xFF00U)
#define SEMA4_RSTGT_RSTGTN_SHIFT                 (8U)
#define SEMA4_RSTGT_RSTGTN(x)                    (((uint16_t)(((uint16_t)(x)) << SEMA4_RSTGT_RSTGTN_SHIFT)) & SEMA4_RSTGT_RSTGTN_MASK)
/*! @} */


/*!
 * @addtogroup sema4
 * @{
 */

/******************************************************************************
 * Definitions
 *****************************************************************************/

/*! @name Driver version */
/*@{*/
/*! @brief SEMA4 driver version */
#define FSL_SEMA4_DRIVER_VERSION (MAKE_VERSION(2, 0, 0))
/*@}*/

/*! @brief The number to reset all SEMA4 gates. */
#define SEMA4_GATE_NUM_RESET_ALL (64U)

/*!
 * @brief SEMA4 gate n register address.
 */
#define SEMA4_GATEn(base, n) (*(&((base)->Gate00) + (n)))

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initializes the SEMA4 module.
 *
 * This function initializes the SEMA4 module. It only enables the clock but does
 * not reset the gates because the module might be used by other processors
 * at the same time. To reset the gates, call either SEMA4_ResetGate or
 * SEMA4_ResetAllGates function.
 *
 * @param base SEMA4 peripheral base address.
 */
void SEMA4_Init(SEMA4_Type *base);

/*!
 * @brief De-initializes the SEMA4 module.
 *
 * This function de-initializes the SEMA4 module. It only disables the clock.
 *
 * @param base SEMA4 peripheral base address.
 */
void SEMA4_Deinit(SEMA4_Type *base);

/*!
 * @brief Tries to lock the SEMA4 gate.
 *
 * This function tries to lock the specific SEMA4 gate. If the gate has been
 * locked by another processor, this function returns an error code.
 *
 * @param base SEMA4 peripheral base address.
 * @param gateNum  Gate number to lock.
 * @param procNum  Current processor number.
 *
 * @retval kStatus_Success     Lock the sema4 gate successfully.
 * @retval kStatus_Fail Sema4 gate has been locked by another processor.
 */
status_t SEMA4_TryLock(SEMA4_Type *base, uint8_t gateNum, uint8_t procNum);

/*!
 * @brief Locks the SEMA4 gate.
 *
 * This function locks the specific SEMA4 gate. If the gate has been
 * locked by other processors, this function waits until it is unlocked and then
 * lock it.
 *
 * @param base SEMA4 peripheral base address.
 * @param gateNum  Gate number to lock.
 * @param procNum  Current processor number.
 */
void SEMA4_Lock(SEMA4_Type *base, uint8_t gateNum, uint8_t procNum);

/*!
 * @brief Unlocks the SEMA4 gate.
 *
 * This function unlocks the specific SEMA4 gate. It only writes unlock value
 * to the SEMA4 gate register. However, it does not check whether the SEMA4 gate is locked
 * by the current processor or not. As a result, if the SEMA4 gate is not locked by the current
 * processor, this function has no effect.
 *
 * @param base SEMA4 peripheral base address.
 * @param gateNum  Gate number to unlock.
 */
static inline void SEMA4_Unlock(SEMA4_Type *base, uint8_t gateNum)
{
    assert(gateNum < FSL_FEATURE_SEMA4_GATE_COUNT);

    SEMA4_GATEn(base, gateNum) = 0U;
}

/*!
 * @brief Gets the status of the SEMA4 gate.
 *
 * This function checks the lock status of a specific SEMA4 gate.
 *
 * @param base SEMA4 peripheral base address.
 * @param gateNum  Gate number.
 *
 * @return Return -1 if the gate is unlocked, otherwise return the
 * processor number which has locked the gate.
 */
static inline int32_t SEMA4_GetLockProc(SEMA4_Type *base, uint8_t gateNum)
{
    assert(gateNum < FSL_FEATURE_SEMA4_GATE_COUNT);

    return (SEMA4_GATEn(base, gateNum)) - 1;
}

/*!
 * @brief Resets the SEMA4 gate to an unlocked status.
 *
 * This function resets a SEMA4 gate to an unlocked status.
 *
 * @param base SEMA4 peripheral base address.
 * @param gateNum  Gate number.
 *
 * @retval kStatus_Success         SEMA4 gate is reset successfully.
 * @retval kStatus_Fail Some other reset process is ongoing.
 */
status_t SEMA4_ResetGate(SEMA4_Type *base, uint8_t gateNum);

/*!
 * @brief Resets all SEMA4 gates to an unlocked status.
 *
 * This function resets all SEMA4 gate to an unlocked status.
 *
 * @param base SEMA4 peripheral base address.
 *
 * @retval kStatus_Success         SEMA4 is reset successfully.
 * @retval kStatus_Fail Some other reset process is ongoing.
 */
static inline status_t SEMA4_ResetAllGates(SEMA4_Type *base)
{
    return SEMA4_ResetGate(base, SEMA4_GATE_NUM_RESET_ALL);
}

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */

#endif /* _FSL_SEMA4_H_ */
