/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "imx8_i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Common sets of flags used by the driver. */
enum _i2c_flag_constants {
/*! All flags which are cleared by the driver upon starting a transfer. */
	kClearFlags = kI2C_ArbitrationLostFlag | kI2C_IntPendingFlag,
	kIrqFlags = kI2C_GlobalInterruptEnable,

};

/*******************************************************************************
 * Codes
 ******************************************************************************/

void I2C_Init(SA_I2C_Type *base)
{
	/* Reset the module. */
	base->IADR = 0;
	base->I2CR = 0;
	base->I2SR = 0;
	/* Disable I2C prior to configuring it. */
	base->I2CR &= ~(I2C_I2CR_IEN_MASK);
	/* Clear all flags. */
	I2C_MasterClearStatusFlags(base, kClearFlags);
	/* Set frequency register */
	base->IFDR = 0xf;
	/* Enable the I2C peripheral based on the configuration. */
	base->I2CR = I2C_I2CR_IEN(1);
}

static status_t I2C_WaitForStatusReady(SA_I2C_Type *base, uint8_t statusFlag)
{
	uint32_t waitTimes = 500;

	/* Wait for TCF bit and manually trigger tx interrupt. */
	while ((!(base->I2SR & statusFlag)) && (--waitTimes))
		;

	if (waitTimes == 0)
		return kStatus_I2C_Timeout;
	return 0;
}

static status_t I2C_CheckAndClearError(SA_I2C_Type *base, uint32_t status)
{
	status_t result = 0;

	/* Check arbitration lost. */
	if (status & kI2C_ArbitrationLostFlag) {
		/* Clear arbitration lost flag. */
		base->I2SR &= (uint8_t) (~kI2C_ArbitrationLostFlag);

		/* Reset I2C controller*/
		base->I2CR &= ~I2C_I2CR_IEN_MASK;
		base->I2CR |= I2C_I2CR_IEN_MASK;

		result = kStatus_I2C_ArbitrationLost;
	} else if (status & kI2C_ReceiveNakFlag) { /* Check NAK */
		result = kStatus_I2C_Nak;
	} else {
	}

	return result;
}

status_t I2C_MasterStart(SA_I2C_Type *base, uint8_t address,
	i2c_direction_t direction)
{
	status_t result = 0;
	uint32_t statusFlags = I2C_MasterGetStatusFlags(base);

	/* Return an error if the bus is already in use. */
	if (statusFlags & kI2C_BusBusyFlag) {
		result = kStatus_I2C_Busy;
	} else {
		/* Send the START signal. */
		base->I2CR |= I2C_I2CR_MSTA_MASK | I2C_I2CR_MTX_MASK;
		base->I2DR = (((uint32_t) address) << 1U |
			((direction == kI2C_Read) ? 1U : 0U));
	}

	return result;
}

status_t I2C_MasterRepeatedStart(SA_I2C_Type *base, uint8_t address,
	i2c_direction_t direction)
{
	status_t result = 0;
	uint32_t statusFlags = I2C_MasterGetStatusFlags(base);

	/* Return an error if the bus is already in use, but not by us. */
	if ((statusFlags & kI2C_BusBusyFlag) &&
		((base->I2CR & I2C_I2CR_MSTA_MASK) == 0)) {
		result = kStatus_I2C_Busy;
	} else {
		/* We are already in a transfer, so send a repeated start. */
		base->I2CR |= I2C_I2CR_RSTA_MASK | I2C_I2CR_MTX_MASK;
		base->I2DR = (((uint32_t) address) << 1U |
			((direction == kI2C_Read) ? 1U : 0U));
	}

	return result;
}

status_t I2C_MasterStop(SA_I2C_Type *base)
{
	/* Issue the STOP command on the bus. */
	base->I2CR &= ~(I2C_I2CR_MSTA_MASK |
		I2C_I2CR_MTX_MASK |
		I2C_I2CR_TXAK_MASK);

	return I2C_WaitForStatusReady(base, kI2C_BusBusyFlag);
}


status_t I2C_MasterWrite(SA_I2C_Type *base, const uint8_t txBuff)
{
	status_t result = 0;
	uint8_t statusFlags = 0;

	if (I2C_WaitForStatusReady(base, kI2C_TransferCompleteFlag) != 0)
		return kStatus_I2C_Timeout;

	/* Clear the IICIF flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

	/* Setup the I2C peripheral to transmit data. */
	base->I2CR |= I2C_I2CR_MTX_MASK;

	/* Send a byte of data. */
	base->I2DR = txBuff;

	if (I2C_WaitForStatusReady(base, kI2C_IntPendingFlag) != 0)
		return kStatus_I2C_Timeout;

	statusFlags = base->I2SR;

	/* Clear the IICIF flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

	/*
	 * Check if arbitration lost or no acknowledgement (NAK),
	 * return failure status.
	 */
	if (statusFlags & kI2C_ArbitrationLostFlag) {
		base->I2SR = kI2C_ArbitrationLostFlag;
		result = kStatus_I2C_ArbitrationLost;
	}

	/* Clear the IICIF flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

	/* Send stop. */
	result = I2C_MasterStop(base);

	return result;
}

status_t I2C_MasterRead(SA_I2C_Type *base, uint8_t *rxBuff)
{
	status_t result = 0;
	volatile uint8_t dummy = 0;

	/* Add this to avoid build warning. */
	dummy++;

	if (I2C_WaitForStatusReady(base, kI2C_TransferCompleteFlag) != 0)
		return kStatus_I2C_Timeout;

	/* Clear the IICIF flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

	/* Setup the I2C peripheral to receive data. */
	base->I2CR &= ~(I2C_I2CR_MTX_MASK | I2C_I2CR_TXAK_MASK);

	/* Issue NACK on read. */
	base->I2CR |= I2C_I2CR_TXAK_MASK;

	/* Do dummy read. */
	dummy = base->I2DR;

	if (I2C_WaitForStatusReady(base, kI2C_IntPendingFlag) != 0)
		return kStatus_I2C_Timeout;

	/* Clear the IICIF flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

	/* Single byte use case. */
	/* Issue STOP command before reading last byte. */
	result = I2C_MasterStop(base);

	/* Read from the data register. */
	*rxBuff = base->I2DR;

	return result;
}

status_t I2C_MasterTransferBlocking(SA_I2C_Type *base, uint8_t slaveAddress,
	uint8_t subaddress, uint8_t *data, i2c_direction_t direction)
{
	status_t result = 0;

	/* Clear all status before transfer. */
	I2C_MasterClearStatusFlags(base, kClearFlags);

	if (I2C_WaitForStatusReady(base, kI2C_TransferCompleteFlag) != 0)
		return kStatus_I2C_Timeout;

	result = I2C_MasterStart(base, slaveAddress, kI2C_Write);
	/* Return if error. */
	if (result)
		return result;

	if (I2C_WaitForStatusReady(base, kI2C_IntPendingFlag) != 0)
		return kStatus_I2C_Timeout;

	/* Check if there's transfer error. */
	result = I2C_CheckAndClearError(base, base->I2SR);

	/* Return if error. */
	if (result) {
		if (result == kStatus_I2C_Nak) {
			result = kStatus_I2C_Addr_Nak;
			I2C_MasterStop(base);
		}

		return result;
	}

	/* Send subaddress. */
	/* Clear interrupt pending flag. */
	base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;
	base->I2DR = subaddress;

	if (I2C_WaitForStatusReady(base, kI2C_IntPendingFlag) != 0)
		return kStatus_I2C_Timeout;

	/* Check if there's transfer error. */
	result = I2C_CheckAndClearError(base, base->I2SR);

	if (result) {
		if (result == kStatus_I2C_Nak)
			I2C_MasterStop(base);

		return result;
	}

	if (direction == kI2C_Read) {
		/* Clear pending flag. */
		base->I2SR &= (uint8_t) ~kI2C_IntPendingFlag;

		/* Send repeated start and slave address. */
		result = I2C_MasterRepeatedStart(base, slaveAddress, kI2C_Read);

		/* Return if error. */
		if (result)
			return result;

		if (I2C_WaitForStatusReady(base, kI2C_IntPendingFlag) != 0)
			return kStatus_I2C_Timeout;

		/* Check if there's transfer error. */
		result = I2C_CheckAndClearError(base, base->I2SR);

		if (result) {
			if (result == kStatus_I2C_Nak) {
				result = kStatus_I2C_Addr_Nak;
				I2C_MasterStop(base);
			}
			return result;
		}
	}


	/* Transmit data. */
	if (direction == kI2C_Write) {
		/* Send Data. */
		result = I2C_MasterWrite(base, *data);
	} else {
		/* Receive Data. */
		result = I2C_MasterRead(base, data);
	}

	return result;
}

status_t I2C_Send(uint8_t slaveAddress, uint8_t subaddress, uint8_t byte)
{
	status_t status = I2C_MasterTransferBlocking((SA_I2C_Type *) 0x30a20000,
		slaveAddress, subaddress, &byte, kI2C_Write);

	while (status != 0) {
		I2C_Init((SA_I2C_Type *) 0x30a20000);
		status = I2C_MasterTransferBlocking(
			(SA_I2C_Type *) 0x30a20000, slaveAddress, subaddress,
			&byte, kI2C_Write);
	}

	return 0;
}

status_t I2C_Receive(uint8_t slaveAddress, uint8_t subaddress, uint8_t *byte)
{
	status_t status = I2C_MasterTransferBlocking(
		(SA_I2C_Type *) 0x30a20000, slaveAddress, subaddress,
		byte, kI2C_Read);

	while (status != 0) {
		I2C_Init((SA_I2C_Type *) 0x30a20000);
		status = I2C_MasterTransferBlocking(
			(SA_I2C_Type *) 0x30a20000, slaveAddress,
			subaddress, byte, kI2C_Read);
	}

	return 0;
}
