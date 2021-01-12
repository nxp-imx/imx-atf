/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IMX8M_I2C_H_
#define _IMX8M_I2C_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*! @brief Type used for all status and error return values. */
typedef int32_t status_t;

#define MAKE_STATUS(group, code) ((((group)*100) + (code)))

#define __IO volatile

/** I2C - Register Layout Typedef */
typedef struct {
	/**< I2C Address Register, offset: 0x0 */
	__IO uint16_t IADR;
	uint8_t RESERVED_0[2];
	/**< I2C Frequency Divider Register, offset: 0x4 */
	__IO uint16_t IFDR;
	uint8_t RESERVED_1[2];
	/**< I2C Control Register, offset: 0x8 */
	__IO uint16_t I2CR;
	uint8_t RESERVED_2[2];
	/**< I2C Status Register, offset: 0xC */
	__IO uint16_t I2SR;
	uint8_t RESERVED_3[2];
	/**< I2C Data I/O Register, offset: 0x10 */
	__IO uint16_t I2DR;
} SA_I2C_Type;

/* -------------------------------------------------------------------------
 * -- I2C Register Masks
 * -------------------------------------------------------------------------
 */

/*!
 * @addtogroup I2C_Register_Masks I2C Register Masks
 * @{
 */

/*! @name IADR - I2C Address Register */
/*! @{ */
#define I2C_IADR_ADR_MASK	(0xFEU)
#define I2C_IADR_ADR_SHIFT	(1U)
#define I2C_IADR_ADR(x)		(((uint16_t)(((uint16_t)(x)) << I2C_IADR_ADR_SHIFT)) & I2C_IADR_ADR_MASK)
/*! @} */

/*! @name IFDR - I2C Frequency Divider Register */
/*! @{ */
#define I2C_IFDR_IC_MASK	(0x3FU)
#define I2C_IFDR_IC_SHIFT	(0U)
#define I2C_IFDR_IC(x)		(((uint16_t)(((uint16_t)(x)) << I2C_IFDR_IC_SHIFT)) & I2C_IFDR_IC_MASK)
/*! @} */

/*! @name I2CR - I2C Control Register */
/*! @{ */
#define I2C_I2CR_RSTA_MASK	(0x4U)
#define I2C_I2CR_RSTA_SHIFT	(2U)
/*! RSTA
 *  0b0..No repeat start
 *  0b1..Generates a Repeated Start condition
 */
#define I2C_I2CR_RSTA(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_RSTA_SHIFT)) & I2C_I2CR_RSTA_MASK)
#define I2C_I2CR_TXAK_MASK	(0x8U)
#define I2C_I2CR_TXAK_SHIFT	(3U)
/*! TXAK
 *  0b0..An acknowledge signal is sent to the bus at the ninth clock bit after
 *  receiving one byte of data.
 *  0b1..No acknowledge signal response is sent (that is, the acknowledge
 *  bit = 1).
 */
#define I2C_I2CR_TXAK(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_TXAK_SHIFT)) & I2C_I2CR_TXAK_MASK)
#define I2C_I2CR_MTX_MASK	(0x10U)
#define I2C_I2CR_MTX_SHIFT	(4U)
/*! MTX
 *  0b0..Receive.When a slave is addressed, the software should set MTX
 *  according to the slave read/write bit in the I2C status
 *  register (I2C_I2SR[SRW]).
 *  0b1..Transmit.In Master mode, MTX should be set according to the type of
 *  transfer required. Therefore, for address cycles, MTX is always 1.
 */
#define I2C_I2CR_MTX(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_MTX_SHIFT)) & I2C_I2CR_MTX_MASK)
#define I2C_I2CR_MSTA_MASK	(0x20U)
#define I2C_I2CR_MSTA_SHIFT	(5U)
/*! MSTA
 *  0b0..Slave mode. Changing MSTA from 1 to 0 generates a Stop and selects
 *  Slave mode.
 *  0b1..Master mode. Changing MSTA from 0 to 1 signals a Start on the bus and
 *  selects Master mode.
 */
#define I2C_I2CR_MSTA(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_MSTA_SHIFT)) & I2C_I2CR_MSTA_MASK)
#define I2C_I2CR_IIEN_MASK	(0x40U)
#define I2C_I2CR_IIEN_SHIFT	(6U)
/*! IIEN
 *  0b0..I2C interrupts are disabled, but the status flag I2C_I2SR[IIF]
 *  continues to be set when an Interrupt condition occurs.
 *  0b1..I2C interrupts are enabled. An I2C interrupt occurs if
 *  I2C_I2SR[IIF] is also set.
 */
#define I2C_I2CR_IIEN(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_IIEN_SHIFT)) & I2C_I2CR_IIEN_MASK)
#define I2C_I2CR_IEN_MASK	(0x80U)
#define I2C_I2CR_IEN_SHIFT	(7U)
/*! IEN
 *  0b0..The block is disabled, but registers can still be accessed.
 *  0b1..The I2C is enabled. This bit must be set before any other
 *  I2C_I2CR bits have an effect.
 */
#define I2C_I2CR_IEN(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2CR_IEN_SHIFT)) & I2C_I2CR_IEN_MASK)
/*! @} */

/*! @name I2SR - I2C Status Register */
/*! @{ */
#define I2C_I2SR_RXAK_MASK	(0x1U)
#define I2C_I2SR_RXAK_SHIFT	(0U)
/*! RXAK
 *  0b0..An "acknowledge" signal was received after the completion of
 *  an 8-bit data transmission on the bus.
 *  0b1..A "No acknowledge" signal was detected at the ninth clock.
 */
#define I2C_I2SR_RXAK(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_RXAK_SHIFT)) & I2C_I2SR_RXAK_MASK)
#define I2C_I2SR_IIF_MASK	(0x2U)
#define I2C_I2SR_IIF_SHIFT	(1U)
/*! IIF
 *  0b0..No I2C interrupt pending.
 *  0b1..An interrupt is pending.This causes a processor interrupt request
 *  (if the interrupt enable is asserted [IIEN = 1]).
 *  The interrupt is set when one of the following occurs:
 *  One byte transfer is completed (the interrupt is set at the falling edge of
 *  the ninth clock). An address is received that matches its own specific address in Slave Receive mode. Arbitration is lost.
 */
#define I2C_I2SR_IIF(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_IIF_SHIFT)) & I2C_I2SR_IIF_MASK)
#define I2C_I2SR_SRW_MASK	(0x4U)
#define I2C_I2SR_SRW_SHIFT	(2U)
/*! SRW
 *  0b0..Slave receive, master writing to slave
 *  0b1..Slave transmit, master reading from slave
 */
#define I2C_I2SR_SRW(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_SRW_SHIFT)) & I2C_I2SR_SRW_MASK)
#define I2C_I2SR_IAL_MASK	(0x10U)
#define I2C_I2SR_IAL_SHIFT	(4U)
/*! IAL
 *  0b0..No arbitration lost.
 *  0b1..Arbitration is lost.
 */
#define I2C_I2SR_IAL(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_IAL_SHIFT)) & I2C_I2SR_IAL_MASK)
#define I2C_I2SR_IBB_MASK	(0x20U)
#define I2C_I2SR_IBB_SHIFT	(5U)
/*! IBB
 *  0b0..Bus is idle. If a Stop signal is detected, IBB is cleared.
 *  0b1..Bus is busy. When Start is detected, IBB is set.
 */
#define I2C_I2SR_IBB(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_IBB_SHIFT)) & I2C_I2SR_IBB_MASK)
#define I2C_I2SR_IAAS_MASK	(0x40U)
#define I2C_I2SR_IAAS_SHIFT	(6U)
/*! IAAS
 *  0b0..Not addressed
 *  0b1..Addressed as a slave. Set when its own address (I2C_IADR) matches the
 *  calling address.
 */
#define I2C_I2SR_IAAS(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_IAAS_SHIFT)) & I2C_I2SR_IAAS_MASK)
#define I2C_I2SR_ICF_MASK	(0x80U)
#define I2C_I2SR_ICF_SHIFT	(7U)
/*! ICF
 *  0b0..Transfer is in progress.
 *  0b1..Transfer is complete. This bit is set by the falling edge of the
 *  ninth clock of the last byte transfer.
 */
#define I2C_I2SR_ICF(x)		(((uint16_t)(((uint16_t)(x)) << I2C_I2SR_ICF_SHIFT)) & I2C_I2SR_ICF_MASK)
/*! @} */

/*! @name I2DR - I2C Data I/O Register */
/*! @{ */
#define I2C_I2DR_DATA_MASK	(0xFFU)
#define I2C_I2DR_DATA_SHIFT	(0U)
#define I2C_I2DR_DATA(x)	(((uint16_t)(((uint16_t)(x)) << I2C_I2DR_DATA_SHIFT)) & I2C_I2DR_DATA_MASK)
/*! @} */


/*!
 * @}
 */ /* end of group I2C_Register_Masks */


/* I2C - Peripheral instance base addresses */
/** Peripheral I2C1 base address */
#define I2C1_BASE	(0x30A20000u)
/** Peripheral I2C1 base pointer */
#define I2C1		((SA_I2C_Type *)I2C1_BASE)
/** Peripheral I2C2 base address */
#define I2C2_BASE	(0x30A30000u)
/** Peripheral I2C2 base pointer */
#define I2C2		((SA_I2C_Type *)I2C2_BASE)
/** Peripheral I2C3 base address */
#define I2C3_BASE	(0x30A40000u)
/** Peripheral I2C3 base pointer */
#define I2C3		((SA_I2C_Type *)I2C3_BASE)
/** Peripheral I2C4 base address */
#define I2C4_BASE	(0x30A50000u)
/** Peripheral I2C4 base pointer */
#define I2C4		((SA_I2C_Type *)I2C4_BASE)
/** Array initializer of I2C peripheral base addresses */
#define I2C_BASE_ADDRS	{ 0u, I2C1_BASE, I2C2_BASE, I2C3_BASE, I2C4_BASE }
/** Array initializer of I2C peripheral base pointers */
#define I2C_BASE_PTRS	{ (SA_I2C_Type *)0u, I2C1, I2C2, I2C3, I2C4 }
/** Interrupt vectors for the I2C peripheral type */
#define I2C_IRQS	{ NotAvail_IRQn, I2C1_IRQn, I2C2_IRQn, I2C3_IRQn, I2C4_IRQn }


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief  I2C status return codes. */
enum _i2c_status {
	/*!< I2C is busy with current transfer. */
	kStatus_I2C_Busy = MAKE_STATUS(11, 0),
	/*!< Bus is Idle. */
	kStatus_I2C_Idle = MAKE_STATUS(11, 1),
	/*!< NAK received during transfer. */
	kStatus_I2C_Nak = MAKE_STATUS(11, 2),
	/*!< Arbitration lost during transfer. */
	kStatus_I2C_ArbitrationLost = MAKE_STATUS(11, 3),
	/*!< Timeout poling status flags. */
	kStatus_I2C_Timeout = MAKE_STATUS(11, 4),
	/*!< NAK received during the address probe. */
	kStatus_I2C_Addr_Nak = MAKE_STATUS(11, 5),
};

/*!
 * @brief I2C peripheral flags
 *
 * The following status register flags can be cleared:
 * - #kI2C_ArbitrationLostFlag
 * - #kI2C_IntPendingFlag
 *
 * @note These enumerations are meant to be OR'd together to form a bit mask.
 *
 */
enum _i2c_flags {
	/*!< I2C receive NAK flag. */
	kI2C_ReceiveNakFlag = I2C_I2SR_RXAK_MASK,
	/*!< I2C interrupt pending flag. */
	kI2C_IntPendingFlag = I2C_I2SR_IIF_MASK,
	/*!< I2C transfer direction flag. */
	kI2C_TransferDirectionFlag = I2C_I2SR_SRW_MASK,
	/*!< I2C arbitration lost flag. */
	kI2C_ArbitrationLostFlag = I2C_I2SR_IAL_MASK,
	/*!< I2C bus busy flag. */
	kI2C_BusBusyFlag = I2C_I2SR_IBB_MASK,
	/*!< I2C address match flag. */
	kI2C_AddressMatchFlag = I2C_I2SR_IAAS_MASK,
	/*!< I2C transfer complete flag. */
	kI2C_TransferCompleteFlag = I2C_I2SR_ICF_MASK,
};

/*! @brief I2C feature interrupt source. */
enum _i2c_interrupt_enable {
	/*!< I2C global interrupt. */
	kI2C_GlobalInterruptEnable = I2C_I2CR_IIEN_MASK,
};

/*! @brief The direction of master and slave transfers. */
typedef enum _i2c_direction {
	kI2C_Write = 0x0U, /*!< Master transmits to the slave. */
	kI2C_Read = 0x1U,  /*!< Master receives from the slave. */
} i2c_direction_t;

/*! @brief I2C transfer control flag. */
enum _i2c_master_transfer_flags {
	/*!< A transfer starts with a start signal, stops with a stop signal. */
	kI2C_TransferDefaultFlag = 0x0U,
	/*!< A transfer starts without a start signal, only support write only
	 * or write+read with no start flag, do not support read only with no
	 * start flag.
	 */
	kI2C_TransferNoStartFlag = 0x1U,

	/*!< A transfer starts with a repeated start signal. */
	kI2C_TransferRepeatedStartFlag = 0x2U,
	/*!< A transfer ends without a stop signal. */
	kI2C_TransferNoStopFlag = 0x4U,
};

/*! @brief I2C master user configuration. */
typedef struct _i2c_master_config {
	/*!< Enables the I2C peripheral at initialization time. */
	bool enableMaster;
	/*!< Baud rate configuration of I2C peripheral. */
	uint32_t baudRate_Bps;
} i2c_master_config_t;

/*! @brief I2C master handle typedef. */
typedef struct _i2c_master_handle i2c_master_handle_t;

/*! @brief I2C master transfer callback typedef. */
typedef void (*i2c_master_transfer_callback_t)(SA_I2C_Type *base,
					       i2c_master_handle_t *handle,
					       status_t status,
					       void *userData);

/*! @brief I2C master transfer structure. */
typedef struct _i2c_master_transfer {
	/*!< A transfer flag which controls the transfer. */
	uint32_t flags;
	/*!< 7-bit slave address. */
	uint8_t slaveAddress;
	/*!< A transfer direction, read or write. */
	i2c_direction_t direction;
	/*!< A sub address. Transferred MSB first. */
	uint32_t subaddress;
	/*!< A size of the command buffer. */
	uint8_t subaddressSize;
	/*!< A transfer buffer. */
	uint8_t *volatile data;
	/*!< A transfer size. */
	volatile size_t dataSize;
} i2c_master_transfer_t;

/*! @brief I2C master handle structure. */
struct _i2c_master_handle {
	/*!< I2C master transfer copy. */
	i2c_master_transfer_t transfer;
	/*!< Total bytes to be transferred. */
	size_t transferSize;
	/*!< A transfer state maintained during transfer. */
	uint8_t state;
	/*!< A callback function called when the transfer is finished. */
	i2c_master_transfer_callback_t completionCallback;
	/*!< A callback parameter passed to the callback function. */
	void *userData;
};

/*****************************************************************************
 * API
 *****************************************************************************
 */

#if defined(__cplusplus)
extern "C" {
#endif /*_cplusplus. */

/*!
 * @name Initialization and deinitialization
 * @{
 */

/*!
 * @brief Initializes the I2C peripheral. Call this API to ungate the I2C clock
 * and configure the I2C with master configuration.
 *
 * @note This API should be called at the beginning of the application.
 * Otherwise, any operation to the I2C module can cause a hard fault
 * because the clock is not enabled. The configuration structure can be custom
 * filled or it can be set with default values by using the
 * I2C_MasterGetDefaultConfig().
 * After calling this API, the master is ready to transfer.
 * This is an example.
 * @code
 * i2c_master_config_t config = {
 * .enableMaster = true,
 * .baudRate_Bps = 100000
 * };
 * I2C_MasterInit(I2C0, &config, 12000000U);
 * @endcode
 *
 * @param base I2C base pointer
 * @param masterConfig A pointer to the master configuration structure
 * @param srcClock_Hz I2C peripheral clock frequency in Hz
 */
void I2C_MasterInit(SA_I2C_Type *base, const i2c_master_config_t *masterConfig,
	uint32_t srcClock_Hz);

/*!
 * @brief De-initializes the I2C master peripheral.
 *        Call this API to gate the I2C clock.
 *        The I2C master module can't work unless the I2C_MasterInit is called.
 * @param base I2C base pointer
 */
void I2C_MasterDeinit(SA_I2C_Type *base);

/*!
 * @name Status
 * @{
 */

/*!
 * @brief Gets the I2C status flags.
 *
 * @param base I2C base pointer
 * @return status flag, use status flag to AND #_i2c_flags to get
 *         the related status.
 */
static inline uint32_t I2C_MasterGetStatusFlags(SA_I2C_Type *base)
{
	return base->I2SR;
}

/*!
 * @brief Clears the I2C status flag state.
 *
 * The following status register flags can be cleared kI2C_ArbitrationLostFlag
 * and kI2C_IntPendingFlag.
 *
 * @param base I2C base pointer
 * @param statusMask The status flag mask, defined in type i2c_status_flag_t.
 *      The parameter can be any combination of the following values:
 *          @arg kI2C_ArbitrationLostFlag
 *          @arg kI2C_IntPendingFlagFlag
 */
static inline void I2C_MasterClearStatusFlags(SA_I2C_Type *base,
	uint32_t statusMask)
{
	base->I2SR = (uint8_t)statusMask;
}

/*!
 * @brief Sends a START on the I2C bus.
 *
 * This function is used to initiate a new master mode transfer by sending
 * the START signal.
 * The slave address is sent following the I2C START signal.
 *
 * @param base I2C peripheral base pointer
 * @param address 7-bit slave device address.
 * @param direction Master transfer directions(transmit/receive).
 * @retval kStatus_Success Successfully send the start signal.
 * @retval kStatus_I2C_Busy Current bus is busy.
 */
status_t I2C_MasterStart(SA_I2C_Type *base, uint8_t address,
	i2c_direction_t direction);

/*!
 * @brief Sends a STOP signal on the I2C bus.
 *
 * @retval kStatus_Success Successfully send the stop signal.
 * @retval kStatus_I2C_Timeout Send stop signal failed, timeout.
 */
status_t I2C_MasterStop(SA_I2C_Type *base);

/*!
 * @brief Sends a REPEATED START on the I2C bus.
 *
 * @param base I2C peripheral base pointer
 * @param address 7-bit slave device address.
 * @param direction Master transfer directions(transmit/receive).
 * @retval kStatus_Success Successfully send the start signal.
 * @retval kStatus_I2C_Busy Current bus is busy but not occupied by
 *                          current I2C master.
 */
status_t I2C_MasterRepeatedStart(SA_I2C_Type *base, uint8_t address,
	i2c_direction_t direction);

/*!
 * @brief Performs a polling send transaction on the I2C bus.
 *
 * @param base  The I2C peripheral base pointer.
 * @param txBuff The pointer to the data to be transferred.
 * @param txSize The length in bytes of the data to be transferred.
 * @param flags Transfer control flag to decide whether need to send a stop,
 *              use kI2C_TransferDefaultFlag to issue a stop and
 *              kI2C_TransferNoStop to not send a stop.
 * @retval kStatus_Success Successfully complete the data transmission.
 * @retval kStatus_I2C_ArbitrationLost Transfer error, arbitration lost.
 * @retval kStataus_I2C_Nak Transfer error, receive NAK during transfer.
 */
status_t I2C_MasterWriteBlocking(SA_I2C_Type *base, const uint8_t *txBuff,
	size_t txSize, uint32_t flags);

/*!
 * @brief Performs a polling receive transaction on the I2C bus.
 *
 * @note The I2C_MasterReadBlocking function stops the bus before reading the
 *       final byte. Without stopping the bus prior for the final read,
 *       the bus issues another read, resulting in garbage data being read
 *       into the data register.
 *
 * @param base I2C peripheral base pointer.
 * @param rxBuff The pointer to the data to store the received data.
 * @param rxSize The length in bytes of the data to be received.
 * @param flags Transfer control flag to decide whether need to send a stop,
 *              use kI2C_TransferDefaultFlag
 *              to issue a stop and kI2C_TransferNoStop to not send a stop.
 * @retval kStatus_Success Successfully complete the data transmission.
 * @retval kStatus_I2C_Timeout Send stop signal failed, timeout.
 */
status_t I2C_MasterReadBlocking(SA_I2C_Type *base, uint8_t *rxBuff,
	size_t rxSize, uint32_t flags);

/*!
 * @brief Performs a master polling transfer on the I2C bus.
 *
 * @note The API does not return until the transfer succeeds or fails due
 * to arbitration lost or receiving a NAK.
 *
 * @param base I2C base pointer.
 * @param slaveAddress
 * @param subaddress register address mapping in i2C slave component
 * @param data pointer to data buffer
 * @param direction kI2C_Read | kI2C_Write
 */
status_t I2C_MasterTransferBlocking(SA_I2C_Type *base, uint8_t slaveAddress,
	uint8_t subaddress, uint8_t *data, i2c_direction_t direction);
/*!
 * @I2C Master Init.
 *
 * @param base I2C base pointer.
 */

void I2C_Init(SA_I2C_Type *base);


status_t I2C_Send(uint8_t slaveAddress, uint8_t subaddress, uint8_t byte);

status_t I2C_Receive(uint8_t slaveAddress, uint8_t subaddress, uint8_t *byte);

/*!
 * @brief Save I2C registers
 *
 * @param base I2C peripheral base pointer.
 * @param i2cSave pointer where I2C register content shall be stored.
 */
static inline void saveI2C_Regs(SA_I2C_Type *base, SA_I2C_Type *i2cSave)
{
	/* Save all registers */
	i2cSave->IADR = base->IADR;
	i2cSave->IFDR = base->IFDR;
	i2cSave->I2CR = base->I2CR;
	i2cSave->I2SR = base->I2SR;
}

/*!
 * @brief Restore I2C registers
 *
 * @param base I2C peripheral base pointer.
 * @param i2cSave pointer from where I2C register content shall be restored.
 */
static inline void restoreI2C_Regs(SA_I2C_Type *base, SA_I2C_Type *i2cSave)
{
	/* Restore all registers */
	base->IADR = i2cSave->IADR;
	base->IFDR = i2cSave->IFDR;
	base->I2CR = i2cSave->I2CR;
	base->I2SR = i2cSave->I2SR;
}

/* @} */
#if defined(__cplusplus)
}
#endif /*_cplusplus. */
/*@}*/

#endif /* _FSL_I2C_H_*/
