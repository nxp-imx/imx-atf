/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPDDR4_DEFINE_H__
#define __LPDDR4_DEFINE_H__

//----------------------------------------------------------------
//     DRAM MR setting
//----------------------------------------------------------------
//#define LPDDR4_FSP0_MR1 0xD4
//#define LPDDR4_FSP0_MR2 0x2D
//#define LPDDR4_FSP0_MR3 0x31
#define DDR_ONE_RANK


#ifdef LPDDR4_DBI_ON
#define LPDDR4_MR3			0xf1
#define LPDDR4_PHY_DMIPinPresent	0x1 
#else
#define LPDDR4_MR3			0x31
#define LPDDR4_PHY_DMIPinPresent	0x0 
#endif

#ifdef DDR_ONE_RANK
#define LPDDR4_CS			0x1
#else
#define LPDDR4_CS			0x3
#endif

#define LPDDR4_HDT_CTL_2D		0xC8
#define LPDDR4_HDT_CTL_3200_1D		0xC8
#define LPDDR4_HDT_CTL_400_1D		0xC8
#define LPDDR4_HDT_CTL_100_1D		0xC8

#define LPDDR4_HDT_CTL_2D		0xC8
#define LPDDR4_HDT_CTL_3200_1D		0xC8
#define LPDDR4_HDT_CTL_400_1D		0xC8
#define LPDDR4_HDT_CTL_100_1D		0xC8

#ifdef RUN_ON_SILICON
/* 400/100 training seq */
#define LPDDR4_TRAIN_SEQ_P2		0x121f
#define LPDDR4_TRAIN_SEQ_P1		0x121f
#define LPDDR4_TRAIN_SEQ_P0		0x121f
#else
#define LPDDR4_TRAIN_SEQ_P2		 0x7
#define LPDDR4_TRAIN_SEQ_P1 		0x7
#define LPDDR4_TRAIN_SEQ_P0 		0x7
#endif

/* 2D share & weight */
#define LPDDR4_2D_WEIGHT 		0x1f7f
#define LPDDR4_2D_SHARE 		1
#define LPDDR4_CATRAIN_3200_1d		0
#define LPDDR4_CATRAIN_400		0
#define LPDDR4_CATRAIN_100		0
#define LPDDR4_CATRAIN_3200_2d		0

/* MRS parameter */

/* for LPDDR4 Rtt */
#define LPDDR4_RTT40		6
#define LPDDR4_RTT48		5
#define LPDDR4_RTT60		4
#define LPDDR4_RTT80		3
#define LPDDR4_RTT120		2
#define LPDDR4_RTT240		1
#define LPDDR4_RTT_DIS		0

/* for LPDDR4 Ron */
#define LPDDR4_RON34		7
#define LPDDR4_RON40		6
#define LPDDR4_RON48		5
#define LPDDR4_RON60		4
#define LPDDR4_RON80		3

#define LPDDR4_PHY_ADDR_RON60		0x1
#define LPDDR4_PHY_ADDR_RON40		0x3
#define LPDDR4_PHY_ADDR_RON30		0x7
#define LPDDR4_PHY_ADDR_RON24		0xf
#define LPDDR4_PHY_ADDR_RON20		0x1f

/* for read channel */
#define LPDDR4_RON			LPDDR4_RON40
#define LPDDR4_PHY_RTT			30
#define LPDDR4_PHY_VREF_VALUE 		17

/* for write channel */
#define LPDDR4_PHY_RON			30
#define LPDDR4_PHY_ADDR_RON		LPDDR4_PHY_ADDR_RON40
#define LPDDR4_RTT_DQ			LPDDR4_RTT40
#define LPDDR4_RTT_CA			LPDDR4_RTT40
#define LPDDR4_RTT_CA_BANK0		LPDDR4_RTT40
#define LPDDR4_RTT_CA_BANK1		LPDDR4_RTT40
#define LPDDR4_VREF_VALUE_CA		((1<<6)|(0xd))
#define LPDDR4_VREF_VALUE_DQ_RANK0	((1<<6)|(0xd))
#define LPDDR4_VREF_VALUE_DQ_RANK1	((1<<6)|(0xd))
#define LPDDR4_MR22_RANK0		((0<<5)|(0<<4)|(0<<3)|(LPDDR4_RTT40))
#define LPDDR4_MR22_RANK1		((1<<5)|(0<<4)|(1<<3)|(LPDDR4_RTT40))

#define LPDDR4_MR3_PU_CAL		1

#endif /*__LPDDR4_DEFINE_H__ */
