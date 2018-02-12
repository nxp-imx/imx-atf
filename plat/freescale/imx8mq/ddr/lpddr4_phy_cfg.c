/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <ddrc.h>
#include <mmio.h>
#include <platform_def.h>
#include <spinlock.h>
#include <soc.h>

struct ddrphy_cfg_param {
	uint32_t offset; /*reg offset */
	uint32_t val; /* config param */
};

/* lpddr4 phy init config parameters */
static const struct ddrphy_cfg_param phy_init_cfg[] = {
	{ .offset = 0x20110, .val = 0x02 }, /* MapCAB0toDFI */
	{ .offset = 0x20111, .val = 0x03 }, /* MapCAB1toDFI */
	{ .offset = 0x20112, .val = 0x04 }, /* MapCAB2toDFI */
	{ .offset = 0x20113, .val = 0x05 }, /* MapCAB3toDFI */
	{ .offset = 0x20114, .val = 0x00 }, /* MapCAB4toDFI */
	{ .offset = 0x20115, .val = 0x01 }, /* MapCAB5toDFI */

	/* Initialize PHY Configuration */
	{ .offset = 0x1005f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p0
	{ .offset = 0x1015f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p0
	{ .offset = 0x1105f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p0
	{ .offset = 0x1115f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p0
	{ .offset = 0x1205f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p0
	{ .offset = 0x1215f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p0
	{ .offset = 0x1305f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p0
	{ .offset = 0x1315f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p0

	{ .offset = 0x11005f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p1
	{ .offset = 0x11015f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p1
	{ .offset = 0x11105f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p1
	{ .offset = 0x11115f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p1
	{ .offset = 0x11205f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p1
	{ .offset = 0x11215f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p1
	{ .offset = 0x11305f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p1
	{ .offset = 0x11315f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p1

	{ .offset = 0x21005f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b0_p2
	{ .offset = 0x21015f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE0_TxSlewRate_b1_p2
	{ .offset = 0x21105f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b0_p2
	{ .offset = 0x21115f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE1_TxSlewRate_b1_p2
	{ .offset = 0x21205f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b0_p2
	{ .offset = 0x21215f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE2_TxSlewRate_b1_p2
	{ .offset = 0x21305f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b0_p2
	{ .offset = 0x21315f, .val = 0x1ff }, // DWC_DDRPHYA_DBYTE3_TxSlewRate_b1_p2

	{ .offset = 0x55, .val = 0x1ff }, // DWC_DDRPHYA_ANIB0_ATxSlewRate
	{ .offset = 0x1055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB1_ATxSlewRate
	{ .offset = 0x2055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB2_ATxSlewRate
	{ .offset = 0x3055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB3_ATxSlewRate
	{ .offset = 0x4055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB4_ATxSlewRate
	{ .offset = 0x5055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB5_ATxSlewRate
	{ .offset = 0x6055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB6_ATxSlewRate
	{ .offset = 0x7055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB7_ATxSlewRate
	{ .offset = 0x8055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB8_ATxSlewRate
	{ .offset = 0x9055, .val = 0x1ff }, // DWC_DDRPHYA_ANIB9_ATxSlewRate
	{ .offset = 0x200c5, .val = 0x19 }, // DWC_DDRPHYA_MASTER0_PllCtrl2_p0
	{ .offset = 0x1200c5, .val = 0x7 }, // DWC_DDRPHYA_MASTER0_PllCtrl2_p1
	{ .offset = 0x2200c5, .val = 0x7 }, // DWC_DDRPHYA_MASTER0_PllCtrl2_p2
	{ .offset = 0x2002e, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p0
	{ .offset = 0x12002e, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p1
	{ .offset = 0x22002e, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_ARdPtrInitVal_p2
	{ .offset = 0x90204, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p0
	{ .offset = 0x190204, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p1
	{ .offset = 0x290204, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_Seq0BGPR4_p2

	{ .offset = 0x20024, .val = 0xe3 }, // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p0
	{ .offset = 0x2003a, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
	{ .offset = 0x120024, .val = 0xa3 }, // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p1
	{ .offset = 0x2003a, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
	{ .offset = 0x220024, .val = 0xa3 }, // DWC_DDRPHYA_MASTER0_DqsPreambleControl_p2
	{ .offset = 0x2003a, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl

	{ .offset = 0x20056, .val = 0x3 }, // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p0
	{ .offset = 0x120056, .val = 0xa }, // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p1
	{ .offset = 0x220056, .val = 0xa }, // DWC_DDRPHYA_MASTER0_ProcOdtTimeCtl_p2

	{ .offset = 0x1004d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p0
	{ .offset = 0x1014d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p0
	{ .offset = 0x1104d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p0
	{ .offset = 0x1114d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p0
	{ .offset = 0x1204d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p0
	{ .offset = 0x1214d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p0
	{ .offset = 0x1304d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p0
	{ .offset = 0x1314d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p0
	{ .offset = 0x11004d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p1
	{ .offset = 0x11014d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p1
	{ .offset = 0x11104d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p1
	{ .offset = 0x11114d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p1
	{ .offset = 0x11204d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p1
	{ .offset = 0x11214d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p1
	{ .offset = 0x11304d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p1
	{ .offset = 0x11314d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p1
	{ .offset = 0x21004d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b0_p2
	{ .offset = 0x21014d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE0_TxOdtDrvStren_b1_p2
	{ .offset = 0x21104d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b0_p2
	{ .offset = 0x21114d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE1_TxOdtDrvStren_b1_p2
	{ .offset = 0x21204d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b0_p2
	{ .offset = 0x21214d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE2_TxOdtDrvStren_b1_p2
	{ .offset = 0x21304d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b0_p2
	{ .offset = 0x21314d, .val = 0xe00 }, // DWC_DDRPHYA_DBYTE3_TxOdtDrvStren_b1_p2

	{ .offset = 0x10049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p0
	{ .offset = 0x10149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p0
	{ .offset = 0x11049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p0
	{ .offset = 0x11149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p0
	{ .offset = 0x12049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p0
	{ .offset = 0x12149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p0
	{ .offset = 0x13049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p0
	{ .offset = 0x13149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p0

	{ .offset = 0x110049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p1
	{ .offset = 0x110149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p1
	{ .offset = 0x111049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p1
	{ .offset = 0x111149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p1
	{ .offset = 0x112049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p1
	{ .offset = 0x112149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p1
	{ .offset = 0x113049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p1
	{ .offset = 0x113149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p1

	{ .offset = 0x210049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b0_p2
	{ .offset = 0x210149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE0_TxImpedanceCtrl1_b1_p2
	{ .offset = 0x211049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b0_p2
	{ .offset = 0x211149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE1_TxImpedanceCtrl1_b1_p2
	{ .offset = 0x212049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b0_p2
	{ .offset = 0x212149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE2_TxImpedanceCtrl1_b1_p2
	{ .offset = 0x213049, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b0_p2
	{ .offset = 0x213149, .val = 0xfbe }, // DWC_DDRPHYA_DBYTE3_TxImpedanceCtrl1_b1_p2

	{ .offset = 0x43, .val = 0x63 }, // DWC_DDRPHYA_ANIB0_ATxImpedance
	{ .offset = 0x1043, .val = 0x63 }, // DWC_DDRPHYA_ANIB1_ATxImpedance
	{ .offset = 0x2043, .val = 0x63 }, // DWC_DDRPHYA_ANIB2_ATxImpedance
	{ .offset = 0x3043, .val = 0x63 }, // DWC_DDRPHYA_ANIB3_ATxImpedance
	{ .offset = 0x4043, .val = 0x63 }, // DWC_DDRPHYA_ANIB4_ATxImpedance
	{ .offset = 0x5043, .val = 0x63 }, // DWC_DDRPHYA_ANIB5_ATxImpedance
	{ .offset = 0x6043, .val = 0x63 }, // DWC_DDRPHYA_ANIB6_ATxImpedance
	{ .offset = 0x7043, .val = 0x63 }, // DWC_DDRPHYA_ANIB7_ATxImpedance
	{ .offset = 0x8043, .val = 0x63 }, // DWC_DDRPHYA_ANIB8_ATxImpedance
	{ .offset = 0x9043, .val = 0x63 }, // DWC_DDRPHYA_ANIB9_ATxImpedance

	{ .offset = 0x20018, .val = 0x3 }, // DWC_DDRPHYA_MASTER0_DfiMode
	{ .offset = 0x20075, .val = 0x4 }, // DWC_DDRPHYA_MASTER0_DfiCAMode
	{ .offset = 0x20050, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_CalDrvStr0
	{ .offset = 0x20008, .val = 0x320 }, // DWC_DDRPHYA_MASTER0_CalUclkInfo_p0
	{ .offset = 0x120008, .val = 0xa7 }, // DWC_DDRPHYA_MASTER0_CalUclkInfo_p1
	{ .offset = 0x220008, .val = 0x19 }, // DWC_DDRPHYA_MASTER0_CalUclkInfo_p2
	{ .offset = 0x20088, .val = 0x9 }, // DWC_DDRPHYA_MASTER0_CalRate
	{ .offset = 0x200b2, .val = 0x104 }, // DWC_DDRPHYA_MASTER0_VrefInGlobal_p0
	{ .offset = 0x10043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p0
	{ .offset = 0x10143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p0
	{ .offset = 0x11043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p0
	{ .offset = 0x11143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p0
	{ .offset = 0x12043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p0
	{ .offset = 0x12143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p0
	{ .offset = 0x13043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p0
	{ .offset = 0x13143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p0
	{ .offset = 0x1200b2, .val = 0x104 }, // DWC_DDRPHYA_MASTER0_VrefInGlobal_p1
	{ .offset = 0x110043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p1
	{ .offset = 0x110143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p1
	{ .offset = 0x111043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p1
	{ .offset = 0x111143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p1
	{ .offset = 0x112043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p1
	{ .offset = 0x112143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p1
	{ .offset = 0x113043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p1
	{ .offset = 0x113143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p1
	{ .offset = 0x2200b2, .val = 0x104 }, // DWC_DDRPHYA_MASTER0_VrefInGlobal_p2
	{ .offset = 0x210043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b0_p2
	{ .offset = 0x210143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE0_DqDqsRcvCntrl_b1_p2
	{ .offset = 0x211043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b0_p2
	{ .offset = 0x211143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE1_DqDqsRcvCntrl_b1_p2
	{ .offset = 0x212043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b0_p2
	{ .offset = 0x212143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE2_DqDqsRcvCntrl_b1_p2
	{ .offset = 0x213043, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b0_p2
	{ .offset = 0x213143, .val = 0x5a1 }, // DWC_DDRPHYA_DBYTE3_DqDqsRcvCntrl_b1_p2
	{ .offset = 0x200fa, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p0
	{ .offset = 0x1200fa, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p1
	{ .offset = 0x2200fa, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_DfiFreqRatio_p2
	{ .offset = 0x20019, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_TristateModeCA_p0
	{ .offset = 0x120019, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_TristateModeCA_p1
	{ .offset = 0x220019, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_TristateModeCA_p2
	{ .offset = 0x200f0, .val = 0x600 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat0
	{ .offset = 0x200f1, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat1
	{ .offset = 0x200f2, .val = 0x4444 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat2
	{ .offset = 0x200f3, .val = 0x8888 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat3
	{ .offset = 0x200f4, .val = 0x5655 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat4
	{ .offset = 0x200f5, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat5
	{ .offset = 0x200f6, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat6
	{ .offset = 0x200f7, .val = 0xf000 }, // DWC_DDRPHYA_MASTER0_DfiFreqXlat7
	{ .offset = 0x20025, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_MasterX4Config
	{ .offset = 0x2002d, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DMIPinPresent_p0
	{ .offset = 0x12002d, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DMIPinPresent_p1
	{ .offset = 0x22002d, .val = 0x0 }, // DWC_DDRPHYA_MASTER0_DMIPinPresent_p2
};

/* lpddr4 phy trained CSR registers */
static struct ddrphy_cfg_param phy_trained_csr[] = {
        { .offset = 0x200b2,  .val = 0 },
        { .offset = 0x1200b2,  .val = 0 },
        { .offset = 0x2200b2,  .val = 0 },
        { .offset = 0x200cb,  .val = 0 },
        { .offset = 0x10043,  .val = 0 },
        { .offset = 0x110043,  .val = 0 },
        { .offset = 0x210043,  .val = 0 },
        { .offset = 0x10143,  .val = 0 },
        { .offset = 0x110143,  .val = 0 },
        { .offset = 0x210143,  .val = 0 },
        { .offset = 0x11043,  .val = 0 },
        { .offset = 0x111043,  .val = 0 },
        { .offset = 0x211043,  .val = 0 },
        { .offset = 0x11143,  .val = 0 },
        { .offset = 0x111143,  .val = 0 },
        { .offset = 0x211143,  .val = 0 },
        { .offset = 0x12043,  .val = 0 },
        { .offset = 0x112043,  .val = 0 },
        { .offset = 0x212043,  .val = 0 },
        { .offset = 0x12143,  .val = 0 },
        { .offset = 0x112143,  .val = 0 },
        { .offset = 0x212143,  .val = 0 },
        { .offset = 0x13043,  .val = 0 },
        { .offset = 0x113043,  .val = 0 },
        { .offset = 0x213043,  .val = 0 },
        { .offset = 0x13143,  .val = 0 },
        { .offset = 0x113143,  .val = 0 },
        { .offset = 0x213143,  .val = 0 },
        { .offset = 0x80,  .val = 0 },
        { .offset = 0x100080,  .val = 0 },
        { .offset = 0x200080,  .val = 0 },
        { .offset = 0x1080,  .val = 0 },
        { .offset = 0x101080,  .val = 0 },
        { .offset = 0x201080,  .val = 0 },
        { .offset = 0x2080,  .val = 0 },
        { .offset = 0x102080,  .val = 0 },
        { .offset = 0x202080,  .val = 0 },
        { .offset = 0x3080,  .val = 0 },
        { .offset = 0x103080,  .val = 0 },
        { .offset = 0x203080,  .val = 0 },
        { .offset = 0x4080,  .val = 0 },
        { .offset = 0x104080,  .val = 0 },
        { .offset = 0x204080,  .val = 0 },
        { .offset = 0x5080,  .val = 0 },
        { .offset = 0x105080,  .val = 0 },
        { .offset = 0x205080,  .val = 0 },
        { .offset = 0x6080,  .val = 0 },
        { .offset = 0x106080,  .val = 0 },
        { .offset = 0x206080,  .val = 0 },
        { .offset = 0x7080,  .val = 0 },
        { .offset = 0x107080,  .val = 0 },
        { .offset = 0x207080,  .val = 0 },
        { .offset = 0x8080,  .val = 0 },
        { .offset = 0x108080,  .val = 0 },
        { .offset = 0x208080,  .val = 0 },
        { .offset = 0x9080,  .val = 0 },
        { .offset = 0x109080,  .val = 0 },
        { .offset = 0x209080,  .val = 0 },
        { .offset = 0x10080,  .val = 0 },
        { .offset = 0x110080,  .val = 0 },
        { .offset = 0x210080,  .val = 0 },
        { .offset = 0x10180,  .val = 0 },
        { .offset = 0x110180,  .val = 0 },
        { .offset = 0x210180,  .val = 0 },
        { .offset = 0x11080,  .val = 0 },
        { .offset = 0x111080,  .val = 0 },
        { .offset = 0x211080,  .val = 0 },
        { .offset = 0x11180,  .val = 0 },
        { .offset = 0x111180,  .val = 0 },
        { .offset = 0x211180,  .val = 0 },
        { .offset = 0x12080,  .val = 0 },
        { .offset = 0x112080,  .val = 0 },
        { .offset = 0x212080,  .val = 0 },
        { .offset = 0x12180,  .val = 0 },
        { .offset = 0x112180,  .val = 0 },
        { .offset = 0x212180,  .val = 0 },
        { .offset = 0x13080,  .val = 0 },
        { .offset = 0x113080,  .val = 0 },
        { .offset = 0x213080,  .val = 0 },
        { .offset = 0x13180,  .val = 0 },
        { .offset = 0x113180,  .val = 0 },
        { .offset = 0x213180,  .val = 0 },
        { .offset = 0x10081,  .val = 0 },
        { .offset = 0x110081,  .val = 0 },
        { .offset = 0x210081,  .val = 0 },
        { .offset = 0x10181,  .val = 0 },
        { .offset = 0x110181,  .val = 0 },
        { .offset = 0x210181,  .val = 0 },
        { .offset = 0x11081,  .val = 0 },
        { .offset = 0x111081,  .val = 0 },
        { .offset = 0x211081,  .val = 0 },
        { .offset = 0x11181,  .val = 0 },
        { .offset = 0x111181,  .val = 0 },
        { .offset = 0x211181,  .val = 0 },
        { .offset = 0x12081,  .val = 0 },
        { .offset = 0x112081,  .val = 0 },
        { .offset = 0x212081,  .val = 0 },
        { .offset = 0x12181,  .val = 0 },
        { .offset = 0x112181,  .val = 0 },
        { .offset = 0x212181,  .val = 0 },
        { .offset = 0x13081,  .val = 0 },
        { .offset = 0x113081,  .val = 0 },
        { .offset = 0x213081,  .val = 0 },
        { .offset = 0x13181,  .val = 0 },
        { .offset = 0x113181,  .val = 0 },
        { .offset = 0x213181,  .val = 0 },
        { .offset = 0x100d0,  .val = 0 },
        { .offset = 0x1100d0,  .val = 0 },
        { .offset = 0x2100d0,  .val = 0 },
        { .offset = 0x101d0,  .val = 0 },
        { .offset = 0x1101d0,  .val = 0 },
        { .offset = 0x2101d0,  .val = 0 },
        { .offset = 0x110d0,  .val = 0 },
        { .offset = 0x1110d0,  .val = 0 },
        { .offset = 0x2110d0,  .val = 0 },
        { .offset = 0x111d0,  .val = 0 },
        { .offset = 0x1111d0,  .val = 0 },
        { .offset = 0x2111d0,  .val = 0 },
        { .offset = 0x120d0,  .val = 0 },
        { .offset = 0x1120d0,  .val = 0 },
        { .offset = 0x2120d0,  .val = 0 },
        { .offset = 0x121d0,  .val = 0 },
        { .offset = 0x1121d0,  .val = 0 },
        { .offset = 0x2121d0,  .val = 0 },
        { .offset = 0x130d0,  .val = 0 },
        { .offset = 0x1130d0,  .val = 0 },
        { .offset = 0x2130d0,  .val = 0 },
        { .offset = 0x131d0,  .val = 0 },
        { .offset = 0x1131d0,  .val = 0 },
        { .offset = 0x2131d0,  .val = 0 },
        { .offset = 0x100d1,  .val = 0 },
        { .offset = 0x1100d1,  .val = 0 },
        { .offset = 0x2100d1,  .val = 0 },
        { .offset = 0x101d1,  .val = 0 },
        { .offset = 0x1101d1,  .val = 0 },
        { .offset = 0x2101d1,  .val = 0 },
        { .offset = 0x110d1,  .val = 0 },
        { .offset = 0x1110d1,  .val = 0 },
        { .offset = 0x2110d1,  .val = 0 },
        { .offset = 0x111d1,  .val = 0 },
        { .offset = 0x1111d1,  .val = 0 },
        { .offset = 0x2111d1,  .val = 0 },
        { .offset = 0x120d1,  .val = 0 },
        { .offset = 0x1120d1,  .val = 0 },
        { .offset = 0x2120d1,  .val = 0 },
        { .offset = 0x121d1,  .val = 0 },
        { .offset = 0x1121d1,  .val = 0 },
        { .offset = 0x2121d1,  .val = 0 },
        { .offset = 0x130d1,  .val = 0 },
        { .offset = 0x1130d1,  .val = 0 },
        { .offset = 0x2130d1,  .val = 0 },
        { .offset = 0x131d1,  .val = 0 },
        { .offset = 0x1131d1,  .val = 0 },
        { .offset = 0x2131d1,  .val = 0 },
        { .offset = 0x10068,  .val = 0 },
        { .offset = 0x10168,  .val = 0 },
        { .offset = 0x10268,  .val = 0 },
        { .offset = 0x10368,  .val = 0 },
        { .offset = 0x10468,  .val = 0 },
        { .offset = 0x10568,  .val = 0 },
        { .offset = 0x10668,  .val = 0 },
        { .offset = 0x10768,  .val = 0 },
        { .offset = 0x10868,  .val = 0 },
        { .offset = 0x11068,  .val = 0 },
        { .offset = 0x11168,  .val = 0 },
        { .offset = 0x11268,  .val = 0 },
        { .offset = 0x11368,  .val = 0 },
        { .offset = 0x11468,  .val = 0 },
        { .offset = 0x11568,  .val = 0 },
        { .offset = 0x11668,  .val = 0 },
        { .offset = 0x11768,  .val = 0 },
        { .offset = 0x11868,  .val = 0 },
        { .offset = 0x12068,  .val = 0 },
        { .offset = 0x12168,  .val = 0 },
        { .offset = 0x12268,  .val = 0 },
        { .offset = 0x12368,  .val = 0 },
        { .offset = 0x12468,  .val = 0 },
        { .offset = 0x12568,  .val = 0 },
        { .offset = 0x12668,  .val = 0 },
        { .offset = 0x12768,  .val = 0 },
        { .offset = 0x12868,  .val = 0 },
        { .offset = 0x13068,  .val = 0 },
        { .offset = 0x13168,  .val = 0 },
        { .offset = 0x13268,  .val = 0 },
        { .offset = 0x13368,  .val = 0 },
        { .offset = 0x13468,  .val = 0 },
        { .offset = 0x13568,  .val = 0 },
        { .offset = 0x13668,  .val = 0 },
        { .offset = 0x13768,  .val = 0 },
        { .offset = 0x13868,  .val = 0 },
        { .offset = 0x10069,  .val = 0 },
        { .offset = 0x10169,  .val = 0 },
        { .offset = 0x10269,  .val = 0 },
        { .offset = 0x10369,  .val = 0 },
        { .offset = 0x10469,  .val = 0 },
        { .offset = 0x10569,  .val = 0 },
        { .offset = 0x10669,  .val = 0 },
        { .offset = 0x10769,  .val = 0 },
        { .offset = 0x10869,  .val = 0 },
        { .offset = 0x11069,  .val = 0 },
        { .offset = 0x11169,  .val = 0 },
        { .offset = 0x11269,  .val = 0 },
        { .offset = 0x11369,  .val = 0 },
        { .offset = 0x11469,  .val = 0 },
        { .offset = 0x11569,  .val = 0 },
        { .offset = 0x11669,  .val = 0 },
        { .offset = 0x11769,  .val = 0 },
        { .offset = 0x11869,  .val = 0 },
        { .offset = 0x12069,  .val = 0 },
        { .offset = 0x12169,  .val = 0 },
        { .offset = 0x12269,  .val = 0 },
        { .offset = 0x12369,  .val = 0 },
        { .offset = 0x12469,  .val = 0 },
        { .offset = 0x12569,  .val = 0 },
        { .offset = 0x12669,  .val = 0 },
        { .offset = 0x12769,  .val = 0 },
        { .offset = 0x12869,  .val = 0 },
        { .offset = 0x13069,  .val = 0 },
        { .offset = 0x13169,  .val = 0 },
        { .offset = 0x13269,  .val = 0 },
        { .offset = 0x13369,  .val = 0 },
        { .offset = 0x13469,  .val = 0 },
        { .offset = 0x13569,  .val = 0 },
        { .offset = 0x13669,  .val = 0 },
        { .offset = 0x13769,  .val = 0 },
        { .offset = 0x13869,  .val = 0 },
        { .offset = 0x1008c,  .val = 0 },
        { .offset = 0x11008c,  .val = 0 },
        { .offset = 0x21008c,  .val = 0 },
        { .offset = 0x1018c,  .val = 0 },
        { .offset = 0x11018c,  .val = 0 },
        { .offset = 0x21018c,  .val = 0 },
        { .offset = 0x1108c,  .val = 0 },
        { .offset = 0x11108c,  .val = 0 },
        { .offset = 0x21108c,  .val = 0 },
        { .offset = 0x1118c,  .val = 0 },
        { .offset = 0x11118c,  .val = 0 },
        { .offset = 0x21118c,  .val = 0 },
        { .offset = 0x1208c,  .val = 0 },
        { .offset = 0x11208c,  .val = 0 },
        { .offset = 0x21208c,  .val = 0 },
        { .offset = 0x1218c,  .val = 0 },
        { .offset = 0x11218c,  .val = 0 },
        { .offset = 0x21218c,  .val = 0 },
        { .offset = 0x1308c,  .val = 0 },
        { .offset = 0x11308c,  .val = 0 },
        { .offset = 0x21308c,  .val = 0 },
        { .offset = 0x1318c,  .val = 0 },
        { .offset = 0x11318c,  .val = 0 },
        { .offset = 0x21318c,  .val = 0 },
        { .offset = 0x1008d,  .val = 0 },
        { .offset = 0x11008d,  .val = 0 },
        { .offset = 0x21008d,  .val = 0 },
        { .offset = 0x1018d,  .val = 0 },
        { .offset = 0x11018d,  .val = 0 },
        { .offset = 0x21018d,  .val = 0 },
        { .offset = 0x1108d,  .val = 0 },
        { .offset = 0x11108d,  .val = 0 },
        { .offset = 0x21108d,  .val = 0 },
        { .offset = 0x1118d,  .val = 0 },
        { .offset = 0x11118d,  .val = 0 },
        { .offset = 0x21118d,  .val = 0 },
        { .offset = 0x1208d,  .val = 0 },
        { .offset = 0x11208d,  .val = 0 },
        { .offset = 0x21208d,  .val = 0 },
        { .offset = 0x1218d,  .val = 0 },
        { .offset = 0x11218d,  .val = 0 },
        { .offset = 0x21218d,  .val = 0 },
        { .offset = 0x1308d,  .val = 0 },
        { .offset = 0x11308d,  .val = 0 },
        { .offset = 0x21308d,  .val = 0 },
        { .offset = 0x1318d,  .val = 0 },
        { .offset = 0x11318d,  .val = 0 },
        { .offset = 0x21318d,  .val = 0 },
        { .offset = 0x100c0,  .val = 0 },
        { .offset = 0x1100c0,  .val = 0 },
        { .offset = 0x2100c0,  .val = 0 },
        { .offset = 0x101c0,  .val = 0 },
        { .offset = 0x1101c0,  .val = 0 },
        { .offset = 0x2101c0,  .val = 0 },
        { .offset = 0x102c0,  .val = 0 },
        { .offset = 0x1102c0,  .val = 0 },
        { .offset = 0x2102c0,  .val = 0 },
        { .offset = 0x103c0,  .val = 0 },
        { .offset = 0x1103c0,  .val = 0 },
        { .offset = 0x2103c0,  .val = 0 },
        { .offset = 0x104c0,  .val = 0 },
        { .offset = 0x1104c0,  .val = 0 },
        { .offset = 0x2104c0,  .val = 0 },
        { .offset = 0x105c0,  .val = 0 },
        { .offset = 0x1105c0,  .val = 0 },
        { .offset = 0x2105c0,  .val = 0 },
        { .offset = 0x106c0,  .val = 0 },
        { .offset = 0x1106c0,  .val = 0 },
        { .offset = 0x2106c0,  .val = 0 },
        { .offset = 0x107c0,  .val = 0 },
        { .offset = 0x1107c0,  .val = 0 },
        { .offset = 0x2107c0,  .val = 0 },
        { .offset = 0x108c0,  .val = 0 },
        { .offset = 0x1108c0,  .val = 0 },
        { .offset = 0x2108c0,  .val = 0 },
        { .offset = 0x110c0,  .val = 0 },
        { .offset = 0x1110c0,  .val = 0 },
        { .offset = 0x2110c0,  .val = 0 },
        { .offset = 0x111c0,  .val = 0 },
        { .offset = 0x1111c0,  .val = 0 },
        { .offset = 0x2111c0,  .val = 0 },
        { .offset = 0x112c0,  .val = 0 },
        { .offset = 0x1112c0,  .val = 0 },
        { .offset = 0x2112c0,  .val = 0 },
        { .offset = 0x113c0,  .val = 0 },
        { .offset = 0x1113c0,  .val = 0 },
        { .offset = 0x2113c0,  .val = 0 },
        { .offset = 0x114c0,  .val = 0 },
        { .offset = 0x1114c0,  .val = 0 },
        { .offset = 0x2114c0,  .val = 0 },
        { .offset = 0x115c0,  .val = 0 },
        { .offset = 0x1115c0,  .val = 0 },
        { .offset = 0x2115c0,  .val = 0 },
        { .offset = 0x116c0,  .val = 0 },
        { .offset = 0x1116c0,  .val = 0 },
        { .offset = 0x2116c0,  .val = 0 },
        { .offset = 0x117c0,  .val = 0 },
        { .offset = 0x1117c0,  .val = 0 },
        { .offset = 0x2117c0,  .val = 0 },
        { .offset = 0x118c0,  .val = 0 },
        { .offset = 0x1118c0,  .val = 0 },
        { .offset = 0x2118c0,  .val = 0 },
        { .offset = 0x120c0,  .val = 0 },
        { .offset = 0x1120c0,  .val = 0 },
        { .offset = 0x2120c0,  .val = 0 },
        { .offset = 0x121c0,  .val = 0 },
        { .offset = 0x1121c0,  .val = 0 },
        { .offset = 0x2121c0,  .val = 0 },
        { .offset = 0x122c0,  .val = 0 },
        { .offset = 0x1122c0,  .val = 0 },
        { .offset = 0x2122c0,  .val = 0 },
        { .offset = 0x123c0,  .val = 0 },
        { .offset = 0x1123c0,  .val = 0 },
        { .offset = 0x2123c0,  .val = 0 },
        { .offset = 0x124c0,  .val = 0 },
        { .offset = 0x1124c0,  .val = 0 },
        { .offset = 0x2124c0,  .val = 0 },
        { .offset = 0x125c0,  .val = 0 },
        { .offset = 0x1125c0,  .val = 0 },
        { .offset = 0x2125c0,  .val = 0 },
        { .offset = 0x126c0,  .val = 0 },
        { .offset = 0x1126c0,  .val = 0 },
        { .offset = 0x2126c0,  .val = 0 },
        { .offset = 0x127c0,  .val = 0 },
        { .offset = 0x1127c0,  .val = 0 },
        { .offset = 0x2127c0,  .val = 0 },
        { .offset = 0x128c0,  .val = 0 },
        { .offset = 0x1128c0,  .val = 0 },
        { .offset = 0x2128c0,  .val = 0 },
        { .offset = 0x130c0,  .val = 0 },
        { .offset = 0x1130c0,  .val = 0 },
        { .offset = 0x2130c0,  .val = 0 },
        { .offset = 0x131c0,  .val = 0 },
        { .offset = 0x1131c0,  .val = 0 },
        { .offset = 0x2131c0,  .val = 0 },
        { .offset = 0x132c0,  .val = 0 },
        { .offset = 0x1132c0,  .val = 0 },
        { .offset = 0x2132c0,  .val = 0 },
        { .offset = 0x133c0,  .val = 0 },
        { .offset = 0x1133c0,  .val = 0 },
        { .offset = 0x2133c0,  .val = 0 },
        { .offset = 0x134c0,  .val = 0 },
        { .offset = 0x1134c0,  .val = 0 },
        { .offset = 0x2134c0,  .val = 0 },
        { .offset = 0x135c0,  .val = 0 },
        { .offset = 0x1135c0,  .val = 0 },
        { .offset = 0x2135c0,  .val = 0 },
        { .offset = 0x136c0,  .val = 0 },
        { .offset = 0x1136c0,  .val = 0 },
        { .offset = 0x2136c0,  .val = 0 },
        { .offset = 0x137c0,  .val = 0 },
        { .offset = 0x1137c0,  .val = 0 },
        { .offset = 0x2137c0,  .val = 0 },
        { .offset = 0x138c0,  .val = 0 },
        { .offset = 0x1138c0,  .val = 0 },
        { .offset = 0x2138c0,  .val = 0 },
        { .offset = 0x100c1,  .val = 0 },
        { .offset = 0x1100c1,  .val = 0 },
        { .offset = 0x2100c1,  .val = 0 },
        { .offset = 0x101c1,  .val = 0 },
        { .offset = 0x1101c1,  .val = 0 },
        { .offset = 0x2101c1,  .val = 0 },
        { .offset = 0x102c1,  .val = 0 },
        { .offset = 0x1102c1,  .val = 0 },
        { .offset = 0x2102c1,  .val = 0 },
        { .offset = 0x103c1,  .val = 0 },
        { .offset = 0x1103c1,  .val = 0 },
        { .offset = 0x2103c1,  .val = 0 },
        { .offset = 0x104c1,  .val = 0 },
        { .offset = 0x1104c1,  .val = 0 },
        { .offset = 0x2104c1,  .val = 0 },
        { .offset = 0x105c1,  .val = 0 },
        { .offset = 0x1105c1,  .val = 0 },
        { .offset = 0x2105c1,  .val = 0 },
        { .offset = 0x106c1,  .val = 0 },
        { .offset = 0x1106c1,  .val = 0 },
        { .offset = 0x2106c1,  .val = 0 },
        { .offset = 0x107c1,  .val = 0 },
        { .offset = 0x1107c1,  .val = 0 },
        { .offset = 0x2107c1,  .val = 0 },
        { .offset = 0x108c1,  .val = 0 },
        { .offset = 0x1108c1,  .val = 0 },
        { .offset = 0x2108c1,  .val = 0 },
        { .offset = 0x110c1,  .val = 0 },
        { .offset = 0x1110c1,  .val = 0 },
        { .offset = 0x2110c1,  .val = 0 },
        { .offset = 0x111c1,  .val = 0 },
        { .offset = 0x1111c1,  .val = 0 },
        { .offset = 0x2111c1,  .val = 0 },
        { .offset = 0x112c1,  .val = 0 },
        { .offset = 0x1112c1,  .val = 0 },
        { .offset = 0x2112c1,  .val = 0 },
        { .offset = 0x113c1,  .val = 0 },
        { .offset = 0x1113c1,  .val = 0 },
        { .offset = 0x2113c1,  .val = 0 },
        { .offset = 0x114c1,  .val = 0 },
        { .offset = 0x1114c1,  .val = 0 },
        { .offset = 0x2114c1,  .val = 0 },
        { .offset = 0x115c1,  .val = 0 },
        { .offset = 0x1115c1,  .val = 0 },
        { .offset = 0x2115c1,  .val = 0 },
        { .offset = 0x116c1,  .val = 0 },
        { .offset = 0x1116c1,  .val = 0 },
        { .offset = 0x2116c1,  .val = 0 },
        { .offset = 0x117c1,  .val = 0 },
        { .offset = 0x1117c1,  .val = 0 },
        { .offset = 0x2117c1,  .val = 0 },
        { .offset = 0x118c1,  .val = 0 },
        { .offset = 0x1118c1,  .val = 0 },
        { .offset = 0x2118c1,  .val = 0 },
        { .offset = 0x120c1,  .val = 0 },
        { .offset = 0x1120c1,  .val = 0 },
        { .offset = 0x2120c1,  .val = 0 },
        { .offset = 0x121c1,  .val = 0 },
        { .offset = 0x1121c1,  .val = 0 },
        { .offset = 0x2121c1,  .val = 0 },
        { .offset = 0x122c1,  .val = 0 },
        { .offset = 0x1122c1,  .val = 0 },
        { .offset = 0x2122c1,  .val = 0 },
        { .offset = 0x123c1,  .val = 0 },
        { .offset = 0x1123c1,  .val = 0 },
        { .offset = 0x2123c1,  .val = 0 },
        { .offset = 0x124c1,  .val = 0 },
        { .offset = 0x1124c1,  .val = 0 },
        { .offset = 0x2124c1,  .val = 0 },
        { .offset = 0x125c1,  .val = 0 },
        { .offset = 0x1125c1,  .val = 0 },
        { .offset = 0x2125c1,  .val = 0 },
        { .offset = 0x126c1,  .val = 0 },
        { .offset = 0x1126c1,  .val = 0 },
        { .offset = 0x2126c1,  .val = 0 },
        { .offset = 0x127c1,  .val = 0 },
        { .offset = 0x1127c1,  .val = 0 },
        { .offset = 0x2127c1,  .val = 0 },
        { .offset = 0x128c1,  .val = 0 },
        { .offset = 0x1128c1,  .val = 0 },
        { .offset = 0x2128c1,  .val = 0 },
        { .offset = 0x130c1,  .val = 0 },
        { .offset = 0x1130c1,  .val = 0 },
        { .offset = 0x2130c1,  .val = 0 },
        { .offset = 0x131c1,  .val = 0 },
        { .offset = 0x1131c1,  .val = 0 },
        { .offset = 0x2131c1,  .val = 0 },
        { .offset = 0x132c1,  .val = 0 },
        { .offset = 0x1132c1,  .val = 0 },
        { .offset = 0x2132c1,  .val = 0 },
        { .offset = 0x133c1,  .val = 0 },
        { .offset = 0x1133c1,  .val = 0 },
        { .offset = 0x2133c1,  .val = 0 },
        { .offset = 0x134c1,  .val = 0 },
        { .offset = 0x1134c1,  .val = 0 },
        { .offset = 0x2134c1,  .val = 0 },
        { .offset = 0x135c1,  .val = 0 },
        { .offset = 0x1135c1,  .val = 0 },
        { .offset = 0x2135c1,  .val = 0 },
        { .offset = 0x136c1,  .val = 0 },
        { .offset = 0x1136c1,  .val = 0 },
        { .offset = 0x2136c1,  .val = 0 },
        { .offset = 0x137c1,  .val = 0 },
        { .offset = 0x1137c1,  .val = 0 },
        { .offset = 0x2137c1,  .val = 0 },
        { .offset = 0x138c1,  .val = 0 },
        { .offset = 0x1138c1,  .val = 0 },
        { .offset = 0x2138c1,  .val = 0 },
        { .offset = 0x10020,  .val = 0 },
        { .offset = 0x110020,  .val = 0 },
        { .offset = 0x210020,  .val = 0 },
        { .offset = 0x11020,  .val = 0 },
        { .offset = 0x111020,  .val = 0 },
        { .offset = 0x211020,  .val = 0 },
        { .offset = 0x12020,  .val = 0 },
        { .offset = 0x112020,  .val = 0 },
        { .offset = 0x212020,  .val = 0 },
        { .offset = 0x13020,  .val = 0 },
        { .offset = 0x113020,  .val = 0 },
        { .offset = 0x213020,  .val = 0 },
        { .offset = 0x20072,  .val = 0 },
        { .offset = 0x20073,  .val = 0 },
        { .offset = 0x20074,  .val = 0 },
        { .offset = 0x100aa,  .val = 0 },
        { .offset = 0x110aa,  .val = 0 },
        { .offset = 0x120aa,  .val = 0 },
        { .offset = 0x130aa,  .val = 0 },
        { .offset = 0x20010,  .val = 0 },
        { .offset = 0x120010,  .val = 0 },
        { .offset = 0x220010,  .val = 0 },
        { .offset = 0x20011,  .val = 0 },
        { .offset = 0x120011,  .val = 0 },
        { .offset = 0x220011,  .val = 0 },
        { .offset = 0x100ae,  .val = 0 },
        { .offset = 0x1100ae,  .val = 0 },
        { .offset = 0x2100ae,  .val = 0 },
        { .offset = 0x100af,  .val = 0 },
        { .offset = 0x1100af,  .val = 0 },
        { .offset = 0x2100af,  .val = 0 },
        { .offset = 0x110ae,  .val = 0 },
        { .offset = 0x1110ae,  .val = 0 },
        { .offset = 0x2110ae,  .val = 0 },
        { .offset = 0x110af,  .val = 0 },
        { .offset = 0x1110af,  .val = 0 },
        { .offset = 0x2110af,  .val = 0 },
        { .offset = 0x120ae,  .val = 0 },
        { .offset = 0x1120ae,  .val = 0 },
        { .offset = 0x2120ae,  .val = 0 },
        { .offset = 0x120af,  .val = 0 },
        { .offset = 0x1120af,  .val = 0 },
        { .offset = 0x2120af,  .val = 0 },
        { .offset = 0x130ae,  .val = 0 },
        { .offset = 0x1130ae,  .val = 0 },
        { .offset = 0x2130ae,  .val = 0 },
        { .offset = 0x130af,  .val = 0 },
        { .offset = 0x1130af,  .val = 0 },
        { .offset = 0x2130af,  .val = 0 },
        { .offset = 0x20020,  .val = 0 },
        { .offset = 0x120020,  .val = 0 },
        { .offset = 0x220020,  .val = 0 },
        { .offset = 0x100a0,  .val = 0 },
        { .offset = 0x100a1,  .val = 0 },
        { .offset = 0x100a2,  .val = 0 },
        { .offset = 0x100a3,  .val = 0 },
        { .offset = 0x100a4,  .val = 0 },
        { .offset = 0x100a5,  .val = 0 },
        { .offset = 0x100a6,  .val = 0 },
        { .offset = 0x100a7,  .val = 0 },
        { .offset = 0x110a0,  .val = 0 },
        { .offset = 0x110a1,  .val = 0 },
        { .offset = 0x110a2,  .val = 0 },
        { .offset = 0x110a3,  .val = 0 },
        { .offset = 0x110a4,  .val = 0 },
        { .offset = 0x110a5,  .val = 0 },
        { .offset = 0x110a6,  .val = 0 },
        { .offset = 0x110a7,  .val = 0 },
        { .offset = 0x120a0,  .val = 0 },
        { .offset = 0x120a1,  .val = 0 },
        { .offset = 0x120a2,  .val = 0 },
        { .offset = 0x120a3,  .val = 0 },
        { .offset = 0x120a4,  .val = 0 },
        { .offset = 0x120a5,  .val = 0 },
        { .offset = 0x120a6,  .val = 0 },
        { .offset = 0x120a7,  .val = 0 },
        { .offset = 0x130a0,  .val = 0 },
        { .offset = 0x130a1,  .val = 0 },
        { .offset = 0x130a2,  .val = 0 },
        { .offset = 0x130a3,  .val = 0 },
        { .offset = 0x130a4,  .val = 0 },
        { .offset = 0x130a5,  .val = 0 },
        { .offset = 0x130a6,  .val = 0 },
        { .offset = 0x130a7,  .val = 0 },
        { .offset = 0x2007c,  .val = 0 },
        { .offset = 0x12007c,  .val = 0 },
        { .offset = 0x22007c,  .val = 0 },
        { .offset = 0x2007d,  .val = 0 },
        { .offset = 0x12007d,  .val = 0 },
        { .offset = 0x22007d,  .val = 0 },
        { .offset = 0x400fd,  .val = 0 },
        { .offset = 0x400c0,  .val = 0 },
        { .offset = 0x90201,  .val = 0 },
        { .offset = 0x190201,  .val = 0 },
        { .offset = 0x290201,  .val = 0 },
        { .offset = 0x90202,  .val = 0 },
        { .offset = 0x190202,  .val = 0 },
        { .offset = 0x290202,  .val = 0 },
        { .offset = 0x90203,  .val = 0 },
        { .offset = 0x190203,  .val = 0 },
        { .offset = 0x290203,  .val = 0 },
        { .offset = 0x90204,  .val = 0 },
        { .offset = 0x190204,  .val = 0 },
        { .offset = 0x290204,  .val = 0 },
        { .offset = 0x90205,  .val = 0 },
        { .offset = 0x190205,  .val = 0 },
        { .offset = 0x290205,  .val = 0 },
        { .offset = 0x90206,  .val = 0 },
        { .offset = 0x190206,  .val = 0 },
        { .offset = 0x290206,  .val = 0 },
        { .offset = 0x90207,  .val = 0 },
        { .offset = 0x190207,  .val = 0 },
        { .offset = 0x290207,  .val = 0 },
        { .offset = 0x90208,  .val = 0 },
        { .offset = 0x190208,  .val = 0 },
        { .offset = 0x290208,  .val = 0 },
        { .offset = 0x10062,  .val = 0 },
        { .offset = 0x10162,  .val = 0 },
        { .offset = 0x10262,  .val = 0 },
        { .offset = 0x10362,  .val = 0 },
        { .offset = 0x10462,  .val = 0 },
        { .offset = 0x10562,  .val = 0 },
        { .offset = 0x10662,  .val = 0 },
        { .offset = 0x10762,  .val = 0 },
        { .offset = 0x10862,  .val = 0 },
        { .offset = 0x11062,  .val = 0 },
        { .offset = 0x11162,  .val = 0 },
        { .offset = 0x11262,  .val = 0 },
        { .offset = 0x11362,  .val = 0 },
        { .offset = 0x11462,  .val = 0 },
        { .offset = 0x11562,  .val = 0 },
        { .offset = 0x11662,  .val = 0 },
        { .offset = 0x11762,  .val = 0 },
        { .offset = 0x11862,  .val = 0 },
        { .offset = 0x12062,  .val = 0 },
        { .offset = 0x12162,  .val = 0 },
        { .offset = 0x12262,  .val = 0 },
        { .offset = 0x12362,  .val = 0 },
        { .offset = 0x12462,  .val = 0 },
        { .offset = 0x12562,  .val = 0 },
        { .offset = 0x12662,  .val = 0 },
        { .offset = 0x12762,  .val = 0 },
        { .offset = 0x12862,  .val = 0 },
        { .offset = 0x13062,  .val = 0 },
        { .offset = 0x13162,  .val = 0 },
        { .offset = 0x13262,  .val = 0 },
        { .offset = 0x13362,  .val = 0 },
        { .offset = 0x13462,  .val = 0 },
        { .offset = 0x13562,  .val = 0 },
        { .offset = 0x13662,  .val = 0 },
        { .offset = 0x13762,  .val = 0 },
        { .offset = 0x13862,  .val = 0 },
        { .offset = 0x20077,  .val = 0 },
        { .offset = 0x10001,  .val = 0 },
        { .offset = 0x11001,  .val = 0 },
        { .offset = 0x12001,  .val = 0 },
        { .offset = 0x13001,  .val = 0 },
        { .offset = 0x10040,  .val = 0 },
        { .offset = 0x10140,  .val = 0 },
        { .offset = 0x10240,  .val = 0 },
        { .offset = 0x10340,  .val = 0 },
        { .offset = 0x10440,  .val = 0 },
        { .offset = 0x10540,  .val = 0 },
        { .offset = 0x10640,  .val = 0 },
        { .offset = 0x10740,  .val = 0 },
        { .offset = 0x10840,  .val = 0 },
        { .offset = 0x10030,  .val = 0 },
        { .offset = 0x10130,  .val = 0 },
        { .offset = 0x10230,  .val = 0 },
        { .offset = 0x10330,  .val = 0 },
        { .offset = 0x10430,  .val = 0 },
        { .offset = 0x10530,  .val = 0 },
        { .offset = 0x10630,  .val = 0 },
        { .offset = 0x10730,  .val = 0 },
        { .offset = 0x10830,  .val = 0 },
        { .offset = 0x11040,  .val = 0 },
        { .offset = 0x11140,  .val = 0 },
        { .offset = 0x11240,  .val = 0 },
        { .offset = 0x11340,  .val = 0 },
        { .offset = 0x11440,  .val = 0 },
        { .offset = 0x11540,  .val = 0 },
        { .offset = 0x11640,  .val = 0 },
        { .offset = 0x11740,  .val = 0 },
        { .offset = 0x11840,  .val = 0 },
        { .offset = 0x11030,  .val = 0 },
        { .offset = 0x11130,  .val = 0 },
        { .offset = 0x11230,  .val = 0 },
        { .offset = 0x11330,  .val = 0 },
        { .offset = 0x11430,  .val = 0 },
        { .offset = 0x11530,  .val = 0 },
        { .offset = 0x11630,  .val = 0 },
        { .offset = 0x11730,  .val = 0 },
        { .offset = 0x11830,  .val = 0 },
        { .offset = 0x12040,  .val = 0 },
        { .offset = 0x12140,  .val = 0 },
        { .offset = 0x12240,  .val = 0 },
        { .offset = 0x12340,  .val = 0 },
        { .offset = 0x12440,  .val = 0 },
        { .offset = 0x12540,  .val = 0 },
        { .offset = 0x12640,  .val = 0 },
        { .offset = 0x12740,  .val = 0 },
        { .offset = 0x12840,  .val = 0 },
        { .offset = 0x12030,  .val = 0 },
        { .offset = 0x12130,  .val = 0 },
        { .offset = 0x12230,  .val = 0 },
        { .offset = 0x12330,  .val = 0 },
        { .offset = 0x12430,  .val = 0 },
        { .offset = 0x12530,  .val = 0 },
        { .offset = 0x12630,  .val = 0 },
        { .offset = 0x12730,  .val = 0 },
        { .offset = 0x12830,  .val = 0 },
        { .offset = 0x13040,  .val = 0 },
        { .offset = 0x13140,  .val = 0 },
        { .offset = 0x13240,  .val = 0 },
        { .offset = 0x13340,  .val = 0 },
        { .offset = 0x13440,  .val = 0 },
        { .offset = 0x13540,  .val = 0 },
        { .offset = 0x13640,  .val = 0 },
        { .offset = 0x13740,  .val = 0 },
        { .offset = 0x13840,  .val = 0 },
        { .offset = 0x13030,  .val = 0 },
        { .offset = 0x13130,  .val = 0 },
        { .offset = 0x13230,  .val = 0 },
        { .offset = 0x13330,  .val = 0 },
        { .offset = 0x13430,  .val = 0 },
        { .offset = 0x13530,  .val = 0 },
        { .offset = 0x13630,  .val = 0 },
        { .offset = 0x13730,  .val = 0 },
        { .offset = 0x13830,  .val = 0 },
};

/* lpddr4 phy PIE image */
static const struct ddrphy_cfg_param phy_pie[] = {
	{ .offset = 0xd0000, .val = 0x0 }, // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
	{ .offset = 0x90000, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s0
	{ .offset = 0x90001, .val = 0x400 }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s1
	{ .offset = 0x90002, .val = 0x10e }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b0s2
	{ .offset = 0x90003, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s0
	{ .offset = 0x90004, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s1
	{ .offset = 0x90005, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_PreSequenceReg0b1s2
	{ .offset = 0x90029, .val = 0xb }, // DWC_DDRPHYA_INITENG0_SequenceReg0b0s0
	{ .offset = 0x9002a, .val = 0x480 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b0s1
	{ .offset = 0x9002b, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b0s2
	{ .offset = 0x9002c, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b1s0
	{ .offset = 0x9002d, .val = 0x448 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b1s1
	{ .offset = 0x9002e, .val = 0x139 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b1s2
	{ .offset = 0x9002f, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b2s0
	{ .offset = 0x90030, .val = 0x478 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b2s1
	{ .offset = 0x90031, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b2s2
	{ .offset = 0x90032, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b3s0
	{ .offset = 0x90033, .val = 0xe8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b3s1
	{ .offset = 0x90034, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b3s2
	{ .offset = 0x90035, .val = 0x2 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b4s0
	{ .offset = 0x90036, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b4s1
	{ .offset = 0x90037, .val = 0x139 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b4s2
	{ .offset = 0x90038, .val = 0xb }, // DWC_DDRPHYA_INITENG0_SequenceReg0b5s0
	{ .offset = 0x90039, .val = 0x7c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b5s1
	{ .offset = 0x9003a, .val = 0x139 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b5s2
	{ .offset = 0x9003b, .val = 0x44 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b6s0
	{ .offset = 0x9003c, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b6s1
	{ .offset = 0x9003d, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b6s2
	{ .offset = 0x9003e, .val = 0x14f }, // DWC_DDRPHYA_INITENG0_SequenceReg0b7s0
	{ .offset = 0x9003f, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b7s1
	{ .offset = 0x90040, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b7s2
	{ .offset = 0x90041, .val = 0x47 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b8s0
	{ .offset = 0x90042, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b8s1
	{ .offset = 0x90043, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b8s2
	{ .offset = 0x90044, .val = 0x4f }, // DWC_DDRPHYA_INITENG0_SequenceReg0b9s0
	{ .offset = 0x90045, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b9s1
	{ .offset = 0x90046, .val = 0x179 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b9s2
	{ .offset = 0x90047, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b10s0
	{ .offset = 0x90048, .val = 0xe0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b10s1
	{ .offset = 0x90049, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b10s2
	{ .offset = 0x9004a, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b11s0
	{ .offset = 0x9004b, .val = 0x7c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b11s1
	{ .offset = 0x9004c, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b11s2
	{ .offset = 0x9004d, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b12s0
	{ .offset = 0x9004e, .val = 0x1 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b12s1
	{ .offset = 0x9004f, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b12s2
	{ .offset = 0x90050, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b13s0
	{ .offset = 0x90051, .val = 0x45a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b13s1
	{ .offset = 0x90052, .val = 0x9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b13s2
	{ .offset = 0x90053, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b14s0
	{ .offset = 0x90054, .val = 0x448 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b14s1
	{ .offset = 0x90055, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b14s2
	{ .offset = 0x90056, .val = 0x40 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b15s0
	{ .offset = 0x90057, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b15s1
	{ .offset = 0x90058, .val = 0x179 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b15s2
	{ .offset = 0x90059, .val = 0x1 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b16s0
	{ .offset = 0x9005a, .val = 0x618 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b16s1
	{ .offset = 0x9005b, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b16s2
	{ .offset = 0x9005c, .val = 0x40c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b17s0
	{ .offset = 0x9005d, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b17s1
	{ .offset = 0x9005e, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b17s2
	{ .offset = 0x9005f, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b18s0
	{ .offset = 0x90060, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b18s1
	{ .offset = 0x90061, .val = 0x48 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b18s2
	{ .offset = 0x90062, .val = 0x4040 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b19s0
	{ .offset = 0x90063, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b19s1
	{ .offset = 0x90064, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b19s2
	{ .offset = 0x90065, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b20s0
	{ .offset = 0x90066, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b20s1
	{ .offset = 0x90067, .val = 0x48 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b20s2
	{ .offset = 0x90068, .val = 0x40 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b21s0
	{ .offset = 0x90069, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b21s1
	{ .offset = 0x9006a, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b21s2
	{ .offset = 0x9006b, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b22s0
	{ .offset = 0x9006c, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b22s1
	{ .offset = 0x9006d, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b22s2
	{ .offset = 0x9006e, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b23s0
	{ .offset = 0x9006f, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b23s1
	{ .offset = 0x90070, .val = 0x78 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b23s2
	{ .offset = 0x90071, .val = 0x549 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b24s0
	{ .offset = 0x90072, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b24s1
	{ .offset = 0x90073, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b24s2
	{ .offset = 0x90074, .val = 0xd49 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b25s0
	{ .offset = 0x90075, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b25s1
	{ .offset = 0x90076, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b25s2
	{ .offset = 0x90077, .val = 0x94a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b26s0
	{ .offset = 0x90078, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b26s1
	{ .offset = 0x90079, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b26s2
	{ .offset = 0x9007a, .val = 0x441 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b27s0
	{ .offset = 0x9007b, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b27s1
	{ .offset = 0x9007c, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b27s2
	{ .offset = 0x9007d, .val = 0x42 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b28s0
	{ .offset = 0x9007e, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b28s1
	{ .offset = 0x9007f, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b28s2
	{ .offset = 0x90080, .val = 0x1 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b29s0
	{ .offset = 0x90081, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b29s1
	{ .offset = 0x90082, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b29s2
	{ .offset = 0x90083, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b30s0
	{ .offset = 0x90084, .val = 0xe0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b30s1
	{ .offset = 0x90085, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b30s2
	{ .offset = 0x90086, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b31s0
	{ .offset = 0x90087, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b31s1
	{ .offset = 0x90088, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b31s2
	{ .offset = 0x90089, .val = 0x9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b32s0
	{ .offset = 0x9008a, .val = 0x3c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b32s1
	{ .offset = 0x9008b, .val = 0x149 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b32s2
	{ .offset = 0x9008c, .val = 0x9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b33s0
	{ .offset = 0x9008d, .val = 0x3c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b33s1
	{ .offset = 0x9008e, .val = 0x159 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b33s2
	{ .offset = 0x9008f, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b34s0
	{ .offset = 0x90090, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b34s1
	{ .offset = 0x90091, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b34s2
	{ .offset = 0x90092, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b35s0
	{ .offset = 0x90093, .val = 0x3c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b35s1
	{ .offset = 0x90094, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b35s2
	{ .offset = 0x90095, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b36s0
	{ .offset = 0x90096, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b36s1
	{ .offset = 0x90097, .val = 0x48 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b36s2
	{ .offset = 0x90098, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b37s0
	{ .offset = 0x90099, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b37s1
	{ .offset = 0x9009a, .val = 0x58 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b37s2
	{ .offset = 0x9009b, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b38s0
	{ .offset = 0x9009c, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b38s1
	{ .offset = 0x9009d, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b38s2
	{ .offset = 0x9009e, .val = 0x2 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b39s0
	{ .offset = 0x9009f, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b39s1
	{ .offset = 0x900a0, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b39s2
	{ .offset = 0x900a1, .val = 0x5 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b40s0
	{ .offset = 0x900a2, .val = 0x7c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b40s1
	{ .offset = 0x900a3, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b40s2
	{ .offset = 0x900a4, .val = 0xd }, // DWC_DDRPHYA_INITENG0_SequenceReg0b41s0
	{ .offset = 0x900a5, .val = 0x7c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b41s1
	{ .offset = 0x900a6, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b41s2
	{ .offset = 0x900a7, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b42s0
	{ .offset = 0x900a8, .val = 0x7c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b42s1
	{ .offset = 0x900a9, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b42s2
	{ .offset = 0x40000, .val = 0x811 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x0
	{ .offset = 0x40020, .val = 0x880 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x0
	{ .offset = 0x40040, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x0
	{ .offset = 0x40060, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x0
	{ .offset = 0x40001, .val = 0x4008 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x1
	{ .offset = 0x40021, .val = 0x83 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x1
	{ .offset = 0x40041, .val = 0x4f }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x1
	{ .offset = 0x40061, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x1
	{ .offset = 0x40002, .val = 0x4040 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x2
	{ .offset = 0x40022, .val = 0x83 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x2
	{ .offset = 0x40042, .val = 0x51 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x2
	{ .offset = 0x40062, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x2
	{ .offset = 0x40003, .val = 0x811 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x3
	{ .offset = 0x40023, .val = 0x880 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x3
	{ .offset = 0x40043, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x3
	{ .offset = 0x40063, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x3
	{ .offset = 0x40004, .val = 0x720 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x4
	{ .offset = 0x40024, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x4
	{ .offset = 0x40044, .val = 0x1740 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x4
	{ .offset = 0x40064, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x4
	{ .offset = 0x40005, .val = 0x16 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x5
	{ .offset = 0x40025, .val = 0x83 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x5
	{ .offset = 0x40045, .val = 0x4b }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x5
	{ .offset = 0x40065, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x5
	{ .offset = 0x40006, .val = 0x716 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x6
	{ .offset = 0x40026, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x6
	{ .offset = 0x40046, .val = 0x2001 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x6
	{ .offset = 0x40066, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x6
	{ .offset = 0x40007, .val = 0x716 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x7
	{ .offset = 0x40027, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x7
	{ .offset = 0x40047, .val = 0x2800 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x7
	{ .offset = 0x40067, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x7
	{ .offset = 0x40008, .val = 0x716 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x8
	{ .offset = 0x40028, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x8
	{ .offset = 0x40048, .val = 0xf00 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x8
	{ .offset = 0x40068, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x8
	{ .offset = 0x40009, .val = 0x720 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x9
	{ .offset = 0x40029, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x9
	{ .offset = 0x40049, .val = 0x1400 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x9
	{ .offset = 0x40069, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x9
	{ .offset = 0x4000a, .val = 0xe08 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x10
	{ .offset = 0x4002a, .val = 0xc15 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x10
	{ .offset = 0x4004a, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x10
	{ .offset = 0x4006a, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x10
	{ .offset = 0x4000b, .val = 0x623 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x11
	{ .offset = 0x4002b, .val = 0x15 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x11
	{ .offset = 0x4004b, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x11
	{ .offset = 0x4006b, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x11
	{ .offset = 0x4000c, .val = 0x4028 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x12
	{ .offset = 0x4002c, .val = 0x80 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x12
	{ .offset = 0x4004c, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x12
	{ .offset = 0x4006c, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x12
	{ .offset = 0x4000d, .val = 0xe08 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x13
	{ .offset = 0x4002d, .val = 0xc1a }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x13
	{ .offset = 0x4004d, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x13
	{ .offset = 0x4006d, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x13
	{ .offset = 0x4000e, .val = 0x623 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x14
	{ .offset = 0x4002e, .val = 0x1a }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x14
	{ .offset = 0x4004e, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x14
	{ .offset = 0x4006e, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x14
	{ .offset = 0x4000f, .val = 0x4040 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x15
	{ .offset = 0x4002f, .val = 0x80 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x15
	{ .offset = 0x4004f, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x15
	{ .offset = 0x4006f, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x15
	{ .offset = 0x40010, .val = 0x2604 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x16
	{ .offset = 0x40030, .val = 0x15 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x16
	{ .offset = 0x40050, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x16
	{ .offset = 0x40070, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x16
	{ .offset = 0x40011, .val = 0x708 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x17
	{ .offset = 0x40031, .val = 0x5 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x17
	{ .offset = 0x40051, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x17
	{ .offset = 0x40071, .val = 0x2002 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x17
	{ .offset = 0x40012, .val = 0x8 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x18
	{ .offset = 0x40032, .val = 0x80 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x18
	{ .offset = 0x40052, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x18
	{ .offset = 0x40072, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x18
	{ .offset = 0x40013, .val = 0x2604 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x19
	{ .offset = 0x40033, .val = 0x1a }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x19
	{ .offset = 0x40053, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x19
	{ .offset = 0x40073, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x19
	{ .offset = 0x40014, .val = 0x708 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x20
	{ .offset = 0x40034, .val = 0xa }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x20
	{ .offset = 0x40054, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x20
	{ .offset = 0x40074, .val = 0x2002 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x20
	{ .offset = 0x40015, .val = 0x4040 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x21
	{ .offset = 0x40035, .val = 0x80 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x21
	{ .offset = 0x40055, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x21
	{ .offset = 0x40075, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x21
	{ .offset = 0x40016, .val = 0x60a }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x22
	{ .offset = 0x40036, .val = 0x15 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x22
	{ .offset = 0x40056, .val = 0x1200 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x22
	{ .offset = 0x40076, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x22
	{ .offset = 0x40017, .val = 0x61a }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x23
	{ .offset = 0x40037, .val = 0x15 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x23
	{ .offset = 0x40057, .val = 0x1300 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x23
	{ .offset = 0x40077, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x23
	{ .offset = 0x40018, .val = 0x60a }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x24
	{ .offset = 0x40038, .val = 0x1a }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x24
	{ .offset = 0x40058, .val = 0x1200 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x24
	{ .offset = 0x40078, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x24
	{ .offset = 0x40019, .val = 0x642 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x25
	{ .offset = 0x40039, .val = 0x1a }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x25
	{ .offset = 0x40059, .val = 0x1300 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x25
	{ .offset = 0x40079, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x25
	{ .offset = 0x4001a, .val = 0x4808 }, // DWC_DDRPHYA_ACSM0_AcsmSeq0x26
	{ .offset = 0x4003a, .val = 0x880 }, // DWC_DDRPHYA_ACSM0_AcsmSeq1x26
	{ .offset = 0x4005a, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq2x26
	{ .offset = 0x4007a, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmSeq3x26
	{ .offset = 0x900aa, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b43s0
	{ .offset = 0x900ab, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b43s1
	{ .offset = 0x900ac, .val = 0x11a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b43s2
	{ .offset = 0x900ad, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b44s0
	{ .offset = 0x900ae, .val = 0x7aa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b44s1
	{ .offset = 0x900af, .val = 0x2a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b44s2
	{ .offset = 0x900b0, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b45s0
	{ .offset = 0x900b1, .val = 0x7b2 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b45s1
	{ .offset = 0x900b2, .val = 0x2a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b45s2
	{ .offset = 0x900b3, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b46s0
	{ .offset = 0x900b4, .val = 0x7c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b46s1
	{ .offset = 0x900b5, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b46s2
	{ .offset = 0x900b6, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b47s0
	{ .offset = 0x900b7, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b47s1
	{ .offset = 0x900b8, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b47s2
	{ .offset = 0x900b9, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b48s0
	{ .offset = 0x900ba, .val = 0x2a8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b48s1
	{ .offset = 0x900bb, .val = 0x129 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b48s2
	{ .offset = 0x900bc, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b49s0
	{ .offset = 0x900bd, .val = 0x370 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b49s1
	{ .offset = 0x900be, .val = 0x129 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b49s2
	{ .offset = 0x900bf, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b50s0
	{ .offset = 0x900c0, .val = 0x3c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b50s1
	{ .offset = 0x900c1, .val = 0x1a9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b50s2
	{ .offset = 0x900c2, .val = 0xc }, // DWC_DDRPHYA_INITENG0_SequenceReg0b51s0
	{ .offset = 0x900c3, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b51s1
	{ .offset = 0x900c4, .val = 0x199 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b51s2
	{ .offset = 0x900c5, .val = 0x14 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b52s0
	{ .offset = 0x900c6, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b52s1
	{ .offset = 0x900c7, .val = 0x11a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b52s2
	{ .offset = 0x900c8, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b53s0
	{ .offset = 0x900c9, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b53s1
	{ .offset = 0x900ca, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b53s2
	{ .offset = 0x900cb, .val = 0xe }, // DWC_DDRPHYA_INITENG0_SequenceReg0b54s0
	{ .offset = 0x900cc, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b54s1
	{ .offset = 0x900cd, .val = 0x199 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b54s2
	{ .offset = 0x900ce, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b55s0
	{ .offset = 0x900cf, .val = 0x8568 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b55s1
	{ .offset = 0x900d0, .val = 0x108 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b55s2
	{ .offset = 0x900d1, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b56s0
	{ .offset = 0x900d2, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b56s1
	{ .offset = 0x900d3, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b56s2
	{ .offset = 0x900d4, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b57s0
	{ .offset = 0x900d5, .val = 0x1d8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b57s1
	{ .offset = 0x900d6, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b57s2
	{ .offset = 0x900d7, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b58s0
	{ .offset = 0x900d8, .val = 0x8558 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b58s1
	{ .offset = 0x900d9, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b58s2
	{ .offset = 0x900da, .val = 0x70 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b59s0
	{ .offset = 0x900db, .val = 0x788 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b59s1
	{ .offset = 0x900dc, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b59s2
	{ .offset = 0x900dd, .val = 0x1ff8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b60s0
	{ .offset = 0x900de, .val = 0x85a8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b60s1
	{ .offset = 0x900df, .val = 0x1e8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b60s2
	{ .offset = 0x900e0, .val = 0x50 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b61s0
	{ .offset = 0x900e1, .val = 0x798 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b61s1
	{ .offset = 0x900e2, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b61s2
	{ .offset = 0x900e3, .val = 0x60 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b62s0
	{ .offset = 0x900e4, .val = 0x7a0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b62s1
	{ .offset = 0x900e5, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b62s2
	{ .offset = 0x900e6, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b63s0
	{ .offset = 0x900e7, .val = 0x8310 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b63s1
	{ .offset = 0x900e8, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b63s2
	{ .offset = 0x900e9, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b64s0
	{ .offset = 0x900ea, .val = 0xa310 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b64s1
	{ .offset = 0x900eb, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b64s2
	{ .offset = 0x900ec, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b65s0
	{ .offset = 0x900ed, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b65s1
	{ .offset = 0x900ee, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b65s2
	{ .offset = 0x900ef, .val = 0x6e }, // DWC_DDRPHYA_INITENG0_SequenceReg0b66s0
	{ .offset = 0x900f0, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b66s1
	{ .offset = 0x900f1, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b66s2
	{ .offset = 0x900f2, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b67s0
	{ .offset = 0x900f3, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b67s1
	{ .offset = 0x900f4, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b67s2
	{ .offset = 0x900f5, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b68s0
	{ .offset = 0x900f6, .val = 0x8310 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b68s1
	{ .offset = 0x900f7, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b68s2
	{ .offset = 0x900f8, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b69s0
	{ .offset = 0x900f9, .val = 0xa310 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b69s1
	{ .offset = 0x900fa, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b69s2
	{ .offset = 0x900fb, .val = 0x1ff8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b70s0
	{ .offset = 0x900fc, .val = 0x85a8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b70s1
	{ .offset = 0x900fd, .val = 0x1e8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b70s2
	{ .offset = 0x900fe, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b71s0
	{ .offset = 0x900ff, .val = 0x798 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b71s1
	{ .offset = 0x90100, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b71s2
	{ .offset = 0x90101, .val = 0x78 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b72s0
	{ .offset = 0x90102, .val = 0x7a0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b72s1
	{ .offset = 0x90103, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b72s2
	{ .offset = 0x90104, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b73s0
	{ .offset = 0x90105, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b73s1
	{ .offset = 0x90106, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b73s2
	{ .offset = 0x90107, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b74s0
	{ .offset = 0x90108, .val = 0x8b10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b74s1
	{ .offset = 0x90109, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b74s2
	{ .offset = 0x9010a, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b75s0
	{ .offset = 0x9010b, .val = 0xab10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b75s1
	{ .offset = 0x9010c, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b75s2
	{ .offset = 0x9010d, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b76s0
	{ .offset = 0x9010e, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b76s1
	{ .offset = 0x9010f, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b76s2
	{ .offset = 0x90110, .val = 0x58 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b77s0
	{ .offset = 0x90111, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b77s1
	{ .offset = 0x90112, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b77s2
	{ .offset = 0x90113, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b78s0
	{ .offset = 0x90114, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b78s1
	{ .offset = 0x90115, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b78s2
	{ .offset = 0x90116, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b79s0
	{ .offset = 0x90117, .val = 0x8b10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b79s1
	{ .offset = 0x90118, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b79s2
	{ .offset = 0x90119, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b80s0
	{ .offset = 0x9011a, .val = 0xab10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b80s1
	{ .offset = 0x9011b, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b80s2
	{ .offset = 0x9011c, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b81s0
	{ .offset = 0x9011d, .val = 0x1d8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b81s1
	{ .offset = 0x9011e, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b81s2
	{ .offset = 0x9011f, .val = 0x80 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b82s0
	{ .offset = 0x90120, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b82s1
	{ .offset = 0x90121, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b82s2
	{ .offset = 0x90122, .val = 0x18 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b83s0
	{ .offset = 0x90123, .val = 0x7aa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b83s1
	{ .offset = 0x90124, .val = 0x6a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b83s2
	{ .offset = 0x90125, .val = 0xa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b84s0
	{ .offset = 0x90126, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b84s1
	{ .offset = 0x90127, .val = 0x1e9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b84s2
	{ .offset = 0x90128, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b85s0
	{ .offset = 0x90129, .val = 0x8080 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b85s1
	{ .offset = 0x9012a, .val = 0x108 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b85s2
	{ .offset = 0x9012b, .val = 0xf }, // DWC_DDRPHYA_INITENG0_SequenceReg0b86s0
	{ .offset = 0x9012c, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b86s1
	{ .offset = 0x9012d, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b86s2
	{ .offset = 0x9012e, .val = 0xc }, // DWC_DDRPHYA_INITENG0_SequenceReg0b87s0
	{ .offset = 0x9012f, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b87s1
	{ .offset = 0x90130, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b87s2
	{ .offset = 0x90131, .val = 0x9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b88s0
	{ .offset = 0x90132, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b88s1
	{ .offset = 0x90133, .val = 0x1a9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b88s2
	{ .offset = 0x90134, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b89s0
	{ .offset = 0x90135, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b89s1
	{ .offset = 0x90136, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b89s2
	{ .offset = 0x90137, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b90s0
	{ .offset = 0x90138, .val = 0x8080 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b90s1
	{ .offset = 0x90139, .val = 0x108 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b90s2
	{ .offset = 0x9013a, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b91s0
	{ .offset = 0x9013b, .val = 0x7aa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b91s1
	{ .offset = 0x9013c, .val = 0x6a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b91s2
	{ .offset = 0x9013d, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b92s0
	{ .offset = 0x9013e, .val = 0x8568 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b92s1
	{ .offset = 0x9013f, .val = 0x108 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b92s2
	{ .offset = 0x90140, .val = 0xb7 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b93s0
	{ .offset = 0x90141, .val = 0x790 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b93s1
	{ .offset = 0x90142, .val = 0x16a }, // DWC_DDRPHYA_INITENG0_SequenceReg0b93s2
	{ .offset = 0x90143, .val = 0x1f }, // DWC_DDRPHYA_INITENG0_SequenceReg0b94s0
	{ .offset = 0x90144, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b94s1
	{ .offset = 0x90145, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b94s2
	{ .offset = 0x90146, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b95s0
	{ .offset = 0x90147, .val = 0x8558 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b95s1
	{ .offset = 0x90148, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b95s2
	{ .offset = 0x90149, .val = 0xf }, // DWC_DDRPHYA_INITENG0_SequenceReg0b96s0
	{ .offset = 0x9014a, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b96s1
	{ .offset = 0x9014b, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b96s2
	{ .offset = 0x9014c, .val = 0xc }, // DWC_DDRPHYA_INITENG0_SequenceReg0b97s0
	{ .offset = 0x9014d, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b97s1
	{ .offset = 0x9014e, .val = 0x68 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b97s2
	{ .offset = 0x9014f, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b98s0
	{ .offset = 0x90150, .val = 0x408 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b98s1
	{ .offset = 0x90151, .val = 0x169 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b98s2
	{ .offset = 0x90152, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b99s0
	{ .offset = 0x90153, .val = 0x8558 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b99s1
	{ .offset = 0x90154, .val = 0x168 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b99s2
	{ .offset = 0x90155, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b100s0
	{ .offset = 0x90156, .val = 0x3c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b100s1
	{ .offset = 0x90157, .val = 0x1a9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b100s2
	{ .offset = 0x90158, .val = 0x3 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b101s0
	{ .offset = 0x90159, .val = 0x370 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b101s1
	{ .offset = 0x9015a, .val = 0x129 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b101s2
	{ .offset = 0x9015b, .val = 0x20 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b102s0
	{ .offset = 0x9015c, .val = 0x2aa }, // DWC_DDRPHYA_INITENG0_SequenceReg0b102s1
	{ .offset = 0x9015d, .val = 0x9 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b102s2
	{ .offset = 0x9015e, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b103s0
	{ .offset = 0x9015f, .val = 0x400 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b103s1
	{ .offset = 0x90160, .val = 0x10e }, // DWC_DDRPHYA_INITENG0_SequenceReg0b103s2
	{ .offset = 0x90161, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b104s0
	{ .offset = 0x90162, .val = 0xe8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b104s1
	{ .offset = 0x90163, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b104s2
	{ .offset = 0x90164, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b105s0
	{ .offset = 0x90165, .val = 0x8140 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b105s1
	{ .offset = 0x90166, .val = 0x10c }, // DWC_DDRPHYA_INITENG0_SequenceReg0b105s2
	{ .offset = 0x90167, .val = 0x10 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b106s0
	{ .offset = 0x90168, .val = 0x8138 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b106s1
	{ .offset = 0x90169, .val = 0x10c }, // DWC_DDRPHYA_INITENG0_SequenceReg0b106s2
	{ .offset = 0x9016a, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b107s0
	{ .offset = 0x9016b, .val = 0x7c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b107s1
	{ .offset = 0x9016c, .val = 0x101 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b107s2
	{ .offset = 0x9016d, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b108s0
	{ .offset = 0x9016e, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b108s1
	{ .offset = 0x9016f, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b108s2
	{ .offset = 0x90170, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b109s0
	{ .offset = 0x90171, .val = 0x448 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b109s1
	{ .offset = 0x90172, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b109s2
	{ .offset = 0x90173, .val = 0xf }, // DWC_DDRPHYA_INITENG0_SequenceReg0b110s0
	{ .offset = 0x90174, .val = 0x7c0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b110s1
	{ .offset = 0x90175, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b110s2
	{ .offset = 0x90176, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b111s0
	{ .offset = 0x90177, .val = 0xe8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b111s1
	{ .offset = 0x90178, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b111s2
	{ .offset = 0x90179, .val = 0x47 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b112s0
	{ .offset = 0x9017a, .val = 0x630 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b112s1
	{ .offset = 0x9017b, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b112s2
	{ .offset = 0x9017c, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b113s0
	{ .offset = 0x9017d, .val = 0x618 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b113s1
	{ .offset = 0x9017e, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b113s2
	{ .offset = 0x9017f, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b114s0
	{ .offset = 0x90180, .val = 0xe0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b114s1
	{ .offset = 0x90181, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b114s2
	{ .offset = 0x90182, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b115s0
	{ .offset = 0x90183, .val = 0x7c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b115s1
	{ .offset = 0x90184, .val = 0x109 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b115s2
	{ .offset = 0x90185, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b116s0
	{ .offset = 0x90186, .val = 0x8140 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b116s1
	{ .offset = 0x90187, .val = 0x10c }, // DWC_DDRPHYA_INITENG0_SequenceReg0b116s2
	{ .offset = 0x90188, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b117s0
	{ .offset = 0x90189, .val = 0x1 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b117s1
	{ .offset = 0x9018a, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b117s2
	{ .offset = 0x9018b, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b118s0
	{ .offset = 0x9018c, .val = 0x4 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b118s1
	{ .offset = 0x9018d, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b118s2
	{ .offset = 0x9018e, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b119s0
	{ .offset = 0x9018f, .val = 0x7c8 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b119s1
	{ .offset = 0x90190, .val = 0x101 }, // DWC_DDRPHYA_INITENG0_SequenceReg0b119s2
	{ .offset = 0x90006, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s0
	{ .offset = 0x90007, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s1
	{ .offset = 0x90008, .val = 0x8 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b0s2
	{ .offset = 0x90009, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s0
	{ .offset = 0x9000a, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s1
	{ .offset = 0x9000b, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_PostSequenceReg0b1s2
	{ .offset = 0xd00e7, .val = 0x400 }, // DWC_DDRPHYA_APBONLY0_SequencerOverride
	{ .offset = 0x90017, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_StartVector0b0
	{ .offset = 0x9001f, .val = 0x2b }, // DWC_DDRPHYA_INITENG0_StartVector0b8
	{ .offset = 0x90026, .val = 0x6c }, // DWC_DDRPHYA_INITENG0_StartVector0b15
	{ .offset = 0x400d0, .val = 0x0 }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl0
	{ .offset = 0x400d1, .val = 0x101 }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl1
	{ .offset = 0x400d2, .val = 0x105 }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl2
	{ .offset = 0x400d3, .val = 0x107 }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl3
	{ .offset = 0x400d4, .val = 0x10f }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl4
	{ .offset = 0x400d5, .val = 0x202 }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl5
	{ .offset = 0x400d6, .val = 0x20a }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl6
	{ .offset = 0x400d7, .val = 0x20b }, // DWC_DDRPHYA_ACSM0_AcsmCsMapCtrl7
	{ .offset = 0x2003a, .val = 0x2 }, // DWC_DDRPHYA_MASTER0_DbyteDllModeCntrl
	{ .offset = 0x2000b, .val = 0x64 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p0
	{ .offset = 0x2000c, .val = 0xc8 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p0
	{ .offset = 0x2000d, .val = 0x7d0 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p0
	{ .offset = 0x2000e, .val = 0x2c }, // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p0
	{ .offset = 0x12000b, .val = 0x14 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p1
	{ .offset = 0x12000c, .val = 0x29 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p1
	{ .offset = 0x12000d, .val = 0x1a1 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p1
	{ .offset = 0x12000e, .val = 0x10 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p1
	{ .offset = 0x22000b, .val = 0x3 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY0_p2
	{ .offset = 0x22000c, .val = 0x6 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY1_p2
	{ .offset = 0x22000d, .val = 0x3e }, // DWC_DDRPHYA_MASTER0_Seq0BDLY2_p2
	{ .offset = 0x22000e, .val = 0x10 }, // DWC_DDRPHYA_MASTER0_Seq0BDLY3_p2
	{ .offset = 0x9000c, .val = 0x0 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag0
	{ .offset = 0x9000d, .val = 0x173 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag1
	{ .offset = 0x9000e, .val = 0x60 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag2
	{ .offset = 0x9000f, .val = 0x6110 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag3
	{ .offset = 0x90010, .val = 0x2152 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag4
	{ .offset = 0x90011, .val = 0xdfbd }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag5
	{ .offset = 0x90012, .val = 0x60 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag6
	{ .offset = 0x90013, .val = 0x6152 }, // DWC_DDRPHYA_INITENG0_Seq0BDisableFlag7
	{ .offset = 0x20010, .val = 0x5a }, // DWC_DDRPHYA_MASTER0_PPTTrainSetup_p0
	{ .offset = 0x20011, .val = 0x3 }, // DWC_DDRPHYA_MASTER0_PPTTrainSetup2_p0
	{ .offset = 0x40080, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p0
	{ .offset = 0x40081, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p0
	{ .offset = 0x40082, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p0
	{ .offset = 0x40083, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p0
	{ .offset = 0x40084, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p0
	{ .offset = 0x40085, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p0
	{ .offset = 0x140080, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p1
	{ .offset = 0x140081, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p1
	{ .offset = 0x140082, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p1
	{ .offset = 0x140083, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p1
	{ .offset = 0x140084, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p1
	{ .offset = 0x140085, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p1
	{ .offset = 0x240080, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x0_p2
	{ .offset = 0x240081, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x0_p2
	{ .offset = 0x240082, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x1_p2
	{ .offset = 0x240083, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x1_p2
	{ .offset = 0x240084, .val = 0xe0 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback0x2_p2
	{ .offset = 0x240085, .val = 0x12 }, // DWC_DDRPHYA_ACSM0_AcsmPlayback1x2_p2
	{ .offset = 0x400fd, .val = 0xf }, // DWC_DDRPHYA_ACSM0_AcsmCtrl13
	{ .offset = 0x10011, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_TsmByte1
	{ .offset = 0x10012, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_TsmByte2
	{ .offset = 0x10013, .val = 0x180 }, // DWC_DDRPHYA_DBYTE0_TsmByte3
	{ .offset = 0x10018, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_TsmByte5
	{ .offset = 0x10002, .val = 0x6209 }, // DWC_DDRPHYA_DBYTE0_TrainingParam
	{ .offset = 0x100b2, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm0_i0
	{ .offset = 0x101b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i1
	{ .offset = 0x102b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i2
	{ .offset = 0x103b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i3
	{ .offset = 0x104b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i4
	{ .offset = 0x105b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i5
	{ .offset = 0x106b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i6
	{ .offset = 0x107b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i7
	{ .offset = 0x108b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE0_Tsm2_i8
	{ .offset = 0x11011, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_TsmByte1
	{ .offset = 0x11012, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_TsmByte2
	{ .offset = 0x11013, .val = 0x180 }, // DWC_DDRPHYA_DBYTE1_TsmByte3
	{ .offset = 0x11018, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_TsmByte5
	{ .offset = 0x11002, .val = 0x6209 }, // DWC_DDRPHYA_DBYTE1_TrainingParam
	{ .offset = 0x110b2, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm0_i0
	{ .offset = 0x111b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i1
	{ .offset = 0x112b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i2
	{ .offset = 0x113b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i3
	{ .offset = 0x114b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i4
	{ .offset = 0x115b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i5
	{ .offset = 0x116b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i6
	{ .offset = 0x117b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i7
	{ .offset = 0x118b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE1_Tsm2_i8
	{ .offset = 0x12011, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_TsmByte1
	{ .offset = 0x12012, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_TsmByte2
	{ .offset = 0x12013, .val = 0x180 }, // DWC_DDRPHYA_DBYTE2_TsmByte3
	{ .offset = 0x12018, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_TsmByte5
	{ .offset = 0x12002, .val = 0x6209 }, // DWC_DDRPHYA_DBYTE2_TrainingParam
	{ .offset = 0x120b2, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm0_i0
	{ .offset = 0x121b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i1
	{ .offset = 0x122b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i2
	{ .offset = 0x123b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i3
	{ .offset = 0x124b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i4
	{ .offset = 0x125b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i5
	{ .offset = 0x126b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i6
	{ .offset = 0x127b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i7
	{ .offset = 0x128b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE2_Tsm2_i8
	{ .offset = 0x13011, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_TsmByte1
	{ .offset = 0x13012, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_TsmByte2
	{ .offset = 0x13013, .val = 0x180 }, // DWC_DDRPHYA_DBYTE3_TsmByte3
	{ .offset = 0x13018, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_TsmByte5
	{ .offset = 0x13002, .val = 0x6209 }, // DWC_DDRPHYA_DBYTE3_TrainingParam
	{ .offset = 0x130b2, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm0_i0
	{ .offset = 0x131b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i1
	{ .offset = 0x132b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i2
	{ .offset = 0x133b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i3
	{ .offset = 0x134b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i4
	{ .offset = 0x135b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i5
	{ .offset = 0x136b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i6
	{ .offset = 0x137b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i7
	{ .offset = 0x138b4, .val = 0x1 }, // DWC_DDRPHYA_DBYTE3_Tsm2_i8
	{ .offset = 0x20089, .val = 0x1 }, // DWC_DDRPHYA_MASTER0_CalZap
	{ .offset = 0x20088, .val = 0x19 }, // DWC_DDRPHYA_MASTER0_CalRate
	{ .offset = 0xc0080, .val = 0x2 }, // DWC_DDRPHYA_DRTUB0_UcclkHclkEnables
	{ .offset = 0xd0000, .val = 0x1 }, // DWC_DDRPHYA_APBONLY0_MicroContMuxSel
};

static void lpddr4_phy_init_cfg(void)
{
	int index, offset, val, size;

	size = sizeof(phy_init_cfg) / 8;

	for (index = 0; index < size; index++) {
		val = phy_init_cfg[index].val;
		offset = phy_init_cfg[index].offset;
		dwc_ddrphy_apb_wr(offset, val);
	}
}

static void lpddr4_phy_restore_trained_csr(void)
{
	int index, offset, val, size;

	dwc_ddrphy_apb_wr(0xd0000,0x0);
	size = sizeof(phy_trained_csr) / 8;

	for(index = 0; index < size; index++) {
		val = phy_trained_csr[index].val;
		offset = phy_trained_csr[index].offset;

		dwc_ddrphy_apb_wr(offset, val);
	}
	dwc_ddrphy_apb_wr(0xd0000,0x1);
}

static void lpddr4_phy_load_pie(void)
{
	int index, offset, val, size;

	size = sizeof(phy_pie) / 8;

	for(index = 0; index < size; index++) {
		val = phy_pie[index].val;
		offset = phy_pie[index].offset;

		dwc_ddrphy_apb_wr(offset, val);
	}
}

void lpddr4_phy_save_trained_csr(void)
{
	int index, offset, size;

	size = sizeof(phy_trained_csr) / 8;

	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	dwc_ddrphy_apb_wr(0xc0080, 0x3);


	for (index = 0; index < size; index++) {
		offset = phy_trained_csr[index].offset;
		phy_trained_csr[index].val = dwc_ddrphy_apb_rd(offset);
	}

	dwc_ddrphy_apb_wr(0xc0080, 0x0);
	dwc_ddrphy_apb_wr(0xd0000, 0x1);
}

void lpddr4_phy_cfg(void)
{
	/* load the init config */
	lpddr4_phy_init_cfg();

	/* restore the trained csr */
	lpddr4_phy_restore_trained_csr();

	/* load the phy PIE image */
	lpddr4_phy_load_pie();
}
