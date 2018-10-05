/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <platform_def.h>
#include <utils_def.h>
#include <imx_rdc.h>
#include <mmio.h>

/*
 * Read RDC settings for one peripheral
 * read the given domains field and lock bit
 * for the given PDAP index [0..118]
 */
int imx_rdc_get_pdap(struct rdc_pdap_conf *p)
{
	struct imx_rdc_regs *imx_rdc = (struct imx_rdc_regs *)IMX_RDC_BASE;
	uint32_t reg = 0;

	reg = mmio_read_32((uintptr_t)&imx_rdc->pdap[p->index]);
	p->lock = (reg & RDC_PDAP_LCK_MASK) >> RDC_PDAP_LCK_SHIFT;
	p->domains = reg & 0xFF;

	return 0;
}

/*
 * Write RDC settings for one peripheral
 * Check before write if is already locked then skip
 */
int imx_rdc_set_pdap(struct rdc_pdap_conf *p)
{
	struct imx_rdc_regs *imx_rdc = (struct imx_rdc_regs *)IMX_RDC_BASE;
	struct rdc_pdap_conf r;
	uint32_t i, reg = 0;
	uint8_t multi_domain = 0;

	/* Check if locked */
	r.index = p->index;
	imx_rdc_get_pdap(&r);
	if (r.lock) {
		WARN("RDC_PDAPn %d is already locked \n", p->index);
		return -ENOENT;
	}

	/* Check if no domain assigned */
	if (p->domains == 0)
		return -EINVAL;
	reg |= p->domains;

	/* Check if SREQ is needed */
	for (i = 0; i < 7; i += 2)
		multi_domain += ((p->domains >> i) & 0x3) ? 1 : 0;
	if (multi_domain > 1)
		reg |= RDC_PDAP_SREQ_MASK;
	/* Setup Lock from input */
	reg |= p->lock << RDC_PDAP_LCK_SHIFT;

	NOTICE("imx_rdc_set_pdap(): write addr=0x%p, reg=0x%x\n",
			&imx_rdc->pdap[p->index], reg);

	mmio_write_32((uintptr_t)&imx_rdc->pdap[p->index], reg);

	return 0;
}

/*
 * Setup RDC settings for multiple peripherals
 */
int imx_rdc_set_peripherals(struct rdc_pdap_conf *peripherals_list,
				uint32_t count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		ret = imx_rdc_set_pdap(&peripherals_list[i]);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Read RDC setting for one master
 * For the given index in p.index it returns the lock bit
 * and the domain field into p structure.
 */
int imx_rdc_get_mda(struct rdc_mda_conf *p)
{
	struct imx_rdc_regs *imx_rdc = (struct imx_rdc_regs *)IMX_RDC_BASE;
	uint32_t reg = 0;

	reg = mmio_read_32((uintptr_t)&imx_rdc->mda[p->index]);
	p->domain = reg & RDC_MDA_DID_MASK;
	p->lock = (reg & RDC_MDA_LCK_MASK) >> RDC_MDA_LCK_SHIFT;
	return 0;
}

/*
 * Write RDC setting for one master
 */
int imx_rdc_set_mda(struct rdc_mda_conf *p)
{
	struct imx_rdc_regs *imx_rdc = (struct imx_rdc_regs *)IMX_RDC_BASE;
	struct rdc_mda_conf r;
	uint32_t reg = 0;
	int ret = 0;

	/* Check if it is locked */
	r.index = p->index;
	imx_rdc_get_mda(&r);
	if (!r.lock) {
	reg = (p->domain & RDC_MDA_DID_MASK)
		| ((p->lock << RDC_MDA_LCK_SHIFT) & RDC_MDA_LCK_MASK);
	NOTICE("imx_rdc_setup_mda(): write addr=%p, reg=0x%x\n",
			&imx_rdc->mda[p->index], reg);
	mmio_write_32((uintptr_t)&imx_rdc->mda[p->index], reg);
	} else {
		WARN("RDC_MDAn %d is already locked \n", p->index);
		ret = -ENOENT;
	}

	return ret;
}

/*
 * Setup RDC settings for multiple masters
 */
int imx_rdc_set_masters(struct rdc_mda_conf *masters_list, uint32_t count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		ret = imx_rdc_set_mda(&masters_list[i]);
		if (ret)
			break;
	}

	return ret;
}

#if defined (CSU_RDC_TEST)
/* Default peripherals settings as an example */
static struct rdc_pdap_conf periph_config[] = {
	{RDC_PDAP_CSU, 0x30, 0},
};


/* Default masters settings as an example */
static struct rdc_mda_conf masters_config[] = {
	{RDC_MDA_A53, 0, 0},
};
#else
/* Default peripherals settings as an example */
static struct rdc_pdap_conf periph_config[] = {
	{RDC_PDAP_GPIO1, 0x3, 0},
	{RDC_PDAP_GPIO2, 0x3, 0},
	{RDC_PDAP_GPIO3, 0x3, 0},
	{RDC_PDAP_GPIO4, 0x3, 0},
	{RDC_PDAP_GPIO5, 0x3, 0},
};

/* Default masters settings as an example */
static struct rdc_mda_conf masters_config[] = {
	{RDC_MDA_A53, 0, 0},
	{RDC_MDA_CAAM, 0, 0},
};
#endif
void imx_rdc_set_peripherals_default(void)
{
	imx_rdc_set_peripherals(periph_config, ARRAY_SIZE(periph_config));
}

void imx_rdc_set_masters_default(void)
{
	imx_rdc_set_masters(masters_config, ARRAY_SIZE(masters_config));
}
#if defined (CSU_RDC_TEST)
void rdc_test(void)
{
	imx_rdc_set_peripherals_default();
	imx_rdc_set_masters_default();
}
#endif
