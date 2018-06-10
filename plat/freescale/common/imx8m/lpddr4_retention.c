/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <debug.h>
#include <ddrc.h>
#include <dram.h>
#include <mmio.h>
#include <platform_def.h>
#include <soc.h>

#define SRC_IPS_BASE_ADDR       IMX_SRC_BASE
#define SRC_DDRC_RCR_ADDR	(SRC_IPS_BASE_ADDR + 0x1000)

#define GPC_PU_PWRHSK		(IMX_GPC_BASE + 0x01FC)
#define CCM_SRC_CTRL_OFFSET     (IMX_CCM_BASE + 0x800)
#define CCM_CCGR_OFFSET         (IMX_CCM_BASE + 0x4000)
#define CCM_SRC_CTRL(n)		(CCM_SRC_CTRL_OFFSET + 0x10 * n)
#define CCM_CCGR(n)		(CCM_CCGR_OFFSET + 0x10 * n)

struct dram_cfg_param lpddr4_ddrc_cfg[] = {
	{ .reg =DDRC_DBG1(0),	 .cfg = 0x00000001},
	{ .reg =DDRC_PWRCTL(0),  .cfg = 0x00000001},
	{ .reg =DDRC_MSTR(0),	 .cfg = 0xa1080020},
	{ .reg =DDRC_RFSHTMG(0), .cfg = 0x005b00d2},
	{ .reg =DDRC_INIT0(0),   .cfg = 0xC003061B},
	{ .reg =DDRC_INIT1(0),   .cfg = 0x009D0000},
	{ .reg =DDRC_INIT3(0),   .cfg = 0x00D4002D},
	{ .reg =DDRC_INIT4(0),   .cfg = 0x00330008},
	{ .reg =DDRC_INIT6(0),   .cfg = 0x0066004a},
	{ .reg =DDRC_INIT7(0),   .cfg = 0x0006004a},

	{ .reg =DDRC_DRAMTMG0(0),  .cfg = 0x1A201B22},
	{ .reg =DDRC_DRAMTMG1(0),  .cfg = 0x00060633},
	{ .reg =DDRC_DRAMTMG3(0),  .cfg = 0x00C0C000},
	{ .reg =DDRC_DRAMTMG4(0),  .cfg = 0x0F04080F},
	{ .reg =DDRC_DRAMTMG5(0),  .cfg = 0x02040C0C},
	{ .reg =DDRC_DRAMTMG6(0),  .cfg = 0x01010007},
	{ .reg =DDRC_DRAMTMG7(0),  .cfg = 0x00000401},
	{ .reg =DDRC_DRAMTMG12(0), .cfg = 0x00020600},
	{ .reg =DDRC_DRAMTMG13(0), .cfg = 0x0C100002},
	{ .reg =DDRC_DRAMTMG14(0), .cfg = 0x000000E6},
	{ .reg =DDRC_DRAMTMG17(0), .cfg = 0x00A00050},

	{ .reg =DDRC_ZQCTL0(0), .cfg = 0x03200018},
	{ .reg =DDRC_ZQCTL1(0), .cfg = 0x028061A8},
	{ .reg =DDRC_ZQCTL2(0), .cfg = 0x00000000},

	{ .reg =DDRC_DFITMG0(0), .cfg = 0x0497820A},
	{ .reg =DDRC_DFITMG1(0), .cfg = 0x00080303},
	{ .reg =DDRC_DFIUPD0(0), .cfg = 0xE0400018},

	{ .reg =DDRC_DFIUPD1(0), .cfg = 0x00DF00E4},
	{ .reg =DDRC_DFIUPD2(0), .cfg = 0x80000000},
	{ .reg =DDRC_DFIMISC(0), .cfg = 0x00000011},
	{ .reg =DDRC_DFITMG2(0), .cfg = 0x0000170A},

	{ .reg =DDRC_DBICTL(0),     .cfg = 0x00000001},
	{ .reg =DDRC_DFIPHYMSTR(0), .cfg = 0x00000000},
	{ .reg =DDRC_RANKCTL(0),    .cfg = 0x00000c99},
	{ .reg =DDRC_DRAMTMG2(0),   .cfg = 0x070E171a},

	/* ADDRMAP */
	{ .reg =DDRC_ADDRMAP0(0), .cfg = 0x0000001f},
	{ .reg =DDRC_ADDRMAP1(0), .cfg = 0x00080808},
	{ .reg =DDRC_ADDRMAP2(0), .cfg = 0x00000000},
	{ .reg =DDRC_ADDRMAP3(0), .cfg = 0x00000000},
	{ .reg =DDRC_ADDRMAP4(0), .cfg = 0x00001f1f},
	{ .reg =DDRC_ADDRMAP5(0), .cfg = 0x07070707},
	{ .reg =DDRC_ADDRMAP6(0), .cfg = 0x07070707},
	{ .reg =DDRC_ADDRMAP7(0), .cfg = 0x00000f0f},

	/* perf */
	{ .reg =DDRC_SCHED(0),    .cfg = 0x29001701},
	{ .reg =DDRC_SCHED1(0),   .cfg = 0x0000002c},
	{ .reg =DDRC_PERFHPR1(0), .cfg = 0x04000030},
	{ .reg =DDRC_PERFLPR1(0), .cfg = 0x900093e7},
	{ .reg =DDRC_PCCFG(0),    .cfg = 0x00000111},
	{ .reg =DDRC_PCFGW_0(0),  .cfg = 0x000072ff},
	{ .reg =DDRC_PCFGQOS0_0(0), .cfg = 0x02100e07},
	{ .reg =DDRC_PCFGQOS1_0(0), .cfg = 0x00620096},
	{ .reg =DDRC_PCFGWQOS0_0(0), .cfg = 0x01100e07},
	{ .reg =DDRC_PCFGWQOS1_0(0), .cfg = 0x0000012c},

	/* shadow registers for P1 & P2 */

	/* init fsp */
	{ .reg =DDRC_MSTR2(0), .cfg = 0x0},
};

struct dram_cfg_param lpddr4_ddrphy_cfg[] = {

	/* phy init config */
	{ .reg = DDRPHY_REG(0x1005f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1015f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1105f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1115f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1205f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1215f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1305f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x1315f), .cfg = 0x15f}, 
	{ .reg = DDRPHY_REG(0x55),    .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x1055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x2055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x3055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x4055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x5055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x6055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x7055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x8055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x9055),  .cfg = 0x16f}, 
	{ .reg = DDRPHY_REG(0x200c5), .cfg = 0x19}, 
	{ .reg = DDRPHY_REG(0x2002e), .cfg = 0x2}, 
	{ .reg = DDRPHY_REG(0x90204), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x20024), .cfg = 0xab}, 
	{ .reg = DDRPHY_REG(0x2003a), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x20056), .cfg = 0x3}, 
	{ .reg = DDRPHY_REG(0x1004d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1014d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1104d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1114d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1204d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1214d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1304d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x1314d), .cfg = 0xe00}, 
	{ .reg = DDRPHY_REG(0x10049), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x10149), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x11049), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x11149), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x12049), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x12149), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x13049), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x13149), .cfg = 0xfbe}, 
	{ .reg = DDRPHY_REG(0x43),    .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x1043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x2043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x3043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x4043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x5043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x6043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x7043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x8043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x9043),  .cfg = 0x63}, 
	{ .reg = DDRPHY_REG(0x20018), .cfg = 0x3}, 
	{ .reg = DDRPHY_REG(0x20075), .cfg = 0x4}, 
	{ .reg = DDRPHY_REG(0x20050), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x20008), .cfg = 0x2ee}, 
	{ .reg = DDRPHY_REG(0x20088), .cfg = 0x9}, 
	{ .reg = DDRPHY_REG(0x200b2), .cfg = 0x1d4}, 
	{ .reg = DDRPHY_REG(0x10043), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x10143), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x11043), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x11143), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x12043), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x12143), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x13043), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x13143), .cfg = 0x5a1}, 
	{ .reg = DDRPHY_REG(0x200fa), .cfg = 0x1}, 
	{ .reg = DDRPHY_REG(0x20019), .cfg = 0x1}, 
	{ .reg = DDRPHY_REG(0x200f0), .cfg = 0x600}, 
	{ .reg = DDRPHY_REG(0x200f1), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x200f2), .cfg = 0x4444}, 
	{ .reg = DDRPHY_REG(0x200f3), .cfg = 0x8888}, 
	{ .reg = DDRPHY_REG(0x200f4), .cfg = 0x5655}, 
	{ .reg = DDRPHY_REG(0x200f5), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x200f6), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x200f7), .cfg = 0xf000}, 
	{ .reg = DDRPHY_REG(0x20025), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x2002d), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0x200c7), .cfg = 0x21}, 
	{ .reg = DDRPHY_REG(0x200ca), .cfg = 0x24}, 
	{ .reg = DDRPHY_REG(0x20060), .cfg = 0x2}, 
	{ .reg = DDRPHY_REG(0xd0000), .cfg = 0x0}, 
	{ .reg = DDRPHY_REG(0xd0000), .cfg = 0x1}, 

	/* trained CSR */
	{ .reg = DDRPHY_REG(0x200b2),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1200b2), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2200b2), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x200cb),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10043),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10143),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11043),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11143),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12043),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12143),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13043),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213043), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13143),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213143), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x80),     .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x200080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x101080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x201080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x102080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x202080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x3080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x103080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x203080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x4080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x104080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x204080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x5080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x105080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x205080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x6080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x106080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x206080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x7080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x107080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x207080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x8080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x108080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x208080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x9080),   .cfg = 0 },
	{ .reg = DDRPHY_REG(0x109080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x209080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10080),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10180),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11080),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11180),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12080),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12180),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13080),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213080), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13180),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213180), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10081),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10181),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11081),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11181),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12081),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12181),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13081),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213081), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13181),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213181), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x101d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1101d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2101d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1111d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2111d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x121d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1121d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2121d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x131d0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1131d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2131d0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x101d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1101d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2101d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1111d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2111d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x121d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1121d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2121d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x131d1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1131d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2131d1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10068),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10168),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10268),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10368),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10468),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10568),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10668),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10768),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10868),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11068),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11168),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11268),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11368),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11468),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11568),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11668),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11768),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11868),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12068),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12168),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12268),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12368),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12468),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12568),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12668),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12768),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12868),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13068),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13168),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13268),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13368),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13468),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13568),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13668),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13768),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13868),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10069),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10169),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10269),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10369),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10469),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10569),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10669),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10769),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10869),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11069),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11169),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11269),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11369),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11469),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11569),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11669),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11769),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11869),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12069),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12169),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12269),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12369),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12469),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12569),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12669),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12769),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12869),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13069),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13169),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13269),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13369),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13469),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13569),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13669),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13769),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13869),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1008c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11008c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21008c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1018c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11018c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21018c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1108c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11108c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21108c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1118c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11118c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21118c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1208c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11208c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21208c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1218c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11218c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21218c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1308c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11308c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21308c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1318c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11318c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21318c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1008d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11008d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21008d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1018d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11018d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21018d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1108d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11108d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21108d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1118d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11118d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21118d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1208d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11208d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21208d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1218d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11218d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21218d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1308d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11308d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21308d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1318d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11318d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x21318d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x101c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1101c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2101c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x102c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1102c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2102c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x103c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1103c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2103c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x104c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1104c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2104c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x105c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1105c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2105c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x106c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1106c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2106c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x107c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1107c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2107c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x108c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1108c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2108c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1111c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2111c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1112c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2112c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1113c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2113c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x114c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1114c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2114c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x115c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1115c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2115c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x116c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1116c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2116c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x117c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1117c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2117c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x118c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1118c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2118c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x121c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1121c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2121c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x122c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1122c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2122c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x123c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1123c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2123c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x124c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1124c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2124c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x125c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1125c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2125c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x126c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1126c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2126c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x127c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1127c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2127c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x128c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1128c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2128c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x131c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1131c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2131c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x132c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1132c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2132c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x133c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1133c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2133c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x134c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1134c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2134c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x135c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1135c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2135c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x136c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1136c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2136c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x137c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1137c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2137c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x138c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1138c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2138c0), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x101c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1101c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2101c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x102c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1102c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2102c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x103c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1103c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2103c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x104c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1104c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2104c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x105c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1105c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2105c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x106c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1106c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2106c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x107c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1107c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2107c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x108c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1108c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2108c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1111c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2111c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1112c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2112c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1113c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2113c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x114c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1114c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2114c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x115c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1115c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2115c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x116c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1116c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2116c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x117c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1117c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2117c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x118c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1118c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2118c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x121c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1121c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2121c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x122c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1122c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2122c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x123c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1123c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2123c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x124c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1124c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2124c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x125c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1125c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2125c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x126c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1126c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2126c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x127c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1127c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2127c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x128c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1128c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2128c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x131c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1131c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2131c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x132c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1132c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2132c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x133c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1133c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2133c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x134c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1134c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2134c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x135c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1135c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2135c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x136c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1136c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2136c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x137c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1137c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2137c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x138c1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1138c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2138c1), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10020),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x210020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11020),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x111020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x211020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12020),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x112020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x212020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13020),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x113020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x213020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20072),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20073),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20074),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100aa),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110aa),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120aa),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130aa),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20010),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120010), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x220010), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20011),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120011), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x220011), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100ae),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100af),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1100af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2100af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110ae),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110af),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1110af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2110af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120ae),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120af),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1120af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2120af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130ae),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130ae), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130af),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x1130af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2130af), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20020),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x220020), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a2),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a3),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a4),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a5),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a6),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x100a7),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a2),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a3),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a4),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a5),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a6),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x110a7),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a2),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a3),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a4),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a5),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a6),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x120a7),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a1),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a2),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a3),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a4),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a5),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a6),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x130a7),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2007c),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12007c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x22007c), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x2007d),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12007d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x22007d), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x400fd),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x400c0),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90201),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190201), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290201), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90202),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190202), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290202), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90203),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190203), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290203), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90204),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190204), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290204), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90205),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190205), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290205), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90206),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190206), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290206), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90207),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190207), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290207), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x90208),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x190208), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x290208), .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10062),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10162),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10262),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10362),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10462),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10562),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10662),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10762),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10862),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11062),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11162),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11262),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11362),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11462),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11562),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11662),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11762),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11862),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12062),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12162),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12262),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12362),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12462),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12562),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12662),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12762),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12862),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13062),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13162),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13262),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13362),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13462),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13562),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13662),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13762),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13862),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x20077),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10001),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11001),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12001),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13001),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10040),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10140),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10240),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10340),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10440),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10540),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10640),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10740),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10840),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10030),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10130),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10230),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10330),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10430),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10530),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10630),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10730),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x10830),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11040),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11140),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11240),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11340),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11440),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11540),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11640),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11740),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11840),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11030),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11130),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11230),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11330),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11430),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11530),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11630),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11730),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x11830),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12040),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12140),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12240),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12340),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12440),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12540),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12640),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12740),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12840),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12030),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12130),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12230),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12330),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12430),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12530),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12630),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12730),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x12830),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13040),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13140),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13240),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13340),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13440),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13540),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13640),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13740),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13840),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13030),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13130),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13230),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13330),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13430),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13530),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13630),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13730),  .cfg = 0 },
	{ .reg = DDRPHY_REG(0x13830),  .cfg = 0 },
};

struct dram_info imx8m_lpddr4_dram_info = {
	.ddrc_cfg = lpddr4_ddrc_cfg,
	.ddrc_cfg_num = ARRAY_SIZE(lpddr4_ddrc_cfg),
	.ddrphy_cfg = lpddr4_ddrphy_cfg,
	.ddrphy_cfg_num = ARRAY_SIZE(lpddr4_ddrphy_cfg),
};



void lpddr4_enter_retention(void)
{
	unsigned int tmp, tmp_t, i;
	INFO("enter lpddr4 retention\n");

	/* wait DBGCAM to be empty */
	do {
		tmp = mmio_read_32(DDRC_DBGCAM(0));
	} while(tmp != 0x36000000);

	/* Blocks AXI ports from taking anymore transactions */
	mmio_write_32(DDRC_PCTRL_0(0), 0x00000000);

	/* Waits unit all AXI ports are idle */
	do {
		tmp = mmio_read_32(DDRC_PSTAT(0));
	} while(tmp & 0x10001);

	/* enters self refresh */
	//reg32setbit(DDRC_PWRCTL(0), 5);
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);

	tmp=0;
	/* self-refresh state */
	while (tmp!=0x223) {
		tmp  = 0x33f & (mmio_read_32((DDRC_STAT(0))));
		INFO("C: waiting for STAT selfref_type= Self Refresh\n");
	} 

	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	/* set SWCTL.sw_done to disable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);
	mmio_write_32(DDRC_DFIMISC(0), 0x00001f20);

	/* wait DFISTAT.dfi_init_complete to 0 */
	tmp_t = 0;
	while (tmp_t==1) {
		tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
	}

	mmio_write_32(DDRC_DFIMISC(0), 0x00001f00);

	/* wait DFISTAT.dfi_init_complete to 1 */
	tmp_t = 0;
	while (tmp_t==0) {
		tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
	}

	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000,0x0);
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4*0x00090028);
	tmp = tmp & 0x1;
	if (tmp)
		INFO("C: PhyInLP3 = 1\n");
	else
		INFO("vt_fail5\n");
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	for (i=0; i<20; i++){
 	}

#ifdef M850D
	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 1));
	do {
		tmp = mmio_read_32(GPC_PU_PWRHSK);
	//        printf("C: wait pwrdnackn_async clr\n");
	} while(tmp&(0x1<<18));//wait untill pwrdnackn_async=0
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 1));
#else
	/* pwrdnreqn_async adbm/adbs of ddr */
	mmio_clrbits_32(GPC_PU_PWRHSK, (1 << 2));
	do {
		tmp = mmio_read_32(GPC_PU_PWRHSK);
		INFO("C: wait pwrdnackn_async clr\n");
	} while(tmp&(0x1<<20));//wait untill pwrdnackn_async=0
	mmio_setbits_32(GPC_PU_PWRHSK, (1 << 2));
#endif
	/* remove PowerOk */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000008); 
	INFO("vt_event1\n");

	for (i=0; i<20; i++) {
	}

	mmio_write_32(CCM_CCGR(5),0);
	mmio_write_32(CCM_SRC_CTRL(15),2);// when test on chip, this should be 0x2
	INFO("C: gated all DRAM clocks \n");

	for (i=0; i<20; i++) {
	}
	/* enable the phy iso */
	mmio_setbits_32(0x303a0d40, 1);
	mmio_setbits_32(0x303a0104, (1 << 5));
}

void lpddr4_exit_retention(void)
{
	unsigned int tmp, tmp_t, i;

	INFO("exit lpddr4 retention\n");

	for (i=0; i<50; i++) {
	}

	for (i=0; i<50; i++) {
	}

	/*assert all reset */
#ifdef M850D
	mmio_write_32(SRC_DDRC_RCR_ADDR+0x4, 0x8F000003); // assert [0]src_system_rst_b!
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n,
	mmio_write_32(SRC_DDRC_RCR_ADDR+0x4, 0x8F000000); // deassert  [4]src_system_rst_b!
#else
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00001F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F00000F); // assert [0]ddr1_preset_n, [1]ddr1_core_reset_n, [2]ddr1_phy_reset, [3]ddr1_phy_pwrokin_n
#endif

	mmio_write_32(CCM_CCGR(5),2);
	mmio_write_32(CCM_SRC_CTRL(15),2);
	INFO("C: enable all DRAM clocks \n");

	/* change the clock source of dram_apb_clk_root */
	mmio_write_32(0x3038a088, (0x7<<24)|(0x7<<16));
	mmio_write_32(0x3038a084, (0x4<<24)|(0x3<<16));

	/* disable iso */
	mmio_write_32(0x303A00EC,0x0000ffff); //PGC_CPU_MAPPING
	mmio_setbits_32(0x303a00f8, (1 << 5));

	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000006); // release [0]ddr1_preset_n,  [3]ddr1_phy_pwrokin_n

	/* ddrc re-init */
	dram_umctl2_init();

	/*
	 * Skips the DRAM init routine and starts up in selfrefresh mode
	 * Program INIT0.skip_dram_init = 2'b11
	 */
	mmio_write_32(DDRC_INIT0(0), 0xc0000000 | (mmio_read_32(DDRC_INIT0(0))));
	mmio_write_32(DDRC_MSTR2(0), 0x0);
	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa);
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000004); /* release all reset */
	mmio_write_32(SRC_DDRC_RCR_ADDR, 0x8F000000); /* release all reset */
	mmio_write_32(DDRC_DBG1(0), 0x00000000);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DDR_SS_GPR0, 0x01); /*LPDDR4 mode */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);

	/* dram phy re-init */
	dram_phy_init();

	INFO("ddrphy config to 3000mts\n");

	/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	dwc_ddrphy_apb_wr(0xd0000,0x0); 
	do {
		tmp_t = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0x00020097); 
		INFO("C: Waiting for CalBusy = 0x%x\n",tmp_t);
	} while (tmp_t != 0);
	dwc_ddrphy_apb_wr(0xd0000,0x1);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x00000000);
	mmio_write_32(DDRC_DFIMISC(0), 0x00000020); 

	/* wait DFISTAT.dfi_init_complete to 1 */
	tmp_t = 0;
	while (tmp_t==0) {
		tmp  = mmio_read_32(DDRC_DFISTAT(0));
		tmp_t = tmp & 0x01;
		INFO("wait DFISTAT.dfi_init_complete to 1\n");
	}

	/* clear DFIMISC.dfi_init_start */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000000);
	/* set DFIMISC.dfi_init_complete_en */
	mmio_write_32(DDRC_DFIMISC(0), 0x00000001);
	/* set SWCTL.sw_done to enable quasi-dynamic register programming outside reset. */
	mmio_write_32(DDRC_SWCTL(0), 0x00000001);

	/* wait SWSTAT.sw_done_ack to 1 */
	tmp_t = 0;
	while(tmp_t!=0x1){
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x1;
		INFO("1wait SWSTAT.sw_done_ack to 1\n");
	}

	mmio_write_32(DDRC_PWRCTL(0), 0x000000aa); 
	mmio_write_32(DDRC_SWCTL(0), 0x00000000); 
	mmio_write_32(DDRC_DFIMISC(0), 0x00000001); 
	mmio_write_32(DDRC_SWCTL(0), 0x00000001); 

	/* wait SWSTAT.sw_done_ack to 1 */
	tmp_t = 0;
	while(tmp_t!=0x1){
		tmp  = mmio_read_32(DDRC_SWSTAT(0));
		tmp_t = tmp & 0x1;
		INFO("2wait SWSTAT.sw_done_ack to 1\n");
	}

	mmio_write_32(DDRC_PWRCTL(0), 0x00000088); 

	/* wait STAT to normal state */
	tmp_t = 0;
	while(tmp_t!=0x1) {
		tmp  = mmio_read_32(DDRC_STAT(0));
		tmp_t = tmp & 0x7;
		INFO("wait STAT to normal state\n");
	}
 
	mmio_write_32(DDRC_DERATEEN(0), 0x00000302);

	mmio_write_32(DDRC_PCTRL_0(0), 0x00000001); 
	 /* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x00000000);

	/* should check PhyInLP3 pub reg */
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0xd0000,0x0);
	tmp = mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0x00090028);
	tmp = tmp & 0x1;
	if(!tmp)
		INFO("C: PhyInLP3 = 0\n");
	else
		INFO("vt_fail6\n");
	mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0)+4*0xd0000,0x1);

}
