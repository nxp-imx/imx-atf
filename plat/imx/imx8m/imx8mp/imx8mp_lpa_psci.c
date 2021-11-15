/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/psci/psci.h>

#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>
#include <lib/mmio.h>
#include <sema4.h>
#include <dram.h>

#define SEMA4ID 0
#define CPUCNT  (IMX_SRC_BASE + LPA_STATUS)

#define SRC_DDR1_RCR		(IMX_SRC_BASE + 0x1000)
#define CCM_SRC_CTRL_OFFSET     (IMX_CCM_BASE + 0x800)
#define CCM_CCGR_OFFSET         (IMX_CCM_BASE + 0x4000)
#define CCM_SRC_CTRL(n)		(CCM_SRC_CTRL_OFFSET + 0x10 * (n))
#define CCM_CCGR(n)		(CCM_CCGR_OFFSET + 0x10 * (n))

typedef struct {
        uint8_t index;
        uint16_t p23;
        uint16_t value;
} save_register;

typedef struct {
        uint8_t value;
        uint8_t reserved;
} save_ccgr_register;

extern unsigned int dev_fsp;
void rank_setting_update(void);
bool imx_is_m4_enabled(void);

static uint32_t syspll2_save, syspll3_save, syspll3div_save;
static uint8_t  apll1enabled, apll2enabled;

static save_ccgr_register ccgr_disabled_registers[103];
static uint8_t ccgr_reserved_registers[] = {
                                          22, /* HS*/
                                          23, /* I2C1 */
                                          33, /* MU */
                                          36, /* OCRAM_S */
                                          71, /* SNVS */
                                          97, /* PLL  */
                                          101,/* AUDIO */
};

static save_register syspll1_clk_root_bus_to_24m_registers[] = {
                                                            {6, 0x000,0},   /*AUDIO_AXI*/  /*done by M7 side, but changed by Kernel  */
                                                            {16,0x000,0},   /*MAIN_AXI*/
                                                            {26,0x000,0},   /*NOC*/
                                                            {27,0x000,0},   /*NOC_IO*/     /*can't be touched! */
                                                            {32,0,0},       /*AHB_ROOT*/
                                                            {34,0,0},       /*AUDIO_AHB*/  /*done by M7 side, but changed by Kernel */
                                                            {94,0,0},       /*UART1*/
                                                            {100,0,0},      /*GIC*/
};

static save_register *syspll1_clk_root_bypass_registers;
static uint8_t        syspll1_clk_root_bypass_registers_count;

static save_register syspll1_clk_root_disable_registers[] = {
                                                          {2,0},   /*ML */
                                                          {3,0},   /*GPU3d*/
                                                          {4,0},   /*GPU SHADER*/
                                                          {5,0},   /*GPU 2D*/
                                                          {7,0},   /*HSIO */
                                                          {17,0},  /*ENET_AXI*/
                                                          {18,0},  /*NAND BUS */
                                                          {22,0},  /*HDMI_APB*/
                                                          {23,0},  /*HDMI AXI*/
                                                          {28,0},  /*ML_AXI*/
                                                          {29,0},  /*ML_AHB*/
                                                          {68,0},  /*CAN1 */
                                                          {69,0},  /*CAN2 */
                                                          {70,0},  /*MEMREPAIR_CLK */
                                                          {71,0},  /*PCIE_PHYCLK */
                                                          {72,0},  /*PCIE_AUX_CLK*/
                                                          {73,0},  /*I2C5*/
                                                          {74,0},  /*I2C6*/
                                                          {75,0},  /*SAI1*/
                                                          {80,0},  /*SAI6*/
                                                          {81,0},  /*ENETQOS*/
                                                          {82,0},  /*ENETQOS*/
                                                          {83,0},  /*ENEREF*/
                                                          {84,0},  /*ENETIMER*/
                                                          {85,0},  /*ENETPHY*/
                                                          {86,0},  /*NAND*/
                                                          {87,0},  /*QSPI*/
                                                          {88,0},  /*USDHc1*/
                                                          {89,0},  /*USDHC2*/
                                                          {91,0},  /*I2C2*/
                                                          {93,0},  /*I2C4*/
                                                          {101,0}, /*ESPI1*/
                                                          {102,0}, /*ESPI2*/
                                                          {103,0}, /*PWM1*/
                                                          {104,0}, /*PWM2*/
                                                          {105,0}, /*PWM3*/
                                                          {106,0}, /*PWM4*/
                                                          {108,0}, /*GPT2*/
                                                          {109,0}, /*GPT3*/
                                                          {110,0}, /*GPT4*/
                                                          {111,0}, /*GPT5*/
                                                          {112,0}, /*GPT6*/
                                                          {113,0}, /*TRACE*/
                                                          {115,0}, /*WRCLK*/
                                                          {116,0}, /*IPP_DO1*/
                                                          {117,0}, /*IPP_DO2*/
                                                          {118,0}, /*HDMIFDCC*/
                                                          {119,0}, /* ? */
                                                          {120,0}, /*HDMIREF*/
                                                          {121,0}, /*USDHC3*/
                                                          {122,0}, /*MEDIA_CAM1*/
                                                          {125,0}, /*MEDIA_CAM2*/
                                                          {130,0}, /*MEDIA_MIPI_TEST*/
                                                          {131,0}, /*ESPI3*/
                                                          {134,0}, /* SAI7*/
};

static void dram_exit_retention_with_target(uint32_t target)
{
	VERBOSE("dram exit retention\n");
	/* assert all reset */
#if defined(PLAT_imx8mq)
	mmio_write_32(SRC_DDR2_RCR, 0x8F000003);
	mmio_write_32(SRC_DDR1_RCR, 0x8F00000F);
	mmio_write_32(SRC_DDR2_RCR, 0x8F000000);
#else
	mmio_write_32(SRC_DDR1_RCR, 0x8F00001F);
	mmio_write_32(SRC_DDR1_RCR, 0x8F00000F);
#endif
	mmio_write_32(CCM_CCGR(5), 2);
	mmio_write_32(CCM_SRC_CTRL(15), 2);


        switch(target){
                case 0:
                        /* change the clock source of dram_apb_clk_root */
                        mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
                        mmio_write_32(0x3038a084, (0x4 << 24) | (0x3 << 16));
                        break;
                case 1:
                        /* change the clock source of dram_alt_clk_root to source 1 --400MHz */
                        mmio_write_32(0x3038a008, (0x7 << 24) | (0x7 << 16));
                        mmio_write_32(0x3038a004, (0x1 << 24) | (0x1 << 16));

                        /* change the clock source of dram_apb_clk_root to source 3 --160MHz/2 */
                        mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
                        mmio_write_32(0x3038a084, (0x3 << 24) | (0x1 << 16));
                        break;
                case 2:
                        /* change the clock source of dram_alt_clk_root to source 2 --100MHz */
                        mmio_write_32(0x3038a008, (0x7 << 24) | (0x7 << 16));
                        mmio_write_32(0x3038a004, (0x2 << 24));

                        /* change the clock source of dram_apb_clk_root to source 2 --40MHz/2 */
                        mmio_write_32(0x3038a088, (0x7 << 24) | (0x7 << 16));
                        mmio_write_32(0x3038a084, (0x2 << 24) | (0x1 << 16));
                        break;

        }

	mmio_write_32(SRC_DDR1_RCR, 0x8F000006);


        if (imx_is_m4_enabled() && imx_m4_lpa_active()) {
                /* enable the DRAM PLL */
                mmio_setbits_32(0x30360050, 1 << 9);
        }
	/* wait dram pll locked */
	while(!(mmio_read_32(DRAM_PLL_CTRL) & BIT(31)))
		;

	/* ddrc re-init */
	dram_umctl2_init(dram_info.timing_info);

	/*
	 * Skips the DRAM init routine and starts up in selfrefresh mode
	 * Program INIT0.skip_dram_init = 2'b11
	 */
	mmio_setbits_32(DDRC_INIT0(0), 0xc0000000);

        mmio_write_32(DDRC_MSTR2(0), target);
	/* Keeps the controller in self-refresh mode */
	mmio_write_32(DDRC_PWRCTL(0), 0xaa);
	mmio_write_32(DDRC_DBG1(0), 0x0);
	mmio_write_32(SRC_DDR1_RCR, 0x8F000004);
	mmio_write_32(SRC_DDR1_RCR, 0x8F000000);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0);

#if !PLAT_imx8mn
	if (dram_info.dram_type == DDRC_LPDDR4)
		mmio_write_32(DDRC_DDR_SS_GPR0, 0x01); /*LPDDR4 mode */
#endif /* !PLAT_imx8mn */

	mmio_write_32(DDRC_DFIMISC(0), 0x0 | (target << 8));

	/* dram phy re-init */
	dram_phy_init(dram_info.timing_info);

	/* workaround for rank-to-rank issue */
	rank_setting_update();

	/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	while (dwc_ddrphy_apb_rd(0x20097))
		;
	dwc_ddrphy_apb_wr(0xd0000, 0x1);

	/* before write Dynamic reg, sw_done should be 0 */
	mmio_write_32(DDRC_SWCTL(0), 0x0);


	mmio_write_32(DDRC_DFIMISC(0), 0x20 | (target << 8));
	/* wait DFISTAT.dfi_init_complete to 1 */
	while (!(mmio_read_32(DDRC_DFISTAT(0)) & 0x1))
		;

	/* clear DFIMISC.dfi_init_start */
	mmio_write_32(DDRC_DFIMISC(0), 0x0 | (target << 8));
	/* set DFIMISC.dfi_init_complete_en */
	mmio_write_32(DDRC_DFIMISC(0), 0x1 | (target << 8));

	/* set SWCTL.sw_done to enable quasi-dynamic register programming */
	mmio_write_32(DDRC_SWCTL(0), 0x1);
	/* wait SWSTAT.sw_done_ack to 1 */
	while (!(mmio_read_32(DDRC_SWSTAT(0)) & 0x1))
		;

	mmio_write_32(DDRC_PWRCTL(0), 0x88);
	/* wait STAT to normal state */
	while (0x1 != (mmio_read_32(DDRC_STAT(0)) & 0x7))
		;

	mmio_write_32(DDRC_PCTRL_0(0), 0x1);
	 /* dis_auto-refresh is set to 0 */
	mmio_write_32(DDRC_RFSHCTL3(0), 0x0);

	/* should check PhyInLP3 pub reg */
	dwc_ddrphy_apb_wr(0xd0000, 0x0);
	if (!(dwc_ddrphy_apb_rd(0x90028) & 0x1))
		VERBOSE("PHYInLP3 = 0\n");
	dwc_ddrphy_apb_wr(0xd0000, 0x1);
}

static void bus_freq_dvfs(bool low_bus)
{
        syspll1_clk_root_bypass_registers       =  syspll1_clk_root_bus_to_24m_registers;
        syspll1_clk_root_bypass_registers_count =  ARRAY_SIZE(syspll1_clk_root_bus_to_24m_registers);

        if (low_bus) {

                NOTICE("bus_freq_dvfs low bus  \n");

                syspll2_save    = mmio_read_32(0x30360104);
                syspll3_save    = mmio_read_32(0x30360114);
                syspll3div_save = mmio_read_32(0x30360118);

                /* enable syspll3 firstly, otherwise can't bypass NOC_IO */
                if(!(mmio_read_32(0x30360114) & (0x80000000))){
                        mmio_write_32(0x30360118, 0x0012c032);
                        mmio_setbits_32(0x30360114, (0x1 << 9) | (0x1 << 11));
                        while(!(mmio_read_32(0x30360114) & (0x80000000)));
                        mmio_clrbits_32(0x30360114, (0x1 << 4));
                        NOTICE("enabled SYSPLL3 \n");
                }else
                        NOTICE("no needed enabled SYSPLL3 \n");


                /* save clock root registers of clocks using system PLL1 and bypass these clocks */
                NOTICE("bypass CCM \n");
                for (uint32_t cmpt_i = 0; cmpt_i <  syspll1_clk_root_bypass_registers_count; cmpt_i++) {
                        uint32_t _reg   = 0x30388000 + (128 * syspll1_clk_root_bypass_registers[cmpt_i].index);
                        uint32_t _regv  = mmio_read_32(_reg);
                        /* save mux * pre divider */
                        syspll1_clk_root_bypass_registers[cmpt_i].value = (((_regv >> 24) & 0x7 ) << 4) |
                                                                          (((_regv >> 16) & 0x7 ) << 0) |
                                                                          (((_regv >> 0 ) & 0x3F) << 8) ;

                        _regv = (_regv & ~0x00070000) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0x0F) >> 0) << 16);
                        _regv = (_regv & ~0x0000003F) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0x3F00) >> 8) << 0);
                        /* change divider */
                        mmio_write_32(_reg,  _regv);
                        /* change mux twice to avoid ccm core/bus switch source issue */
                        _regv = (_regv & ~0x07000000) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0xF0) >> 4) << 24);
                        mmio_write_32(_reg,  _regv);
                        mmio_write_32(_reg,  _regv);
                }

                /* disable syspll1 clock root registers */
                NOTICE("Disable CCM \n");
                for (uint32_t i = 0; i < ARRAY_SIZE(syspll1_clk_root_disable_registers); i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_disable_registers[i].index);
                        uint32_t _regv  = mmio_read_32(_reg);
                        syspll1_clk_root_disable_registers[i].value = (_regv >> 28) & 0x1;
                        mmio_write_32(_reg, _regv & ~0x10000000);
                }

                NOTICE("Disable CCGR \r\n");
                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_reserved_registers); index++) {
                        ccgr_disabled_registers[ccgr_reserved_registers[index]].reserved = 1;
                }

                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_disabled_registers); index++) {
                        if(ccgr_disabled_registers[index].reserved != 1){
                                ccgr_disabled_registers[index].value = mmio_read_32(0x30384000 + 16 * index) & 0xFF;
                                mmio_write_32(0x30384000 + 16 * index, 1);
                        }
                }

                NOTICE("bypass ARM \n");
                /* set the a53 clk root 30388000 as 0x10000000, clk from 24M */
                mmio_write_32(0x30388000, 0x10000000);
                /* set the a53 clk change to a53 clk root from ARM PLL */
                mmio_write_32(0x30389880, 0x00000000);
                NOTICE("disable arm pll  \n");
                /* disable the ARM PLL, bypass first, then disable */
                mmio_setbits_32(0x30360084, (0x1 << 4));
                mmio_clrbits_32(0x30360084, (0x1 << 9));

                apll1enabled =  !!(mmio_read_32(0x30360000) & 0x80000000);
                apll2enabled =  !!(mmio_read_32(0x30360014) & 0x80000000);

                if(apll1enabled){
                        mmio_setbits_32(0x30360000, (0x1 << 4));
                        mmio_clrbits_32(0x30360000, (0x1 << 9));
                }
                if(apll2enabled){
                        mmio_setbits_32(0x30360014, (0x1 << 4));
                        mmio_clrbits_32(0x30360014, (0x1 << 9));
                }


                NOTICE("disable sys pll 2  \n");
                /* disable the SYSTEM PLL2, bypass first, then disable */
                mmio_setbits_32(0x30360104, (0x1 << 4));
                mmio_clrbits_32(0x30360104, (0x1 << 9));

                NOTICE("disable sys pll 3  \n");
                /* disable the SYSTEM PLL3, bypass first, then disable */
                mmio_setbits_32(0x30360114, (0x1 << 4));
                mmio_clrbits_32(0x30360114, (0x1 << 9));
                NOTICE("disable sys pll 3 done \n");

                /* disable the SYSTEM PLL1, bypass first, then disable */
                NOTICE("bypass syspll1  \n");
                mmio_setbits_32(0x30360094, (0x1 << 4));
                NOTICE("disable syspll1  \n");
                mmio_clrbits_32(0x30360094, (0x1 << 9));
                NOTICE("bus_freq_dvfs low bus  done \n");
        }else{

                NOTICE("bus_freq_dvfs high bus  \n");

                /* enable the SYSTEM PLL1, enable first, then unbypass */
                mmio_setbits_32(0x30360094, (0x1 << 9));
                while(!(mmio_read_32(0x30360094) & (0x80000000)));
                NOTICE("unbypass syspll1  \n");
                mmio_clrbits_32(0x30360094, (0x1 << 4));

                if(syspll2_save & (0x1 << 9)){
                        /* enable the SYSTEM PLL2, enable first, then unbypass */
                        mmio_setbits_32(0x30360104, (0x1 << 9));
                        while(!(mmio_read_32(0x30360104) & (0x80000000)));
                        mmio_clrbits_32(0x30360104, (0x1 << 4));
                        NOTICE("enabled SYSPLL2 \n");
                }

                /* Need restore SYSPLL3 DIV ? */
                mmio_write_32(0x30360118, syspll3div_save);
                if(syspll3_save & (0x1 << 9)){
                        /* enable the SYSTEM PLL3, enable first, then unbypass */
                        mmio_setbits_32(0x30360114, (0x1 << 9));
                        while(!(mmio_read_32(0x30360114) & (0x80000000)));
                        mmio_clrbits_32(0x30360114, (0x1 << 4));
                        NOTICE("enabled SYSPLL3 \n");
                }

                if(apll1enabled){
                        mmio_setbits_32(0x30360000, (0x1 << 9));
                        while(!(mmio_read_32(0x30360000) & ( (uint32_t) 1 << 31)));
                        mmio_clrbits_32(0x30360000, (0x1 << 4));
                }

                if(apll2enabled){
                        mmio_setbits_32(0x30360014, (0x1 << 9));
                        while(!(mmio_read_32(0x30360014) & ( (uint32_t) 1 << 31)));
                        mmio_clrbits_32(0x30360014, (0x1 << 4));
                }


                /* enable the ARM PLL, enable first, then unbypass */
                mmio_setbits_32(0x30360084, (0x1 << 9));
                while(!(mmio_read_32(0x30360084) & (0x80000000)));
                mmio_clrbits_32(0x30360084, (0x1 << 4));

                /* set the a53 clk root 30388000 as 0x10000000, clk from syspll1 800M */
                mmio_write_32(0x30388000, 0x14000000);
                /* set the a53 clk change to ARM PLL from a53 clk root */
                mmio_write_32(0x30389880, 0x01000000);
                NOTICE("ARM changed to ARM PLL \n");

                /* Enable CCRG */
                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_disabled_registers); index++) {
                        if(ccgr_disabled_registers[index].reserved != 1){
                                mmio_write_32(0x30384000 + 16 * index, ccgr_disabled_registers[index].value);
                        }
                }

                /* enable syspll1 clock root registers */
                for (uint32_t i = 0; i < ARRAY_SIZE(syspll1_clk_root_disable_registers); i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_disable_registers[i].index);
                        if(syspll1_clk_root_disable_registers[i].value)
                                mmio_setbits_32(_reg,  1 << 28);
                }


                /* restore clock root registers of clocks using system PLL1 */
                for (uint32_t cmpt_i = 0; cmpt_i < syspll1_clk_root_bypass_registers_count; cmpt_i++) {
                        uint32_t _reg  = 0x30388000 + (128 * syspll1_clk_root_bypass_registers[cmpt_i].index);
                        uint32_t _regv = mmio_read_32(_reg);

                        _regv = ( _regv & ~0x07000000) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x70) >> 4) << 24);

                        /* restore mux */
                        mmio_write_32(_reg,  _regv);
                        /* restore divider */
                        _regv = (_regv & ~0x00070000) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x07) >> 0) << 16);
                        _regv = (_regv & ~0x0000003F) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x3F00) >> 8) << 0);
                        mmio_write_32(_reg,  _regv);
                }

                /* enable ddr clock */
                mmio_setbits_32(0x3038A000, (0x1 << 28));
                mmio_setbits_32(0x3038A080, (0x1 << 28));



        }
}

void imx_domain_suspend(const psci_power_state_t *target_state)
{
	uint64_t base_addr = BL31_BASE;
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		plat_gic_cpuif_disable();
		imx_set_cpu_secure_entry(core_id, base_addr);
		imx_set_cpu_lpm(core_id, true);
	} else {
		dsb();
		write_scr_el3(read_scr_el3() | SCR_FIQ_BIT);
		isb();
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state)))
		imx_set_cluster_powerdown(core_id, CLUSTER_PWR_STATE(target_state));

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
                sema4_lock(SEMA4ID);
		if (!imx_m4_lpa_active()) {
                        NOTICE("M7 not alive, ddr in retention \n");
			imx_set_sys_lpm(core_id, true);
			dram_enter_retention();
			imx_anamix_override(true);
			imx_noc_wrapper_pre_suspend(core_id);
		} else {
                        uint32_t refcount;
			/*
			 * when A53 don't enter DSM, only need to
			 * set the system wakeup option.
			 */
                        NOTICE("M7 alive, ddr also in retention \n");
                        refcount = mmio_read_32(CPUCNT) & 0xFF;
                        refcount = refcount - 1;
                        mmio_clrsetbits_32(CPUCNT, 0xFF, refcount);
			imx_set_sys_lpm(core_id, true);
                        lpddr4_swffc(&dram_info, dev_fsp, 1);
                        dev_fsp = (~dev_fsp) & 0x1;
			dram_enter_retention();
                        bus_freq_dvfs(true);
			imx_set_sys_wakeup(core_id, true);
                        NOTICE("M7 alive, suspend ok! \n");
		}
                sema4_unlock(SEMA4ID);
	}
}

void imx_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint64_t mpidr = read_mpidr_el1();
	unsigned int core_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (is_local_state_off(SYSTEM_PWR_STATE(target_state))) {
                sema4_lock(SEMA4ID);
		if (!imx_m4_lpa_active()) {
			imx_noc_wrapper_post_resume(core_id);
			imx_anamix_override(false);
			dram_exit_retention();
			imx_set_sys_lpm(core_id, false);
		} else {
                        uint32_t refcount;
                        bus_freq_dvfs(false);
			dram_exit_retention_with_target(1);
                        lpddr4_swffc(&dram_info, dev_fsp, 0);
                        dev_fsp = (~dev_fsp) & 0x1;
                        NOTICE("restore freq  0 done \n");
			imx_set_sys_lpm(core_id, false);
			imx_set_sys_wakeup(core_id, false);

                        refcount = mmio_read_32(CPUCNT) & 0xFF;
                        refcount ++;
                        mmio_clrsetbits_32(CPUCNT, 0xFF, refcount);

		}
                sema4_unlock(SEMA4ID);
	}

	if (!is_local_state_run(CLUSTER_PWR_STATE(target_state))) {
		imx_clear_rbc_count();
		imx_set_cluster_powerdown(core_id, PSCI_LOCAL_STATE_RUN);
	}

	if (is_local_state_off(CORE_PWR_STATE(target_state))) {
		/* mark this core as awake by masking IRQ0 */
		imx_set_cpu_lpm(core_id, false);
		plat_gic_cpuif_enable();
	} else {
		write_scr_el3(read_scr_el3() & (~SCR_FIQ_BIT));
		isb();
	}
}
