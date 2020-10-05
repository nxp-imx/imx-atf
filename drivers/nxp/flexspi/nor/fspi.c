// SPDX-License-Identifier: BSD-3-Clause
/*
 * NXP FlexSpi Controller Driver.
 * Copyright 2020 NXP
 *
 */
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <flash_info.h>
#include "fspi.h"
#include <fspi_api.h>
#include <xspi_error_codes.h>


#ifdef DEBUG_FLEXSPI
#define PR printf("In [%s][%d]\n", __func__, __LINE__)
#define PRA(a, b) printf("In [%s][%d] %s="a"\n", __func__, __LINE__, #b, b)
#else
#define PR
#define PRA(a, b)
#endif

/*
 * This errata is valid for all NXP SoC.
 */
#define ERRATA_FLASH_A050272 1

static uintptr_t fspi_base_reg_addr;
static uintptr_t fspi_flash_base_addr;

//void *memcpy(void *dst, const void *src, size_t len);
static void fspi_RDSR(uint32_t *, const void *, uint32_t);

static void fspi_writel(uint32_t xAddr, uint32_t xVal)
{
	fspi_out32((uint32_t *)(fspi_base_reg_addr + xAddr),
		 (uint32_t) xVal);
}

static uint32_t fspi_readl(uint32_t xAddr)
{
	return fspi_in32((uint32_t *)(fspi_base_reg_addr + xAddr));
}

static void fspi_MDIS(uint8_t xDisable)
{
	uint32_t  uiReg;

	uiReg = fspi_readl(FSPI_MCR0);
	if (xDisable)
		uiReg |= FSPI_MCR0_MDIS;
	else
		uiReg &= (uint32_t) (~FSPI_MCR0_MDIS);

	fspi_writel(FSPI_MCR0, uiReg);
}

static void fspi_lock_LUT(void)
{
	fspi_writel(FSPI_LUTKEY, FSPI_LUTKEY_VALUE);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
	fspi_writel(FSPI_LCKCR, FSPI_LCKER_LOCK);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
}

static void fspi_unlock_LUT(void)
{
	fspi_writel(FSPI_LUTKEY,  FSPI_LUTKEY_VALUE);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
	fspi_writel(FSPI_LCKCR, FSPI_LCKER_UNLOCK);
	VERBOSE("%s 0x%x\n", __func__, fspi_readl(FSPI_LCKCR));
}

static void fspi_setup_LUT(void)
{
	uint32_t xAddr, xInstr0, xInstr1;

	VERBOSE("In func %s\n", __func__);
	fspi_unlock_LUT();

	/* LUT Setup for READ Command 3-Byte low Frequency */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiREAD_SEQ_ID);
	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_READ)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr), xInstr1 | xInstr0);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_READ", xAddr);
	} else {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_READ_4B)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr), xInstr1 | xInstr0);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_READ_4B", xAddr);
	}
	xInstr0 = FSPI_INSTR_OPRND0(0)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_LUT_READ);
	xInstr1 = 0;
	fspi_writel((xAddr + 0x4), (xInstr1 + xInstr0));
	fspi_writel((xAddr + 0x8), (uint32_t) 0x0);	/* STOP command */
	fspi_writel((xAddr + 0xc), (uint32_t) 0x0);	/* STOP command */

	/* LUT Setup for FAST READ Command 3-Byte/4-Byte high Frequency */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiFASTREAD_SEQ_ID);
	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_FASTREAD)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr), xInstr1 | xInstr0);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_FASTREAD", xAddr);
	} else {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_FASTREAD_4B)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr), xInstr1 | xInstr0);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_FASTREAD_4B",
			xAddr);

	}
	xInstr0 = FSPI_INSTR_OPRND0(8)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_DUMMY_SDR)
		  | FSPI_INSTR_OPRND1(0)
		  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE1(FSPI_LUT_READ);
	xInstr1 = 0;
	fspi_writel((xAddr + 0x4), (xInstr1 + xInstr0));
	fspi_writel((xAddr + 0x8), (uint32_t) 0x0);	/* STOP command */
	fspi_writel((xAddr + 0xc), (uint32_t) 0x0);	/* STOP command */

	/* LUT Setup for Page Program */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiWRITE_SEQ_ID);
	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_PP)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr + 0x0), (xInstr1 | xInstr0));
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_PP", xAddr);
	} else {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_PP_4B)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
		xInstr1 = FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		fspi_writel((xAddr + 0x0), (xInstr1 | xInstr0));
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_PP_4B", xAddr);
	}
	xInstr0 = FSPI_INSTR_OPRND0(0)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_LUT_WRITE);
	xInstr1 = 0;
	fspi_writel((xAddr + 0x4), (xInstr1 | xInstr0));
	fspi_writel((xAddr + 0x8), (uint32_t) 0x0);	/* STOP command */
	fspi_writel((xAddr + 0xc), (uint32_t) 0x0);	/* STOP command */

	/* LUT Setup for WREN */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiWREN_SEQ_ID);
	xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_WREN)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
	fspi_writel((xAddr + 0x0), (xInstr0));
	VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_WREN", xAddr);
	fspi_writel((xAddr + 0x4), (uint32_t) 0x0);
	fspi_writel((xAddr + 0x8), (uint32_t) 0x0);	/* STOP command */
	fspi_writel((xAddr + 0xc), (uint32_t) 0x0);	/* STOP command */

	/* LUT Setup for Sector_Erase */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiSE_SEQ_ID);
	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_SE_64K)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD)
			  | FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_SE_64K", xAddr);
	} else {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_SE_64K_4B)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD)
			  | FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_SE_64K_4B", xAddr);
	}

	fspi_writel((xAddr + 0x0),  (xInstr0));
	fspi_writel((xAddr + 0x4),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0x8),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0xc),  (uint32_t) 0x0);

	/* LUT Setup for Sub Sector 4K Erase */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiSE_4K_SEQ_ID);
	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES) {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_SE_4K)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD)
			  | FSPI_INSTR_OPRND1(FSPI_LUT_ADDR24BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_SE_4K", xAddr);
	} else {
		xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_SE_4K_4B)
			  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD)
			  | FSPI_INSTR_OPRND1(FSPI_LUT_ADDR32BIT)
			  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
			  | FSPI_INSTR_OPCODE1(FSPI_LUT_ADDR);
		VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_SE_4K_4B", xAddr);
	}

	fspi_writel((xAddr + 0x0),  (xInstr0));
	fspi_writel((xAddr + 0x4),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0x8),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0xc),  (uint32_t) 0x0);

	/* LUT Setup for Bulk_Erase */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiBE_SEQ_ID);
	xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_BE)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD);
	fspi_writel((xAddr + 0x0),  (xInstr0));
	VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_BE", xAddr);
	fspi_writel((xAddr + 0x4),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0x8),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0xc),  (uint32_t) 0x0);

	/* Read Status */
	xAddr = FSPI_LUTREG_OFFSET + (uint32_t)(0x10 * fspiRDSR_SEQ_ID);
	xInstr0 = FSPI_INSTR_OPRND0(FSPI_NOR_CMD_RDSR)
		  | FSPI_INSTR_PAD0(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE0(FSPI_LUT_CMD)
		  | FSPI_INSTR_OPRND1(1)
		  | FSPI_INSTR_PAD1(FSPI_LUT_PAD1)
		  | FSPI_INSTR_OPCODE1(FSPI_LUT_READ);
	fspi_writel((xAddr + 0x0),  (xInstr0));
	VERBOSE("%s offset = 0x%x\n", "FSPI_NOR_CMD_RDSR", xAddr);
	fspi_writel((xAddr + 0x4),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0x8),  (uint32_t) 0x0);
	fspi_writel((xAddr + 0xc),  (uint32_t) 0x0);

	fspi_lock_LUT();
}

static inline void fspi_ahb_invalidate(void)
{
	uint32_t reg;

	VERBOSE("In func %s %d\n", __func__, __LINE__);
	reg = fspi_readl(FSPI_MCR0);
	reg |= FSPI_MCR0_SWRST;
	fspi_writel(FSPI_MCR0, reg);
	while ((fspi_readl(FSPI_MCR0) & FSPI_MCR0_SWRST))
		;  /* FSPI_MCR0_SWRESET_MASK */
	VERBOSE("In func %s %d\n", __func__, __LINE__);

}

#if defined(CONFIG_FSPI_AHB)
static void fspi_init_ahb(void)
{
	uint32_t i, xFlashCR2, seq_id;

	xFlashCR2 = 0;
	/* Reset AHB RX buffer CR configuration */
	for (i = 0; i < 8; i++)
		fspi_writel((FSPI_AHBRX_BUF0CR0 + 4 * i), 0);

	// Set ADATSZ with the maximum AHB buffer size
	fspi_writel(FSPI_AHBRX_BUF7CR0,
			((uint32_t) ((FSPI_RX_MAX_AHBBUF_SIZE / 8) |
				    FSPI_AHBRXBUF0CR7_PREF)));

	// Known limitation handling: prefetch and no start address alignment.
	fspi_writel(FSPI_AHBCR, FSPI_AHBCR_PREF_EN);
	INFO("xAhbcr=0x%x\n", fspi_readl(FSPI_AHBCR));

	// Setup AHB READ sequenceID for all flashes.
	xFlashCR2 = fspi_readl(FSPI_FLSHA1CR2);
	INFO("xFlashCR2=0x%x\n", xFlashCR2);

	seq_id = CONFIG_FSPI_FASTREAD ?
			fspiFASTREAD_SEQ_ID : fspiREAD_SEQ_ID;
	xFlashCR2 |= ((seq_id << FSPI_FLSHXCR2_ARDSEQI_SHIFT) & 0x1f);

	INFO("xFlashCR2=0x%x\n", xFlashCR2);

	fspi_writel(FSPI_FLSHA1CR2,  xFlashCR2);
	xFlashCR2 = fspi_readl(FSPI_FLSHA1CR2);
	INFO("xFlashCR2=0x%x\n", xFlashCR2);

/* TODO Not needed since there is no alternate CS
 *	fspi_writel(FSPI_FLSHA2CR2,  fspiREAD_SEQ_ID);
 *	fspi_writel(FSPI_FLSHB1CR2,  fspiREAD_SEQ_ID);
 *	fspi_writel(FSPI_FLSHB2CR2,  fspiREAD_SEQ_ID);
 */
}
#endif

int xspi_read(uint32_t pcRxAddr, uint32_t *pcRxBuf, uint32_t xSize_Bytes)
{
	if (!xSize_Bytes) {
		ERROR("Zero length reads are not allowed\n");
		return -XSPI_READ_FAIL;
	}

#if defined(CONFIG_FSPI_AHB)
	return xspi_ahb_read(pcRxAddr, pcRxBuf, xSize_Bytes);
#else
	return xspi_ip_read(pcRxAddr, pcRxBuf, xSize_Bytes);
#endif
}
#if defined(CONFIG_FSPI_AHB)
int xspi_ahb_read(uint32_t pcRxAddr, uint32_t *pcRxBuf, uint32_t xSize_Bytes)
{
	VERBOSE("In func %s 0x%x\n", __func__, (pcRxAddr));

	if (F_FLASH_SIZE_BYTES <= SZ_16M_BYTES)
		pcRxAddr = ((uint32_t)(pcRxAddr & MASK_24BIT_ADDRESS));
	else
		pcRxAddr = ((uint32_t)(pcRxAddr & MASK_32BIT_ADDRESS));

	pcRxAddr = ((uint32_t)(pcRxAddr + fspi_flash_base_addr));

	if (pcRxAddr % 4 || (uintptr_t)pcRxBuf % 4) {
		WARN("%s: unaligned Start Address src=%ld dst=0x%p\n",
		     __func__, (pcRxAddr - fspi_flash_base_addr), pcRxBuf);
	}

	/* Directly copy from AHB Buffer */
	memcpy(pcRxBuf, (void *)(uintptr_t)pcRxAddr, xSize_Bytes);

	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}
#endif

int xspi_ip_read(uint32_t pcRxAddr, uint32_t *pvRxBuf, uint32_t uiLen)
{

	uint32_t i = 0, j = 0, xRem = 0;
	uint32_t xIteration = 0, xSizeRx = 0, xSizeWm, temp_size;
	uint32_t data = 0;
	uint32_t xLen_Bytes;
	uint32_t xAddr, sts0, intr, seq_id;

	xAddr = (uint32_t) pcRxAddr;
	xLen_Bytes = uiLen;

	/* Watermark level : 8 bytes. (BY DEFAULT) */
	xSizeWm = 8;

	/* Clear  RX Watermark interrupt in INT register, if any existing.  */
	fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA);
	PRA("0x%x", fspi_readl(FSPI_INTR));
	/* Invalid the RXFIFO, to run next IP Command */
	/* Clears data entries in IP Rx FIFOs, Also reset R/W pointers */
	fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	fspi_writel(FSPI_INTR, FSPI_INTEN_IPCMDDONE);

	while (xLen_Bytes) {

		/* FlexSPI can store no more than  FSPI_RX_IPBUF_SIZE */
		xSizeRx = (xLen_Bytes >  FSPI_RX_IPBUF_SIZE) ?
			   FSPI_RX_IPBUF_SIZE : xLen_Bytes;

		/* IP Control Register0 - SF Address to be read */
		fspi_writel(FSPI_IPCR0, xAddr);
		PRA("0x%x", fspi_readl(FSPI_IPCR0));
		/* IP Control Register1 - SEQID_READ operation, Size */

		seq_id = CONFIG_FSPI_FASTREAD ?
				fspiFASTREAD_SEQ_ID : fspiREAD_SEQ_ID;

		fspi_writel(FSPI_IPCR1,
			    (uint32_t)(seq_id << FSPI_IPCR1_ISEQID_SHIFT) |
			    (uint16_t) xSizeRx);

		PRA("0x%x", fspi_readl(FSPI_IPCR1));

		do {
			sts0 = fspi_readl(FSPI_STS0);
			if ((sts0 & FSPI_STS0_ARB_IDLE) &&
			    (sts0 & FSPI_STS0_SEQ_IDLE))
				break;
		} while (1);

		/* Trigger IP Read Command */
		fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);
		PRA("0x%x", fspi_readl(FSPI_IPCMD));

		intr = fspi_readl(FSPI_INTR);
		if ((intr & FSPI_INTR_IPCMDGE) ||
		    (intr & FSPI_INTR_IPCMDERR)) {
			ERROR("Error in IP READ INTR=0x%x\n", intr);
			return -XSPI_IP_READ_FAIL;
		}
		/* Will read in n iterations of each 8 FIFO's(WM level) */
		xIteration = xSizeRx / xSizeWm;
		for (i = 0; i < xIteration; i++) {
			if (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPRXWA_MASK)) {
				PRA("0x%x", fspi_readl(FSPI_INTR));
			}
			/* Wait for IP Rx Watermark Fill event */
			while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPRXWA_MASK)) {
				PRA("0x%x", fspi_readl(FSPI_INTR));
			}

			/* Read RX FIFO's(upto WM level) & copy to rxbuffer */
			for (j = 0; j < xSizeWm; j += 4) {
				/* Read FIFO Data Register */
				data = fspi_readl(FSPI_RFDR + j);
#if FSPI_IPDATA_SWAP /* Just In case you want swap */
				data = bswap32(data);
#endif
				memcpy(pvRxBuf++, &data, 4);
			}

			/* Clear IP_RX_WATERMARK Event in INTR register */
			/* Reset FIFO Read pointer for next iteration.*/
			fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA);
		}

		xRem = xSizeRx % xSizeWm;

		if (xRem) {
			/* Wait for data filled */
			while (!(fspi_readl(FSPI_IPRXFSTS) & FSPI_IPRXFSTS_FILL_MASK)) {
				PRA("0x%x", fspi_readl(FSPI_IPRXFSTS));
			}

			temp_size = 0;
			j = 0;
			while (xRem > 0) {
				data = 0;
				data =  fspi_readl(FSPI_RFDR + j);
#if FSPI_IPDATA_SWAP /* Just In case you want swap */
				data = bswap32(data);
#endif
				temp_size = (xRem < 4) ? xRem : 4;
				memcpy(pvRxBuf++, &data, temp_size);
				xRem -= temp_size;
			}
		}


		while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK)) {
			PRA("0x%x", fspi_readl(FSPI_INTR));
		}

		/* Invalid the RX FIFO, to run next IP Command */
		fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
		/* Clear IP Command Done flag in interrupt register*/
		fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

		/* Update remaining len, Increment xAddr read pointer. */
		xLen_Bytes -= xSizeRx;
		xAddr += xSizeRx;
	}
	PR;
	return XSPI_SUCCESS;
}

void xspi_ip_write(uint32_t pcWrAddr, uint32_t *pvWrBuf, uint32_t uiLen)
{

	uint32_t xIteration = 0, xRem = 0;
	uint32_t xSizeTx = 0, xSizeWm, temp_size;
	uint32_t i = 0, j = 0;
	uint32_t uiData = 0;
	uint32_t xAddr, xLen_Bytes;


	xSizeWm = 8;	/* Default TX WaterMark level: 8 Bytes. */
	xAddr = (uint32_t)pcWrAddr;
	xLen_Bytes = uiLen;
	VERBOSE("In func %s[%d] xAddr =0x%x xLen_bytes=%d\n",
			__func__, __LINE__, xAddr, xLen_Bytes);

	/* Invalid the TXFIFO, to run next IP Command remove this */
	//fspi_writel( FSPI_IPTXFCR, FSPI_IPTXFCR_CLR );

	while (xLen_Bytes) {

		xSizeTx = (xLen_Bytes >  FSPI_TX_IPBUF_SIZE) ?
				FSPI_TX_IPBUF_SIZE : xLen_Bytes;

		/* IP Control Register0 - SF Address to be read */
		fspi_writel(FSPI_IPCR0, xAddr);
		INFO("In func %s[%d] xAddr =0x%x xLen_bytes=%d\n",
				__func__, __LINE__, xAddr, xLen_Bytes);

		/*
		 * Fill TX FIFO's..
		 *
		 */

		xIteration = xSizeTx / xSizeWm;
		for (i = 0; i < xIteration; i++) {

			/* Ensure TX FIFO Watermark Available */
			while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPTXWE_MASK))
				;


			/* Fill TxFIFO's ( upto watermark level) */
			for (j = 0; j < xSizeWm; j += 4) {
				memcpy(&uiData, pvWrBuf++,  4);
				/* Write TX FIFO Data Register */
				fspi_writel((FSPI_TFDR + j), uiData);

			}

			/* Clear IP_TX_WATERMARK Event in INTR register */
			/* Reset the FIFO Write pointer for next iteration */
			fspi_writel(FSPI_INTR, FSPI_INTR_IPTXWE);
		}

		xRem = xSizeTx % xSizeWm;

		if (xRem) {
			/* Wait for TXFIFO empty */
			while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPTXWE))
				;

			temp_size = 0;
			j = 0;
			while (xRem > 0) {
				uiData = 0;
				temp_size = (xRem < 4) ? xRem : 4;
				memcpy(&uiData, pvWrBuf++, temp_size);
				INFO("%d ---> pvWrBuf=0x%p\n", __LINE__, pvWrBuf);
				fspi_writel((FSPI_TFDR + j), uiData);
				xRem -= temp_size;
				j += 4 ; /* TODO: May not be needed*/
			}
			/* Clear IP_TX_WATERMARK Event in INTR register */
			/* Reset FIFO's Write pointer for next iteration.*/
			fspi_writel(FSPI_INTR, FSPI_INTR_IPTXWE);
		}

		/* IP Control Register1 - SEQID_WRITE operation, Size */
		fspi_writel(FSPI_IPCR1, (uint32_t)(fspiWRITE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | (uint16_t) xSizeTx);
		/* Trigger IP Write Command */
		fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

		/* Wait for IP Write command done */
		while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK))
			;

		/* Invalidate TX FIFOs & acknowledge IP_CMD_DONE event */
		fspi_writel(FSPI_IPTXFCR, FSPI_IPTXFCR_CLR);
		fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

		/* for next iteration */
		xLen_Bytes  -=  xSizeTx;
		xAddr += xSizeTx;
	}

}

int xspi_write(uint32_t pcWrAddr, void *pvWrBuf, uint32_t uiLen)
{

	uint32_t xAddr;
	uint32_t xPage1_Len = 0, xPageL_Len = 0;
	uint32_t i, j = 0;
	void *Buf = pvWrBuf;

	VERBOSE("\nIn func %s\n", __func__);

	xAddr = (uint32_t)(pcWrAddr);
	if ((uiLen <= F_PAGE_256) && !(xAddr % F_PAGE_256)) {
		xPage1_Len = uiLen;
		INFO("%d ---> xPage1_Len=0x%x xPageL_Len =0x%x j=0x%x\n", __LINE__, xPage1_Len, xPageL_Len, j);
	} else if ((uiLen <= F_PAGE_256) && (xAddr % F_PAGE_256)) {
		xPage1_Len = (F_PAGE_256 - (xAddr % F_PAGE_256));
		if (uiLen > xPage1_Len) {
			xPageL_Len = (uiLen - xPage1_Len) % F_PAGE_256;
		} else {
			xPage1_Len = uiLen;
			xPageL_Len = 0;
		}
		j = 0;
		INFO("%d 0x%x 0x%x\n", xAddr % F_PAGE_256, xAddr % F_PAGE_256, F_PAGE_256);
		INFO("%d ---> xPage1_Len=0x%x xPageL_Len =0x%x j=0x%x\n", __LINE__, xPage1_Len, xPageL_Len, j);
	} else if ((uiLen > F_PAGE_256) && !(xAddr % F_PAGE_256)) {
		j = uiLen / F_PAGE_256;
		xPageL_Len = uiLen % F_PAGE_256;
		INFO("%d ---> xPage1_Len=0x%x xPageL_Len =0x%x j=0x%x\n", __LINE__, xPage1_Len, xPageL_Len, j);
	} else if ((uiLen > F_PAGE_256) && (xAddr % F_PAGE_256)) {
		xPage1_Len = (F_PAGE_256 - (xAddr % F_PAGE_256));
		j = (uiLen - xPage1_Len) / F_PAGE_256;
		xPageL_Len = (uiLen - xPage1_Len) % F_PAGE_256;
		INFO("%d ---> xPage1_Len=0x%x xPageL_Len =0x%x j=0x%x\n", __LINE__, xPage1_Len, xPageL_Len, j);
	}

	if (xPage1_Len) {
		xspi_wren(xAddr);
		xspi_ip_write(xAddr, (uint32_t *)Buf, xPage1_Len);
		while (is_flash_busy())
			;
		INFO("%d Initial pcWrAddr=0x%x, Final xAddr=0x%x, Initial uiLen=0x%x Final uiLen=0x%x\n",
		     __LINE__, pcWrAddr, xAddr, uiLen, (xAddr-pcWrAddr));
		INFO("Initial Buf pvWrBuf=%p, final Buf=%p\n", pvWrBuf, Buf);
		xAddr += xPage1_Len;
		/* TODO What is buf start is not 4 aligned */
		Buf = Buf + xPage1_Len;
	}

	for (i = 0; i < j; i++) {
		INFO("In for loop Buf pvWrBuf=%p, final Buf=%p xAddr=0x%x offset_buf %d.\n",
				pvWrBuf, Buf, xAddr, xPage1_Len/4);
		xspi_wren(xAddr);
		xspi_ip_write(xAddr, (uint32_t *)Buf, F_PAGE_256);
		while (is_flash_busy())
			;
		INFO("%d Initial pcWrAddr=0x%x, Final xAddr=0x%x, Initial uiLen=0x%x Final uiLen=0x%x\n",
		     __LINE__, pcWrAddr, xAddr, uiLen, (xAddr-pcWrAddr));
		xAddr += F_PAGE_256;
		/* TODO What is buf start is not 4 aligned */
		Buf = Buf + F_PAGE_256;
		INFO("Initial Buf pvWrBuf=%p, final Buf=%p\n", pvWrBuf, Buf);
	}

	if (xPageL_Len) {
		INFO("%d Initial Buf pvWrBuf=%p, final Buf=%p xPageL_Len=0x%x\n", __LINE__, pvWrBuf, Buf, xPageL_Len);
		xspi_wren(xAddr);
		xspi_ip_write(xAddr, (uint32_t *)Buf, xPageL_Len);
		while (is_flash_busy())
			;
		INFO("%d Initial pcWrAddr=0x%x, Final xAddr=0x%x, Initial uiLen=0x%x Final uiLen=0x%x\n",
				__LINE__, pcWrAddr, xAddr, uiLen, (xAddr-pcWrAddr));
	}

	VERBOSE("Now calling func call Invalidate%s\n", __func__);
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}

int xspi_wren(uint32_t pcWrAddr)
{
	VERBOSE("In func %s Addr=0x%x\n", __func__, pcWrAddr);

	fspi_writel(FSPI_IPTXFCR, FSPI_IPTXFCR_CLR);

	fspi_writel(FSPI_IPCR0, (uint32_t)pcWrAddr);
	fspi_writel(FSPI_IPCR1, ((fspiWREN_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) |  0));
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK))
		;

	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);
	return XSPI_SUCCESS;
}

static void fspi_bbluk_er(void)
{
	VERBOSE("In func %s\n", __func__);
	fspi_writel(FSPI_IPCR0, 0x0);
	fspi_writel(FSPI_IPCR1, ((fspiBE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 20));
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK))
		;
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

}

static void fspi_RDSR(uint32_t *rxbuf, const void *p_addr, uint32_t size)
{
	uint32_t iprxfcr = 0;
	uint32_t data = 0;

	iprxfcr = fspi_readl(FSPI_IPRXFCR);
	/* IP RX FIFO would be read by processor */
	iprxfcr = iprxfcr & (uint32_t)~FSPI_IPRXFCR_CLR;
	/* Invalid data entries in IP RX FIFO */
	iprxfcr = iprxfcr | FSPI_IPRXFCR_CLR;
	fspi_writel(FSPI_IPRXFCR, iprxfcr);

	fspi_writel(FSPI_IPCR0, (uintptr_t) p_addr);
	fspi_writel(FSPI_IPCR1,
		    (uint32_t) ((fspiRDSR_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT)
		    | (uint16_t) size));
	/* Trigger the command */
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);
	/* Wait for command done */
	while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK))
		;
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

	data = fspi_readl(FSPI_RFDR);
	memcpy(rxbuf, &data, size);

	/* Rx FIFO invalidation needs to be done prior w1c of INTR.IPRXWA bit */
	fspi_writel(FSPI_IPRXFCR, FSPI_IPRXFCR_CLR);
	fspi_writel(FSPI_INTR, FSPI_INTR_IPRXWA_MASK);
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);

}

bool is_flash_busy(void)
{
#define FSPI_ONE_BYTE 1
	uint8_t data[4];

	VERBOSE("In func %s\n\n", __func__);
	fspi_RDSR((uint32_t *) data, 0, FSPI_ONE_BYTE);

	return !!((uint32_t) data[0] & FSPI_NOR_SR_WIP_MASK);
}

int xspi_bulk_erase(void)
{
	VERBOSE("In func %s\n", __func__);
	xspi_wren((uint32_t) 0x0);
	fspi_bbluk_er();
	while (is_flash_busy())
		;
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}

static void fspi_sec_er(uint32_t pcWrAddr)
{
	uint32_t xAddr;

	VERBOSE("In func %s\n", __func__);
	xAddr = (uint32_t)(pcWrAddr);

	fspi_writel(FSPI_IPCR0, xAddr);
	INFO("In [%s][%d] Erase address 0x%x\n", __func__, __LINE__, (xAddr));
#if CONFIG_FSPI_ERASE_4K
	fspi_writel(FSPI_IPCR1, ((fspiSE_4K_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 0));
#else
	fspi_writel(FSPI_IPCR1, ((fspiSE_SEQ_ID << FSPI_IPCR1_ISEQID_SHIFT) | 0));
#endif
	fspi_writel(FSPI_IPCMD, FSPI_IPCMD_TRG_MASK);

	while (!(fspi_readl(FSPI_INTR) & FSPI_INTR_IPCMDDONE_MASK)) {
		PRA("0x%x", fspi_readl(FSPI_INTR));
	}
	fspi_writel(FSPI_INTR, FSPI_INTR_IPCMDDONE_MASK);
}

int xspi_sector_erase(uint32_t pcWrAddr, uint32_t uiLen)
{
	uint32_t xAddr, xLen_Bytes, i, num_sector = 0;

	VERBOSE("In func %s\n", __func__);
	xAddr = (uint32_t)(pcWrAddr);
	if (xAddr % F_SECTOR_ERASE_SZ) {
		ERROR("!!! In func %s, unalinged start address can only be in multiples of 0x%x\n",
		      __func__, F_SECTOR_ERASE_SZ);
		return -XSPI_ERASE_FAIL;
	}

	xLen_Bytes = uiLen * 1;
	if (xLen_Bytes < F_SECTOR_ERASE_SZ) {
		ERROR("!!! In func %s, Less than 1 sector can only be in multiples of 0x%x\n",
				__func__, F_SECTOR_ERASE_SZ);
		return -XSPI_ERASE_FAIL;
	}

	num_sector = xLen_Bytes/F_SECTOR_ERASE_SZ;
	num_sector += xLen_Bytes % F_SECTOR_ERASE_SZ ? 1 : 0;
	INFO("F_SECTOR_ERASE_SZ: 0x%08x, num_sector: %d\n", F_SECTOR_ERASE_SZ, num_sector);

	for (i = 0; i < num_sector ; i++) {
		xspi_wren(xAddr + (F_SECTOR_ERASE_SZ * i));
		fspi_sec_er(xAddr + (F_SECTOR_ERASE_SZ * i));
		while (is_flash_busy())
			;
	}
	fspi_ahb_invalidate();
	return XSPI_SUCCESS;
}


__attribute__((unused)) static void  fspi_delay_ms(uint32_t x)
{
	volatile unsigned long  ulCount;

	for (ulCount = 0; ulCount < (30 * x); ulCount++)
		;

}


#if defined(DEBUG_FLEXSPI)
static void fspi_dump_regs(void)
{
	uint32_t i;

	VERBOSE("\nRegisters Dump:\n");
	VERBOSE("Flexspi: Register FSPI_MCR0(0x%x) = 0x%08x\n", FSPI_MCR0, fspi_readl(FSPI_MCR0));
	VERBOSE("Flexspi: Register FSPI_MCR2(0x%x) = 0x%08x\n", FSPI_MCR2, fspi_readl(FSPI_MCR2));
	VERBOSE("Flexspi: Register FSPI_DLL_A_CR(0x%x) = 0x%08x\n", FSPI_DLLACR, fspi_readl(FSPI_DLLACR));
	VERBOSE("\n");

	for (i = 0; i < 8; i++)
		VERBOSE("Flexspi: Register FSPI_AHBRX_BUF0CR0(0x%x) = 0x%08x\n", FSPI_AHBRX_BUF0CR0 + i * 4, fspi_readl((FSPI_AHBRX_BUF0CR0 + i * 4)));
	VERBOSE("\n");

	VERBOSE("Flexspi: Register FSPI_AHBRX_BUF7CR0(0x%x) = 0x%08x\n", FSPI_AHBRX_BUF7CR0, fspi_readl(FSPI_AHBRX_BUF7CR0));
	VERBOSE("Flexspi: Register FSPI_AHB_CR(0x%x) \t  = 0x%08x\n", FSPI_AHBCR, fspi_readl(FSPI_AHBCR));
	VERBOSE("\n");

	for (i = 0; i < 4; i++)
		VERBOSE("Flexspi: Register FSPI_FLSH_A1_CR2,(0x%x) = 0x%08x\n", FSPI_FLSHA1CR2 + i * 4, fspi_readl(FSPI_FLSHA1CR2 + i * 4));
}
#endif

int fspi_init(uint32_t base_reg_addr, uint32_t flash_start_addr)
{
	uint32_t	mcrx;
	uint32_t	flash_size;

	if (fspi_base_reg_addr) {
		INFO("FSPI is already initialized.\n");
		return XSPI_SUCCESS;
	}

	fspi_base_reg_addr = base_reg_addr;
	fspi_flash_base_addr = flash_start_addr;

	INFO("Flexspi driver: Version v1.0\n");
	INFO("Flexspi: Default MCR0 = 0x%08x, before reset\n", fspi_readl(FSPI_MCR0));
	VERBOSE("Flexspi: Resetting controller...\n");

	/* Reset FlexSpi Controller */
	fspi_writel(FSPI_MCR0, FSPI_MCR0_SWRST);
	while ((fspi_readl(FSPI_MCR0) & FSPI_MCR0_SWRST))
		;  /* FSPI_MCR0_SWRESET_MASK */


	/* Disable Controller Module before programming its registersi, especially MCR0 (Master Control Register0) */
	fspi_MDIS(1);
	/*
	 * Program MCR0 with default values, AHB Timeout(0xff), IP Timeout(0xff).  {FSPI_MCR0- 0xFFFF0000}
	 */

	/* Timeout wait cycle for AHB command grant */
	mcrx = fspi_readl(FSPI_MCR0);
	mcrx |= (uint32_t)((FSPI_MAX_TIMEOUT_AHBCMD << FSPI_MCR0_AHBGRANTWAIT_SHIFT) & (FSPI_MCR0_AHBGRANTWAIT_MASK));

	/* Time out wait cycle for IP command grant*/
	mcrx |= (uint32_t) (FSPI_MAX_TIMEOUT_IPCMD << FSPI_MCR0_IPGRANTWAIT_SHIFT) & (FSPI_MCR0_IPGRANTWAIT_MASK);

	/* TODO why BE64 set BE32*/
	mcrx |= (uint32_t) (FSPI_ENDCFG_LE64 << FSPI_MCR0_ENDCFG_SHIFT) & FSPI_MCR0_ENDCFG_MASK;

	fspi_writel(FSPI_MCR0, mcrx);

	/* Reset the DLL register to default value */
	fspi_writel(FSPI_DLLACR, FSPI_DLLACR_OVRDEN);
	fspi_writel(FSPI_DLLBCR, FSPI_DLLBCR_OVRDEN);

#if ERRATA_FLASH_A050272	/* ERRATA DLL */
	for (uint8_t delay = 100U; delay > 0U; delay--)	{
		__asm__ volatile ("nop");
	}
#endif

/*
 *	// Disable SameDeviceEnable, to configure each chip select device indivisualy
 *	fspi_writel(FSPI_MCR2, (fspi_readl(FSPI_MCR2) | (~((uint32_t) FSPI_MCR2_SAMEDEVICEEN))));
 */
	/* Configure flash control registers for different chip select */
	flash_size = (F_FLASH_SIZE_BYTES * FLASH_NUM) / FSPI_BYTES_PER_KBYTES;
	fspi_writel(FSPI_FLSHA1CR0, flash_size);
/*
 *	fspi_writel(FSPI_FLSHA1CR1, (((20 <<  FSPI_FLSHXCR1_TCSH_SHIFT) & FSPI_FLSHXCR1_TCSH_MASK) |
 *				((4 << FSPI_FLSHXCR1_TCSS_SHIFT) & FSPI_FLSHXCR1_TCSS_MASK)));
 */
	fspi_writel(FSPI_FLSHA2CR0, 0);
	fspi_writel(FSPI_FLSHB1CR0, 0);
	fspi_writel(FSPI_FLSHB2CR0, 0);

#if defined(CONFIG_FSPI_AHB)
	fspi_init_ahb();
#endif

	/* RE-Enable Controller Module */
	fspi_MDIS(0);
	INFO("Flexspi: After MCR0 = 0x%08x,\n", fspi_readl(FSPI_MCR0));
	fspi_setup_LUT();

	/* Dump of all registers, ensure controller not disabled anymore*/
#if defined(DEBUG_FLEXSPI)
	fspi_dump_regs();
#endif

	INFO("Flexspi: Init done!!\n");

#if DEBUG_FLEXSPI

	uint32_t xpiSfAddress = SZ_57M;

	/*
	 * Second argument of fspi_test is the size of buffer(s) passed
	 * to the function.
	 * SIZE_BUFFER defined in test_fspi.c is kept large enough to
	 * accommodate variety of sizes for regressive tests.
	 */
	fspi_test(xpiSfAddress, 0x40, 0);
	fspi_test(xpiSfAddress, 0x15, 2);
	fspi_test(xpiSfAddress, 0x80, 0);
	fspi_test(xpiSfAddress, 0x81, 0);
	fspi_test(xpiSfAddress, 0x79, 3);

	fspi_test(xpiSfAddress + 0x11, 0x15, 0);
	fspi_test(xpiSfAddress + 0x11, 0x40, 0);
	fspi_test(xpiSfAddress + 0xff, 0x40, 1);
	fspi_test(xpiSfAddress + 0x25, 0x81, 2);
	fspi_test(xpiSfAddress + 0xef, 0x6f, 3);

	fspi_test((xpiSfAddress - F_SECTOR_ERASE_SZ), 0x229, 0);
#endif

	return XSPI_SUCCESS;
}
