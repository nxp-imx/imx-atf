/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCMI_IMX9_H
#define SCMI_IMX9_H

#include <stddef.h>
#include <stdint.h>

#define SCMI_CPU_SLEEP_RUN			0
#define SCMI_CPU_SLEEP_WAIT			1
#define SCMI_CPU_SLEEP_STOP			2
#define SCMI_CPU_SLEEP_SUSPEND			3

#define SCMI_CPU_PD_LPM_ON_NEVER		0U
#define SCMI_CPU_PD_LPM_ON_RUN			1U
#define SCMI_CPU_PD_LPM_ON_RUN_WAIT		2U
#define SCMI_CPU_PD_LPM_ON_RUN_WAIT_STOP	3U
#define SCMI_CPU_PD_LPM_ON_ALWAYS		4U

#if defined(PLAT_imx95)
#define CPU_PER_LPI_IDX_GPIO1 			0U
#define CPU_PER_LPI_IDX_GPIO2 			1U
#define CPU_PER_LPI_IDX_GPIO3 			2U
#define CPU_PER_LPI_IDX_GPIO4 			3U
#define CPU_PER_LPI_IDX_GPIO5 			4U
#define CPU_PER_LPI_IDX_CAN1 			5U
#define CPU_PER_LPI_IDX_CAN2 			6U
#define CPU_PER_LPI_IDX_CAN3 			7U
#define CPU_PER_LPI_IDX_CAN4 			8U
#define CPU_PER_LPI_IDX_CAN5 			9U
#define CPU_PER_LPI_IDX_LPUART1 		10U
#define CPU_PER_LPI_IDX_LPUART2 		11U
#define CPU_PER_LPI_IDX_LPUART3 		12U
#define CPU_PER_LPI_IDX_LPUART4 		13U
#define CPU_PER_LPI_IDX_LPUART5 		14U
#define CPU_PER_LPI_IDX_LPUART6 		15U
#define CPU_PER_LPI_IDX_LPUART7 		16U
#define CPU_PER_LPI_IDX_LPUART8 		17U
#define CPU_PER_LPI_IDX_WDOG3 			18U
#define CPU_PER_LPI_IDX_WDOG4 			19U
#define CPU_PER_LPI_IDX_WDOG5 			20U
#elif defined(PLAT_imx94)
#define CPU_PER_LPI_IDX_GPIO1			0U
#define CPU_PER_LPI_IDX_GPIO2			1U
#define CPU_PER_LPI_IDX_GPIO3			2U
#define CPU_PER_LPI_IDX_GPIO4			3U
#define CPU_PER_LPI_IDX_GPIO5			4U
#define CPU_PER_LPI_IDX_GPIO6			5U
#define CPU_PER_LPI_IDX_GPIO7			6U
#define CPU_PER_LPI_IDX_CAN1			7U
#define CPU_PER_LPI_IDX_CAN2			8U
#define CPU_PER_LPI_IDX_CAN3			9U
#define CPU_PER_LPI_IDX_CAN4			10U
#define CPU_PER_LPI_IDX_CAN5			11U
#define CPU_PER_LPI_IDX_LPUART1			12U
#define CPU_PER_LPI_IDX_LPUART2			13U
#define CPU_PER_LPI_IDX_LPUART3			14U
#define CPU_PER_LPI_IDX_LPUART4			15U
#define CPU_PER_LPI_IDX_LPUART5			16U
#define CPU_PER_LPI_IDX_LPUART6			17U
#define CPU_PER_LPI_IDX_LPUART7			18U
#define CPU_PER_LPI_IDX_LPUART8			19U
#define CPU_PER_LPI_IDX_LPUART9			20U
#define CPU_PER_LPI_IDX_LPUART10		21U
#define CPU_PER_LPI_IDX_LPUART11		22U
#define CPU_PER_LPI_IDX_LPUART12		23U
#define CPU_PER_LPI_IDX_WDOG3			24U
#define CPU_PER_LPI_IDX_WDOG4			25U
#define CPU_PER_LPI_IDX_WDOG5			26U
#define CPU_PER_LPI_IDX_WDOG6			27U
#define CPU_PER_LPI_IDX_WDOG7			28U
#define CPU_PER_LPI_IDX_WDOG8			29U
#else /* imx952 */
#define CPU_PER_LPI_IDX_GPIO1 			0U
#define CPU_PER_LPI_IDX_GPIO2 			1U
#define CPU_PER_LPI_IDX_GPIO3 			2U
#define CPU_PER_LPI_IDX_GPIO4 			3U
#define CPU_PER_LPI_IDX_GPIO5 			4U
#define CPU_PER_LPI_IDX_CAN1 			5U
#define CPU_PER_LPI_IDX_CAN2 			6U
#define CPU_PER_LPI_IDX_CAN3 			7U
#define CPU_PER_LPI_IDX_LPUART1 		8U
#define CPU_PER_LPI_IDX_LPUART2 		9U
#define CPU_PER_LPI_IDX_LPUART3 		10U
#define CPU_PER_LPI_IDX_LPUART4 		11U
#define CPU_PER_LPI_IDX_LPUART5 		12U
#define CPU_PER_LPI_IDX_LPUART6 		13U
#define CPU_PER_LPI_IDX_LPUART7 		14U
#define CPU_PER_LPI_IDX_LPUART8 		15U
#define CPU_PER_LPI_IDX_WDOG3 			16U
#define CPU_PER_LPI_IDX_WDOG4 			17U
#define CPU_PER_LPI_IDX_WDOG5 			18U
#endif

#define MAX_PER_LPI_CONFIGS_PER_CMD		9

#define IMX9_SCMI_PERF_PROTO_ID			0x13

#define IMX9_SCMI_CORE_PERFLEVELSET_MSG		0x7
#define IMX9_SCMI_CORE_PERFLEVELSET_MSG_LEN	12
#define IMX9_SCMI_CORE_PERFLEVELSET_RESP_LEN	8

#define IMX9_SCMI_CORE_PROTO_ID			0x82

#define IMX9_SCMI_CORE_START_MSG		0x4
#define IMX9_SCMI_CORE_START_MSG_LEN		8
#define IMX9_SCMI_CORE_START_RESP_LEN		8

#define IMX9_SCMI_CORE_STOP_MSG			0x5
#define IMX9_SCMI_CORE_STOP_MSG_LEN		8
#define IMX9_SCMI_CORE_STOP_RESP_LEN		8

#define IMX9_SCMI_CORE_RESET_ADDR_SET_MSG	0x6
#define IMX9_SCMI_CORE_RESET_ADDR_SET_MSG_LEN	20
#define IMX9_SCMI_CORE_RESET_ADDR_SET_RESP_LEN	8

#define IMX9_SCMI_CORE_SETSLEEPMODE_MSG		0x7
#define IMX9_SCMI_CORE_SETSLEEPMODE_MSG_LEN	16
#define IMX9_SCMI_CORE_SETSLEEPMODE_RESP_LEN	8

#define IMX9_SCMI_CORE_SETIRQWAKESET_MSG	0x8
#define IMX9_SCMI_CORE_SETIRQWAKESET_MSG_LEN	76
#define IMX9_SCMI_CORE_SETIRQWAKESET_RESP_LEN	8

#define IMX9_SCMI_CORE_NONIRQWAKESET_MSG       0x9
#define IMX9_SCMI_CORE_NONIRQWAKESET_MSG_LEN   64
#define IMX9_SCMI_CORE_NONIRQWAKESET_RESP_LEN  8

#define IMX9_SCMI_CORE_LPMMODESET_MSG		0xA
#define IMX9_SCMI_CORE_LPMMODESET_MSG_LEN	12
#define IMX9_SCMI_CORE_LPMMODESET_RESP_LEN	8

#define IMX9_SCMI_PER_LPMMODESET_MSG		0xB
#define IMX9_SCMI_PER_LPMMODESET_MSG_LEN	12
#define IMX9_SCMI_PER_LPMMODESET_RESP_LEN	8

#define IMX9_SCMI_CORE_GETINFO_MSG		0xC
#define IMX9_SCMI_CORE_GETINFO_MSG_LEN		8
#define IMX9_SCMI_CORE_GETINFO_RESP_LEN		24

#define SCMI_CPU_VEC_FLAGS_BOOT			BIT(30)
#define SCMI_CPU_VEC_FLAGS_RESUME		BIT(31)

#define IMX9_SCMI_LMM_PROTO_ID			0x80

#define IMX9_SCMI_LMM_POWER_ON                  0xB
#define IMX9_SCMI_LMM_POWER_ON_MSG_LEN          0x8
#define IMX9_SCMI_LMM_POWER_ON_RESP_LEN         0x8

#define IMX9_SCMI_LMM_BOOT_MSG                  0x4
#define IMX9_SCMI_LMM_BOOT_MSG_LEN              0x8
#define IMX9_SCMI_LMM_BOOT_RESP_LEN             0x8

#define IMX9_SCMI_LMM_SHUTDOWN_MSG		0x6
#define IMX9_SCMI_LMM_SHUTDOWN_MSG_LEN		0xc
#define IMX9_SCMI_LMM_SHUTDOWN_RESP_LEN		0x8

#define IMX9_SCMI_LMM_SHUTDOWN_FLAG_FORCE	0
#define IMX9_SCMI_LMM_SHUTDOWN_FLAG_GRACEFUL	1

#define IMX9_SCMI_LMM_RESET_VECTOR              0xC
#define IMX9_SCMI_LMM_RESET_VECTOR_MSG_LEN      0x18
#define IMX9_SCMI_LMM_RESET_VECTOR_RESP_LEN     0x8

#define SCMI_GPC_WAKEUP 			0
#define SCMI_GIC_WAKEUP				1
#define SCMI_RESUME_CPU				BIT(1)

struct scmi_cpu_reset_addr_a2p {
	uint32_t cpu_id;
	uint32_t flags;
	uint32_t reset_vector_low;
	uint32_t reset_vector_high;
};

struct scmi_cpu_reset_addr_p2a {
	int32_t status;
};

struct scmi_cpu_start_a2p {
	uint32_t cpu_id;
};

struct scmi_cpu_start_p2a {
	int32_t status;
};

struct scmi_cpu_stop_a2p {
	uint32_t cpu_id;
};

struct scmi_cpu_stop_p2a {
	int32_t status;
};

struct scmi_lpm_config {
	uint32_t power_domain;
	uint32_t lpmsetting;
	uint32_t retentionmask;
};

struct scmi_cpu_pd_info {
	uint32_t cpu_id;
	uint32_t cpu_pd_id;
	uint32_t nmem;
	uint32_t *cpu_mem_pd_id;
};

/*
 * SCMI CPU peripheral LPM configuration
 */
struct scmi_per_lpm_config
 {
    uint32_t perId;
    uint32_t lpmSetting;
};

int scmi_core_set_reset_addr(void *p, uint64_t reset_addr, uint32_t cpu_id, uint32_t attr);
int scmi_core_start(void *p, uint32_t cpu_id);
int scmi_core_stop(void *p, uint32_t cpu_id);
int scmi_core_info_get(void *p, uint32_t cpu_id, uint32_t *run, uint32_t *sleep,
		       uint64_t *vector);
int scmi_core_set_sleep_mode(void *p, uint32_t cpu_id, uint32_t wakeup, uint32_t mode);
int scmi_core_Irq_wake_set(void *p, uint32_t cpu_id, uint32_t mask_idx,
			   uint32_t num_mask, uint32_t *mask);
int scmi_core_nonIrq_wake_set(void *p, uint32_t cpu_id, uint32_t mask_idx,
			uint32_t num_mask, uint32_t mask);
int scmi_core_lpm_mode_set(void *p, uint32_t cpu_id, uint32_t num_configs,
			   struct scmi_lpm_config *cfg);
int scmi_per_lpm_mode_set(void *p, uint32_t cpu_id, uint32_t num_configs,
			   struct scmi_per_lpm_config *cfg);
int scmi_perf_mode_set(void *p, uint32_t domain_id, uint32_t perf_level);
int scmi_lmm_protocol_attributes(void *p, uint32_t *num_lm);
int scmi_lmm_set_reset_vector(void *p, uint32_t lm_id, uint32_t cpuid, uint64_t addr);
int scmi_lmm_power_on(void *p, uint32_t lm_id);
int scmi_lmm_boot(void *p, uint32_t lm_id);
int scmi_lmm_shutdown(void *p, uint32_t lm_id, uint32_t flags);

#endif /* SCMI_IMX9_H */
