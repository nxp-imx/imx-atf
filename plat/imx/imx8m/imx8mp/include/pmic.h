/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __IMX_PMIC_H__
#define __IMX_PMIC_H__

#include <stdint.h>
#include "imx8_i2c.h"

#define PCA9450_ADD		0x25
#define PCA9450_BUCK1OUT_DVS0	0x11

/*!
 * @brief Power Down selected PMIC regulator
 *
 * @param buckAdd PMIC regulator I2C address
 */
static inline void VDD_SOC_LOW(void)
{
	//I2C_Send(PCA9450_ADD, PCA9450_BUCK1OUT_DVS0, 0xC /*0.750v*/);
	//I2C_Send(PCA9450_ADD, PCA9450_BUCK1OUT_DVS0, 0x10 /*0.800v*/);
	I2C_Send(PCA9450_ADD, PCA9450_BUCK1OUT_DVS0, 0x14 /*0.850v*/);
}

/*!
 * @brief Power Up selected PMIC regulator
 *
 * @param buckAdd PMIC regulator I2C address
 */
static inline void VDD_SOC_HIGH(void)
{
	//I2C_Send(PCA9450_ADD,  PCA9450_BUCK1OUT_DVS0, 0x1C /*0.950v*/);
	I2C_Send(PCA9450_ADD, PCA9450_BUCK1OUT_DVS0, 0x14 /*0.850v*/);
}

#endif /* __IMX_PMIC_H__ */
