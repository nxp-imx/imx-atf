#ifndef __IMX_DDRC_H
#define __IMX_DDRC_H

/* DWC ddr umctl2 REGs offset*/
#define DDRC_MSTR	0x0
#define DDRC_STAT	0x4
#define DDRC_MSTR1	0x8
#define DDRC_MRCTRL0	0x10
#define DDRC_MRCTRL1	0x14
#define DDRC_MRSTAT	0x18
#define DDRC_MRCTRL2	0x1c
#define DDRC_DERATEEN	0x20
#define DDRC_DERATEINT	0x24
#define DDRC_MSTR2	0x28
#define DDRC_PWRCTL	0x30
#define DDRC_PWRTMG	0x34
#define DDRC_HWLPCTL	0x38
#define DDRC_HWFFCCTL	0x3c
#define DDRC_HWFFCSTAT	0x40
#define DDRC_RFSHCTL0	0x50
#define DDRC_RFSHCTL1	0x54
#define DDRC_RFSHCTL2	0x58
#define DDRC_RFSHCTL4	0x5c
#define DDRC_RFSHCTL3	0x60
#define DDRC_RFSHTMG	0x64
#define DDRC_RFSHTMG1	0x68

#define DDRC_INIT0	0xd0
#define DDRC_INIT1	0xd4
#define DDRC_INIT2	0xd8
#define DDRC_INIT3	0xdc
#define DDRC_INIT4	0xe0
#define DDRC_INIT5	0xe4
#define DDRC_INIT6	0xe8
#define DDRC_INIT7	0xec
#define DDRC_DIMMCTL	0xf0
#define DDRC_RANKCTL	0xf4
#define DDRC_CHCTL	0xfc

#define DDRC_CRCPARSTAT	0xcc
#define DDRC_DFIMISC	0x1b0
#define DDRC_DFISTAT	0x1bc
#define DDRC_ADDRMAP0	0x200
#define DDRC_ADDRMAP1	0x204
#define DDRC_ADDRMAP2	0x208
#define DDRC_ADDRMAP3	0x20c
#define DDRC_ADDRMAP4	0x210
#define DDRC_ADDRMAP5	0x214
#define DDRC_ADDRMAP6	0x218
#define DDRC_ADDRMAP7	0x21c
#define DDRC_ADDRMAP8	0x220
#define DDRC_ADDRMAP9	0x224
#define DDRC_ADDRMAP10	0x228
#define DDRC_ADDRMAP11	0x22c
#define DDRC_ODTCFG	0x240
#define DDRC_ODTMAP	0x244
#define DDRC_SCHED	0x250
#define DDRC_SCHED1	0x254
#define DDRC_SCHED2	0x258
#define DDRC_PERFHPR1	0x25c
#define DDRC_PERFLPR1	0x264
#define DDRC_PERFWR1	0x26c
#define DDRC_DBG0	0x300
#define DDRC_DBG1	0x304
#define DDRC_DBGCAM	0x308
#define DDRC_DBGCMD	0x30c
#define DDRC_DBGSTAT	0x310
#define DDRC_SWCTL	0x320
#define DDRC_SWSTAT	0x324
#define DDRC_POISONCFG	0x36c
#define DDRC_POISONSTAT	0x370

#define DDRC_PCCFG	0x400
#define DDRC_PCFGR_0	0x404
#define DDRC_PCFGW_0	0x408
#define DDRC_PCTRL_0	0x490
#define DDRC_PCFGQOS0_0	0x494
#define DDRC_PCFGQOS1_0	0x498
#define DDRC_PCFGWQOS0_0 0x49c
#define DDRC_PCFGWQOS1_0 0x4a0

/* shadow register */
#define DDRC_FREQ1_DERATEEN         0x2020
#define DDRC_FREQ1_DERATEINT        0x2024
#define DDRC_FREQ1_RFSHCTL0         0x2050
#define DDRC_FREQ1_RFSHTMG          0x2064
#define DDRC_FREQ1_INIT3            0x20dc
#define DDRC_FREQ1_INIT4            0x20e0
#define DDRC_FREQ1_INIT6            0x20e8
#define DDRC_FREQ1_INIT7            0x20ec
#define DDRC_FREQ1_DRAMTMG0         0x2100
#define DDRC_FREQ1_DRAMTMG1         0x2104
#define DDRC_FREQ1_DRAMTMG2         0x2108
#define DDRC_FREQ1_DRAMTMG3         0x210c
#define DDRC_FREQ1_DRAMTMG4         0x2110
#define DDRC_FREQ1_DRAMTMG5         0x2114
#define DDRC_FREQ1_DRAMTMG6         0x2118
#define DDRC_FREQ1_DRAMTMG7         0x211c
#define DDRC_FREQ1_DRAMTMG8         0x2120
#define DDRC_FREQ1_DRAMTMG9         0x2124
#define DDRC_FREQ1_DRAMTMG10        0x2128
#define DDRC_FREQ1_DRAMTMG11        0x212c
#define DDRC_FREQ1_DRAMTMG12        0x2130
#define DDRC_FREQ1_DRAMTMG13        0x2134
#define DDRC_FREQ1_DRAMTMG14        0x2138
#define DDRC_FREQ1_DRAMTMG15        0x213C
#define DDRC_FREQ1_DRAMTMG16        0x2140
#define DDRC_FREQ1_DRAMTMG17        0x2144
#define DDRC_FREQ1_ZQCTL0           0x2180
#define DDRC_FREQ1_DFITMG0          0x2190
#define DDRC_FREQ1_DFITMG1          0x2194
#define DDRC_FREQ1_DFITMG2          0x21b4
#define DDRC_FREQ1_DFITMG3          0x21b8
#define DDRC_FREQ1_ODTCFG           0x2240

#define DDRC_FREQ2_DERATEEN         0x3020
#define DDRC_FREQ2_DERATEINT        0x3024
#define DDRC_FREQ2_RFSHCTL0         0x3050
#define DDRC_FREQ2_RFSHTMG          0x3064
#define DDRC_FREQ2_INIT3            0x30dc
#define DDRC_FREQ2_INIT4            0x30e0
#define DDRC_FREQ2_INIT6            0x30e8
#define DDRC_FREQ2_INIT7            0x30ec
#define DDRC_FREQ2_DRAMTMG0         0x3100
#define DDRC_FREQ2_DRAMTMG1         0x3104
#define DDRC_FREQ2_DRAMTMG2         0x3108
#define DDRC_FREQ2_DRAMTMG3         0x310c
#define DDRC_FREQ2_DRAMTMG4         0x3110
#define DDRC_FREQ2_DRAMTMG5         0x3114
#define DDRC_FREQ2_DRAMTMG6         0x3118
#define DDRC_FREQ2_DRAMTMG7         0x311c
#define DDRC_FREQ2_DRAMTMG8         0x3120
#define DDRC_FREQ2_DRAMTMG9         0x3124
#define DDRC_FREQ2_DRAMTMG10        0x3128
#define DDRC_FREQ2_DRAMTMG11        0x312c
#define DDRC_FREQ2_DRAMTMG12        0x3130
#define DDRC_FREQ2_DRAMTMG13        0x3134
#define DDRC_FREQ2_DRAMTMG14        0x3138
#define DDRC_FREQ2_DRAMTMG15        0x313C
#define DDRC_FREQ2_DRAMTMG16        0x3140
#define DDRC_FREQ2_DRAMTMG17        0x3144
#define DDRC_FREQ2_ZQCTL0           0x3180
#define DDRC_FREQ2_DFITMG0          0x3190
#define DDRC_FREQ2_DFITMG1          0x3194
#define DDRC_FREQ2_DFITMG2          0x31b4
#define DDRC_FREQ2_DFITMG3          0x31b8
#define DDRC_FREQ2_ODTCFG           0x3240

#define IP2APB_DDRPHY_IPS_BASE_ADDR(X) (0x3c000000 + (X * 0x2000000))
#define dwc_ddrphy_apb_rd(addr) mmio_read_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * (addr))
#define dwc_ddrphy_apb_wr(addr, val) mmio_write_32(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * (addr), val)

#endif /*__IMX_DDRC_H */
