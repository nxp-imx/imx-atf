/*
 * Copyright 2020 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>

#include "upower_soc_defs.h"
#include "upower_api.h"
#include "upower_defs.h"

#define UPOWER_AP_MU1_ADDR	0x29280000

extern void upwr_txrx_isr();

struct MU_tag *muptr = (struct MU_tag *)UPOWER_AP_MU1_ADDR;

void upower_apd_inst_isr()
{
	INFO("%s: entry\n", __func__);
}

uint32_t upower_status(int status)
{
    uint32_t ret = -1;
    switch(status) {
        case 0:
            NOTICE("%s: finished successfully!\n", __func__);
            ret = 0;
            break;
        case -1:
            NOTICE("%s: memory allocation or resource failed!\n", __func__);
            break;
        case -2:
            NOTICE("%s: invalid argument!\n", __func__);
            break;
        case -3:
            NOTICE("%s: called in an invalid API state!\n", __func__);
            break;
        default:
            NOTICE("%s: invalid return status\n", __func__);
            break;
    }
    return ret;
}


void upower_wait_resp()
{
	while(muptr->SR.B.RFP == 0) {
		NOTICE("%s: poll the mu:%x\n", __func__, muptr->SR.R);
		udelay(100);
	}
	upwr_txrx_isr();
}

void user_upwr_rdy_callb(uint32_t soc, uint32_t vmajor, uint32_t vminor)
{
	NOTICE("%s: soc=%x\n", __func__, soc);
	NOTICE("%s: RAM version:%d.%d\n", __func__, vmajor, vminor);
}

uint32_t upower_init(void)
{
	int status;

	INFO("%s: entry\n",  __func__);

	status = upwr_init(APD_DOMAIN, muptr, NULL, NULL, upower_apd_inst_isr, NULL);
	if (upower_status(status)) {
		NOTICE("%s: upower init failure\n", __func__);
		return -EINVAL;
	}

        NOTICE("%s: start uPower RAM service\n", __func__);
        status = upwr_start(1, user_upwr_rdy_callb);
        upower_wait_resp();
	/* poll status */
        if (upower_status(status)) {
            NOTICE("%s: upower init failure\n", __func__);
	    return status;
        }

	/* Only for test */
#if 1
	uint32_t swt;
	int ret_val, ret;

	swt = BIT_32(1);

	ret = upwr_pwm_power_off(&swt, NULL, NULL);
	if (ret) {
		NOTICE("%s failed: ret: %d\n", __func__, ret);
		return ret;
	}
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		NOTICE("Faliure %d, %s\n", ret, __func__);
//		if (ret == UPWR_REQ_BUSY)
//			return -EBUSY;
//		else
//			return -EINVAL;
	}
	NOTICE("=== %x\n", mmio_read_32(0x283590f4));

	ret = upwr_pwm_power_on(&swt, NULL, NULL);
	if (ret) {
		NOTICE("%s failed: ret: %d\n", __func__, ret);
		return ret;
	}
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		NOTICE("Faliure %d, %s\n", ret, __func__);
//		if (ret == UPWR_REQ_BUSY)
//			return -EBUSY;
//		else
//			return -EINVAL;
	}

	NOTICE("=== %x\n", mmio_read_32(0x283590f4));

	ret = upwr_pwm_power_off(&swt, NULL, NULL);
	if (ret) {
		NOTICE("%s failed: ret: %d\n", __func__, ret);
		return ret;
	}
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		NOTICE("Faliure %d, %s\n", ret, __func__);
//		if (ret == UPWR_REQ_BUSY)
//			return -EBUSY;
//		else
//			return -EINVAL;
	}
	NOTICE("=== %x\n", mmio_read_32(0x283590f4));
#endif

#define mmio_setbits32(addr, set)		mmio_write_32(addr, mmio_read_32(addr) | (set))
#define mmio_clrbits32(addr, clear)		mmio_write_32(addr, mmio_read_32(addr) & ~(clear))
	mmio_setbits32(0x2da50008, BIT_32(19) | BIT_32(17) | BIT_32(18));
	mmio_clrbits32(0x2da50008, BIT_32(16));
	printf("xxxxxx %x\n", mmio_read_32(0x2da50008));
extern int xrdc_config_mrc11_hifi_itcm(void);
extern int xrdc_config_mrc11_hifi_dtcm(void);
extern int xrdc_config_pdac(uint32_t, uint32_t, uint32_t, uint32_t);
extern int xrdc_config_mrc6_dma2_ddr(void);
#if 1
	xrdc_config_pdac(3, 47, 0, 7);
	xrdc_config_mrc11_hifi_itcm();
	xrdc_config_mrc11_hifi_dtcm();
	xrdc_config_pdac(4, 8, 1, 7); /* DMA1 (DID=1) access to SAI4 regs */
	xrdc_config_pdac(4, 9, 1, 7); /* DMA1 (DID=1) access to SAI5 regs */
	xrdc_config_pdac(5, 41, 0, 7); /* (DID=0) access to SAI6 regs */
	xrdc_config_pdac(5, 42, 0, 7); /* (DID=0) access to SAI7 regs */
	xrdc_config_pdac(5, 43, 0, 7); /* (DID=0) access to SPDIF regs */
	xrdc_config_pdac(5, 41, 7, 7); /* (DID=7) access to SAI6 regs */
	xrdc_config_pdac(5, 42, 7, 7); /* (DID=7) access to SAI7 regs */
	xrdc_config_pdac(5, 43, 7, 7); /* (DID=7) access to SPDIF regs */
	xrdc_config_mrc6_dma2_ddr();
#endif

	return 0;
}

int upower_pwm(int domain_id, bool pwr_on)
{
	int ret, ret_val;
	uint32_t swt;

	if (domain_id == 9 || domain_id == 11 || domain_id == 12)
		swt = BIT_32(12) | BIT_32(11) | BIT_32(10) | BIT_32(9);
	else
		swt = BIT_32(domain_id);
	/*
	 * TODO:
	 *	Add domain_id check
	 *	mem switch?
	 */
	if (pwr_on)
		ret = upwr_pwm_power_on(&swt, NULL, NULL);
	else
		ret = upwr_pwm_power_off(&swt, NULL, NULL);

	if (ret) {
		NOTICE("%s failed: ret: %d, pwr_on: %d\n", __func__, ret, pwr_on);
		return ret;
	}
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		NOTICE("Faliure %d, %s\n", ret, __func__);
		if (ret == UPWR_REQ_BUSY)
			return -EBUSY;
		else
			return -EINVAL;
	}

	return 0;
}
