/*
 * Copyright 2020-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef ERRATA_H
#define ERRATA_H

void soc_errata(void);

#ifdef ERRATA_SOC_A008850
void erratum_a008850_post(void);
#endif

#endif /* ERRATA_H */
