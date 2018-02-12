/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright 2017-2018 NXP.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <stdbool.h>
#include <ddrc.h>
#include <mmio.h>
#include <platform_def.h>
#include <spinlock.h>
#include <soc.h>

#define WR_POST_EXT_3200

static inline void umctl2_addrmap(void)
{
	/* Address mapping */
	/* need be refined for DDR vender */
	/* Address map is from MSB 29: r15, r14, cs, r13-r0, b2-b0, c9-c0 */
	mmio_write_32(DDRC_ADDRMAP0(0), 0x00000015);
	mmio_write_32(DDRC_ADDRMAP3(0), 0x00000000);
	mmio_write_32(DDRC_ADDRMAP4(0), 0x00001F1F);
	/* bank interleave */
	mmio_write_32(DDRC_ADDRMAP1(0), 0x00080808);
	mmio_write_32(DDRC_ADDRMAP5(0), 0x07070707);
	mmio_write_32(DDRC_ADDRMAP6(0), 0x08080707);
}

static inline void umctl2_perf(void)
{
	mmio_write_32(DDRC_ODTCFG(0), 0x0b060908);
	mmio_write_32(DDRC_ODTMAP(0), 0x00000000);
	mmio_write_32(DDRC_SCHED(0), 0x29511505);
	mmio_write_32(DDRC_SCHED1(0), 0x0000002c);
	mmio_write_32(DDRC_PERFHPR1(0), 0x5900575b);
	mmio_write_32(DDRC_PERFLPR1(0), 0x00000009);
	mmio_write_32(DDRC_PERFWR1(0), 0x02005574);
	mmio_write_32(DDRC_DBG0(0), 0x00000016);
	mmio_write_32(DDRC_DBG1(0), 0x00000000);
	mmio_write_32(DDRC_DBGCMD(0), 0x00000000);
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);
	mmio_write_32(DDRC_POISONCFG(0), 0x00000011);
	mmio_write_32(DDRC_PCCFG(0), 0x00000111);
	mmio_write_32(DDRC_PCFGR_0(0), 0x000010f3);
	mmio_write_32(DDRC_PCFGW_0(0), 0x000072ff);
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001);
	mmio_write_32(DDRC_PCFGQOS0_0(0), 0x01110d00);
	mmio_write_32(DDRC_PCFGQOS1_0(0), 0x00620790);
	mmio_write_32(DDRC_PCFGWQOS0_0(0), 0x00100001);
	mmio_write_32(DDRC_PCFGWQOS1_0(0), 0x0000041f);
	mmio_write_32(DDRC_FREQ1_DERATEEN(0), 0x00000202);
	mmio_write_32(DDRC_FREQ1_DERATEINT(0), 0xec78f4b5);
	mmio_write_32(DDRC_FREQ1_RFSHCTL0(0), 0x00618040);
	mmio_write_32(DDRC_FREQ1_RFSHTMG(0), 0x00610090);
}

static inline void umctl2_freq1(void)
{
	mmio_write_32(DDRC_FREQ1_DERATEEN(0), 0x0000000);
	mmio_write_32(DDRC_FREQ1_DERATEINT(0), 0x0800000);
	mmio_write_32(DDRC_FREQ1_RFSHCTL0(0), 0x0210000);
	mmio_write_32(DDRC_FREQ1_RFSHTMG(0), 0x014001E);
	mmio_write_32(DDRC_FREQ1_INIT3(0), 0x0140009);
	mmio_write_32(DDRC_FREQ1_INIT4(0), 0x00310008);
	mmio_write_32(DDRC_FREQ1_INIT6(0), 0x0066004a);
	mmio_write_32(DDRC_FREQ1_INIT7(0), 0x0006004a);
	mmio_write_32(DDRC_FREQ1_DRAMTMG0(0), 0xB070A07);
	mmio_write_32(DDRC_FREQ1_DRAMTMG1(0), 0x003040A);
	mmio_write_32(DDRC_FREQ1_DRAMTMG2(0), 0x305080C);
	mmio_write_32(DDRC_FREQ1_DRAMTMG3(0), 0x0505000);
	mmio_write_32(DDRC_FREQ1_DRAMTMG4(0), 0x3040203);
	mmio_write_32(DDRC_FREQ1_DRAMTMG5(0), 0x2030303);
	mmio_write_32(DDRC_FREQ1_DRAMTMG6(0), 0x2020004);
	mmio_write_32(DDRC_FREQ1_DRAMTMG7(0), 0x0000302);
	mmio_write_32(DDRC_FREQ1_DRAMTMG12(0), 0x0020310);
	mmio_write_32(DDRC_FREQ1_DRAMTMG13(0), 0xA100002);
	mmio_write_32(DDRC_FREQ1_DRAMTMG14(0), 0x0000020);
	mmio_write_32(DDRC_FREQ1_DRAMTMG17(0), 0x0220011);
	mmio_write_32(DDRC_FREQ1_ZQCTL0(0), 0x0A70005);
	mmio_write_32(DDRC_FREQ1_DFITMG0(0), 0x3858202);
	mmio_write_32(DDRC_FREQ1_DFITMG1(0), 0x0000404);
	mmio_write_32(DDRC_FREQ1_DFITMG2(0), 0x0000502);
}

void lpddr4_cfg_umctl2(void)
{
	/* Start to config, default 3200mbps */
	mmio_write_32(DDRC_DBG1(0), 0x00000001);
	mmio_write_32(DDRC_PWRCTL(0), 0x00000001);
	mmio_write_32(DDRC_MSTR(0), 0xa3080020);
	mmio_write_32(DDRC_MSTR2(0), 0x00000000);
	mmio_write_32(DDRC_RFSHTMG(0), 0x006100E0);
	mmio_write_32(DDRC_INIT0(0), 0xC003061B);
	mmio_write_32(DDRC_INIT1(0), 0x009D0000);
	mmio_write_32(DDRC_INIT3(0), 0x00D4002D);
#ifdef WR_POST_EXT_3200  /* recommened to define */
	mmio_write_32(DDRC_INIT4(0), 0x00330008);
#else
	mmio_write_32(DDRC_INIT4(0), 0x00310008);
#endif
	mmio_write_32(DDRC_INIT6(0), 0x0066004a);
	mmio_write_32(DDRC_INIT7(0), 0x0006004a);

	mmio_write_32(DDRC_DRAMTMG0(0), 0x1A201B22);
	mmio_write_32(DDRC_DRAMTMG1(0), 0x00060633);
	mmio_write_32(DDRC_DRAMTMG3(0), 0x00C0C000);
	mmio_write_32(DDRC_DRAMTMG4(0), 0x0F04080F);
	mmio_write_32(DDRC_DRAMTMG5(0), 0x02040C0C);
	mmio_write_32(DDRC_DRAMTMG6(0), 0x01010007);
	mmio_write_32(DDRC_DRAMTMG7(0), 0x00000401);
	mmio_write_32(DDRC_DRAMTMG12(0), 0x00020600);
	mmio_write_32(DDRC_DRAMTMG13(0), 0x0C100002);
	mmio_write_32(DDRC_DRAMTMG14(0), 0x000000E6);
	mmio_write_32(DDRC_DRAMTMG17(0), 0x00A00050);

	mmio_write_32(DDRC_ZQCTL0(0), 0x03200018);
	mmio_write_32(DDRC_ZQCTL1(0), 0x028061A8);
	mmio_write_32(DDRC_ZQCTL2(0), 0x00000000);

	mmio_write_32(DDRC_DFITMG0(0), 0x0497820A);
	mmio_write_32(DDRC_DFITMG1(0), 0x00080303);
	mmio_write_32(DDRC_DFIUPD0(0), 0xE0400018);
	mmio_write_32(DDRC_DFIUPD1(0), 0x00DF00E4);
	mmio_write_32(DDRC_DFIUPD2(0), 0x80000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000011);
	mmio_write_32(DDRC_DFITMG2(0), 0x0000170A);

	mmio_write_32(DDRC_DBICTL(0), 0x00000001);
	mmio_write_32(DDRC_DFIPHYMSTR(0), 0x00000001);

	mmio_write_32(DDRC_RANKCTL(0), 0x00000c99);
	mmio_write_32(DDRC_DRAMTMG2(0), 0x070E171a);

	/* address mapping */
	umctl2_addrmap();

	/* performance setting */
	umctl2_perf();

	/* freq set point 1 setting */
	umctl2_freq1();
}
