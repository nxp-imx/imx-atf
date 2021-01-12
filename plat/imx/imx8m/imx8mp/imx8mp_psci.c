/*
 * Copyright 2019-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <dram.h>
#include <gpc.h>
#include <imx8m_psci.h>
#include <plat_imx8.h>
#include <sema4.h>
#include "imx8_i2c.h"
#include "pmic.h"

#define SEMA4ID 0
#define CPUCNT  (IMX_SRC_BASE + LPA_STATUS)


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
extern struct dram_info dram_info;

#if 0
static uint32_t  clk_root_check_registers[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
                                               16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                               32, 33, 34, 38, 48, 49,
                                               64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 120, 121, 122, 123, 124, 125, 126, 130, 131, 132, 133, 134};
#endif



#if 1
static uint32_t syspll2_save, syspll3_save, syspll3div_save;

static save_ccgr_register ccgr_disabled_registers[103];
static uint8_t ccgr_reserved_registers[] = {
                                          //0, /* DVFS */
                                          //1, /* Anamix */
                                          //2, /* CPU */
                                          //3, /* CSU */
                                          //4, /* Debug */
                                          //5, /* DRAM1 */
                                          //6, /* Reserved */
                                          //11,/* GPIO 1*/
                                          //12,
                                          //13,
                                          //14,
                                          //15, /* GPIO 5*/
                                          //16, /* GPT1 */
                                          22, /* HS*/
                                          23, /* I2C1 */
                                          //25, /* I2C3 */
                                          //27, /* IOMUX */
                                          //28, /* IPMUX1 ? */
                                          //29, /* IPMUX2 ? */
                                          //30, /* IPMUX3 ? */
                                          //31, /* Reserved */
                                          //32, /* Reserved */
                                          33, /* MU */
                                          //34, /* OCOTP */
                                          //35, /* OCRAM */
                                          36, /* OCRAM_S */
                                          //38, /* PERFORM 1*/
                                          //39, /* PERFORM 2*/
                                          //44, /* QoS ? */
                                          //45, /* Reserved */
                                          //49, /* RDC */
                                          //55, /* Reserved */
                                          //56, /* Reserved */
                                          //57, /* SCTR*/
                                          //58, /* SDMA1 */
                                          //61, /* SEMA1 */
                                          //62, /* SEMA2 */
                                          //63, /* IRQ_STEER*/
                                          //64, /* sim ethnet */
                                          //65, /* sim m*/
                                          //66, /* sim main */
                                          //67, /* sim s*/
                                          //68, /* sim wakeup*/
                                          71, /* SNVS */
                                          //72, /* TRACE */
                                          //73, /* UART1 */
                                          //74, /* UART2 */
                                          //75, /* UART3 */
                                          //76, /* UART4 */
                                          //83, /* WDOG1*/
                                          //84, /* WDOG2*/
                                          //85, /* WDOG3*/
                                          //87,   /* GPU */
                                          //88, /* NOC */
                                          //92, /* HSIO*/
                                          //96, /* XTAL */
                                          97, /* PLL  */
                                          //100, /*MRPR*/
                                          101,/* AUDIO */
                                          //102,/* Reserved */
                                          

};
#endif


#if 1
static uint8_t  lpa_low_rate = 1;
/* except a53, m7, uart4 */
static save_register syspll1_clk_root_bus_to_24m_registers[] = {
                                                            //{1,0,0},
                                                            {6, 0x000,0},    /*AUDIO_AXI*/  /*done by M7 side, but changed by Kernel  */
                                                            //{7, 0},   /*HSIO */
                                                            {16,0x000,0},   /*MAIN_AXI*/
                                                            //{17,0},   /*ENET_AXI*/
                                                            //{18, 0},  /*NAND BUS */
                                                            //{21,0},   /*MEDIA_APB*/
                                                            //{22,0},   /*HDMI_APB*/
                                                            //{24,0},   /*GPU_AXI*/
                                                            //{25,0},   /*GPU_AHB*/
                                                            {26,0x000,0},   /*NOC*/  
                                                            {27,0x000,0},   /*NOC_IO*/     /*can't be touched! */
                                                            //{28,0},   /*ML_AXI*/
                                                            //{29,0},   /*ML_AHB*/
                                                            {32,0,0},   /*AHB_ROOT*/
                                                            {34,0,0},   /*AUDIO_AHB*/  /*done by M7 side, but changed by Kernel */
                                                            //{64,0},   /*DRAM_ALT*/   /*controlled by ddr dvfs */
                                                            //{65,0},   /*DRAM_APB*/   /*controlled by ddr dvfs */
                                                            //{86,0},   /*NAND*/       
                                                            //{87,0},   /*QSPI*/
                                                            //{88,0},   /*USDHc1*/
                                                            //{89,0},   /*USDHC2*/
                                                            //{90,0},   /*I2C1*/
                                                            //{91,0},   /*I2C2*/
                                                            //{92,0},   /*I2C3*/      /*controlled by M7 */
                                                            {94,0,0},   /*UART1*/
                                                            {95,0,0},   /*UART2*/
                                                            //{96,0},   /*UART3*/      /*debug*/
                                                            {100,0,0},   /*GIC*/
                                                            //{101,0},   /*ESPI1*/       
                                                            //{102,0},   /*ESPI2*/       
                                                            //{115,0},  /*WRCLK*/
                                                            //{121,0},  /*USDHC3*/
                                                            //{131,0},   /*ESPI3*/       
                                                            //{118,0}, 
                                                            //{124,0}
};

static save_register syspll1_clk_root_bus_to_audiopll1_registers[] = {
                                                            {6, 0x350,0},   /*AUDIO_AXI*/  /*done by M7 side, but changed by Kernel  */
                                                            {16,0x053,0},   /*MAIN_AXI*/
                                                            {26,0x053,0},   /*NOC*/  
                                                            {27,0x053,0},   /*NOC_IO*/     /*can't be touched! */
                                                            {32,0,0},   /*AHB_ROOT*/
                                                            {34,0,0},   /*AUDIO_AHB*/  /*done by M7 side, but changed by Kernel */
                                                            {94,0,0},   /*UART1*/
                                                            {95,0,0},   /*UART2*/
                                                            {100,0,0},  /*GIC*/
};

static save_register *syspll1_clk_root_bypass_registers;
static uint8_t        syspll1_clk_root_bypass_registers_count;
#endif

#if 1
static save_register syspll1_clk_root_disable_registers[] = {
                                                          {2,0},  /* ML */
                                                          {3,0},  /* GPU3d*/
                                                          {4,0},  /* GPU SHADER*/
                                                          {5,0},  /* GPU 2D*/
                                                          {7,0},   /*HSIO */
                                                          //8,    /*MEDIA ISP can't disable root clk not avaiable*/
                                                          {17,0},   /*ENET_AXI*/
                                                          {18,0},   /*NAND BUS */
                                                          //19,   /*VPU BUS  why can't disable root clk not avaiable?*/
                                                          //{20,0},   /*MEDIA AXI*/
                                                          //{21,0},   /*MEDIA_APB*/
                                                          {22,0},   /*HDMI_APB*/
                                                          {23,0},   /*HDMI AXI*/
                                                          //{24,0},   /*GPU_AXI*/
                                                          //{25,0},   /*GPU_AHB*/
                                                          {28,0},   /*ML_AXI*/
                                                          {29,0},   /*ML_AHB*/
                                                          //38, /*MEDIA DISP2 can't disable */
                                                          //{64,0},   /*DRAM_ALT*/   /*controlled by ddr dvfs */
                                                          //{65,0},   /*DRAM_APB*/   /*controlled by ddr dvfs */
                                                          //66, /VPU G1_CLK can't disable root clk not avaiable*/
                                                          //67, /VPU G2_CLK can't disable root clk not avaiable*/
                                                          {68,0},  /*CAN1 */
                                                          {69,0},  /*CAN2 */
                                                          {70,0},  /*MEMREPAIR_CLK */
                                                          {71,0},  /*PCIE_PHYCLK */
                                                          {72,0},  /*PCIE_AUX_CLK*/
                                                          {73,0},  /*I2C5*/
                                                          {74,0},  /*I2C6*/
                                                          {75,0},  /*SAI1*/
                                                          //{76,0},  /*SAI2*/
                                                          //{79,0},  /*SAI5*/
                                                          {80,0},  /*SAI6*/
                                                          {81,0},  /*ENETQOS*/
                                                          {82,0},  /*ENETQOS*/
                                                          {83,0},  /*ENEREF*/
                                                          {84,0},  /*ENETIMER*/
                                                          {85,0},  /*ENETPHY*/
                                                          {86,0},   /*NAND*/       
                                                          {87,0},   /*QSPI*/
                                                          {88,0},   /*USDHc1*/
                                                          {89,0},   /*USDHC2*/
                                                          //{90,0},   /*IC21*/
                                                          {91,0},   /*IC22*/
                                                          //{92,0},   /*IC23*/      /*controlled by M7 */
                                                          {93,0},   /*IC24*/
                                                          //94,   /*UART1  why can't disable */
                                                          //95,   /*UART2  why can't disable */
                                                          //96,   /*UART3*/      /*debug*/
                                                          {101,0},   /*ESPI1*/       
                                                          {102,0},   /*ESPI2*/       
                                                          {103,0},   /*PWM1*/
                                                          {104,0},   /*PWM2*/
                                                          {105,0},   /*PWM3*/
                                                          {106,0},   /*PWM4*/
                                                          //107,   /*GPT1 used by M7*/ 
                                                          {108,0},   /*GPT2*/
                                                          {109,0},   /*GPT3*/
                                                          {110,0},   /*GPT4*/
                                                          {111,0},   /*GPT5*/
                                                          {112,0},   /*GPT6*/
                                                          {113,0},   /*TRACE*/
                                                          {115,0},  /*WRCLK*/
                                                          {116,0},  /*IPP_DO1*/
                                                          {117,0},  /*IPP_DO2*/
                                                          {118,0},  /*HDMIFDCC*/
                                                          {119,0},  /* ? */
                                                          {120,0},  /*HDMIREF*/
                                                          {121,0},  /*USDHC3*/
                                                          {122,0},   /*MEDIA_CAM1*/
                                                          {125,0},   /*MEDIA_CAM2*/
                                                          {130,0},   /*MEDIA_MIPI_TEST*/
                                                          {131,0},   /*ESPI3*/       
                                                          //132,   /*PDM can't disable, root clk not avaiable*/
                                                          //133,   /*VPU_VC800E can't disable, root clk not avaiable*/
                                                          {134,0},  /* SAI7*/
                                                            //{124,0}
};
#endif
void bus_freq_dvfs(bool low_bus)
{
        //SA_I2C_Type i2cSave;
        syspll1_clk_root_bypass_registers = lpa_low_rate ? syspll1_clk_root_bus_to_24m_registers : syspll1_clk_root_bus_to_audiopll1_registers;
        syspll1_clk_root_bypass_registers_count =  lpa_low_rate ? ARRAY_SIZE(syspll1_clk_root_bus_to_24m_registers) : ARRAY_SIZE(syspll1_clk_root_bus_to_audiopll1_registers);

        if (low_bus) {

                //NOTICE("bus_freq_dvfs low bus  \n");

#if 1
                mmio_setbits_32(0x30390038, 1);
                mmio_setbits_32(0x3039003c, 1);
                mmio_setbits_32(0x30390040, 1);

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + GPU3D_PGC, 0x1);
		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, GPU3D_PWR_REQ);
		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & GPU3D_PWR_REQ);

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + GPU2D_PGC, 0x1);
		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, GPU2D_PWR_REQ);
		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & GPU2D_PWR_REQ);

		/* set the PGC bit */
		mmio_setbits_32(IMX_GPC_BASE + GPUMIX_PGC, 0x1);
		/* power down the domain */
		mmio_setbits_32(IMX_GPC_BASE + PU_PGC_DN_TRG, GPUMIX_PWR_REQ);
		/* wait for power request done */
		while (mmio_read_32(IMX_GPC_BASE + PU_PGC_DN_TRG) & GPUMIX_PWR_REQ);


                syspll2_save    = mmio_read_32(0x30360104);
                syspll3_save    = mmio_read_32(0x30360114);
                syspll3div_save = mmio_read_32(0x30360118);

                //NOTICE("syspll2_save:0x%x syspll3_save:0x%x \n",  syspll2_save,   syspll3_save);

                /* disable the DRAM AHB ROOT */
                //mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_CLR(1), (1 << 28));
#endif

                /* enable syspll3 firstly, otherwise can't bypass NOC_IO */
                if(!(mmio_read_32(0x30360114) & (0x80000000))){
                        mmio_write_32(0x30360118, 0x0012c032);
                        mmio_setbits_32(0x30360114, (0x1 << 9) | (0x1 << 11));
                        while(!(mmio_read_32(0x30360114) & (0x80000000)));
                        mmio_clrbits_32(0x30360114, (0x1 << 4));
                        NOTICE("enabled SYSPLL3 \n");
                }else
                        NOTICE("no needed enabled SYSPLL3 \n");


#if 1
                //NOTICE("bypass CCM \n");
                /* save clock root registers of clocks using system PLL1 and bypass these clocks */
                for (uint32_t cmpt_i = 0; cmpt_i <  syspll1_clk_root_bypass_registers_count; cmpt_i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_bypass_registers[cmpt_i].index);
                        /* save mux * pre divider */
                        syspll1_clk_root_bypass_registers[cmpt_i].value = (((mmio_read_32(_reg) >> 24) & 0x7 ) << 4) | 
                                                                          (((mmio_read_32(_reg) >> 16) & 0x7 ) << 0) | 
                                                                          (((mmio_read_32(_reg) >> 0 ) & 0x3F) << 8) ;

                        /* change divider */
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x00070000) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0x0F) >> 0) << 16));
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x0000003F) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0x3F00) >> 8) << 0));
                        /* change mux */
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x07000000) | (((syspll1_clk_root_bypass_registers[cmpt_i].p23 & 0xF0) >> 4) << 24));
                        //NOTICE("set clk root %d result: 0x%x \n", syspll1_clk_root_bypass_registers[cmpt_i].index, (mmio_read_32(_reg)));
                }
#endif

#if 1
                NOTICE("Disable CCM \n");
                /* disable syspll1 clock root registers */
                for (uint32_t i = 0; i < ARRAY_SIZE(syspll1_clk_root_disable_registers); i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_disable_registers[i].index);
                        //NOTICE("disable clk root %d\n", syspll1_clk_root_disable_registers[i].index);
                        syspll1_clk_root_disable_registers[i].value = (mmio_read_32(_reg) >> 28) & 0x1;
                        mmio_write_32(_reg, mmio_read_32(_reg) & ~0x10000000);
                }

#if 0
                for (uint32_t index = 0; index < ARRAY_SIZE(clk_root_check_registers); index++) {
                        uint32_t value;
                        value = mmio_read_32(0x30388000+( 128 * clk_root_check_registers[index]));
                        if((value & 0x10000000) >> 28 != 0){
                                NOTICE("index:%d:clock enable not zero!\n", clk_root_check_registers[index]);
                        }
                }
#endif

                NOTICE("Disable CCGR \r\n");
                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_reserved_registers); index++) {
                        ccgr_disabled_registers[ccgr_reserved_registers[index]].reserved = 1;
                }

                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_disabled_registers); index++) {
                        if(ccgr_disabled_registers[index].reserved != 1){
                                //NOTICE("disable CCGR:%d:0x%x \r\n", index, 0x30384000 + 16 * index);
                                ccgr_disabled_registers[index].value = mmio_read_32(0x30384000 + 16 * index) & 0xFF;
                                mmio_write_32(0x30384000 + 16 * index, 1);
                        }
                }
#endif

#if 0               
                saveI2C_Regs((SA_I2C_Type *) I2C1_BASE, &i2cSave);
                I2C_Init((SA_I2C_Type *) I2C1_BASE);
                VDD_SOC_LOW();
                restoreI2C_Regs((SA_I2C_Type *) I2C1_BASE, &i2cSave);
#endif

#if 1
                NOTICE("bypass ARM \n");
                //mmio_write_32(CCM_CORE_CLK_ROOT_GEN_TAGET_CLR(0), (0x7 << 24));
                /* set the a53 clk root 30388000 as 0x10000000, clk from 24M */
                mmio_write_32(0x30388000, 0x10000000);
                /* set the a53 clk change to a53 clk root from ARM PLL */
                mmio_write_32(0x30389880, 0x00000000);
                NOTICE("disable arm pll  \n");
                /* disable the ARM PLL, bypass first, then disable */
                mmio_setbits_32(0x30360084, (0x1 << 4));
                mmio_clrbits_32(0x30360084, (0x1 << 9));
#endif

#if 1

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

#endif


                NOTICE("bus_freq_dvfs low bus  done \n");
        }else{


                NOTICE("bus_freq_dvfs high bus  \n");


#if 1
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
                //mmio_write_32(0x30360104, syspll2_save);
                
                mmio_write_32(0x30360118, syspll3div_save);
                if(syspll3_save & (0x1 << 9)){
                        /* enable the SYSTEM PLL3, enable first, then unbypass */
                        mmio_setbits_32(0x30360114, (0x1 << 9));
                        while(!(mmio_read_32(0x30360114) & (0x80000000)));
                        mmio_clrbits_32(0x30360114, (0x1 << 4));
                        NOTICE("enabled SYSPLL3 \n");
                }
                //mmio_write_32(0x30360114, syspll3_save);
#endif

#if 1
                /* enable the ARM PLL, enable first, then unbypass */
                mmio_setbits_32(0x30360084, (0x1 << 9));
                while(!(mmio_read_32(0x30360084) & (0x80000000)));
                mmio_clrbits_32(0x30360084, (0x1 << 4));

                //mmio_write_32(CCM_CORE_CLK_ROOT_GEN_TAGET_SET(0), (0x1 << 24));
                /* set the a53 clk root 30388000 as 0x10000000, clk from syspll1 800M */
                mmio_write_32(0x30388000, 0x14000000);
                /* set the a53 clk change to ARM PLL from a53 clk root */
                mmio_write_32(0x30389880, 0x01000000);
                NOTICE("ARM changed to ARM PLL \n");
#endif

#if 0
                saveI2C_Regs((SA_I2C_Type *) I2C1_BASE, &i2cSave);
                I2C_Init((SA_I2C_Type *) I2C1_BASE);
                VDD_SOC_HIGH();
                restoreI2C_Regs((SA_I2C_Type *) I2C1_BASE, &i2cSave);

                for(volatile int i=0;i<50000;i++){} //200us
#endif
                
#if 1
                /* Enable CCRG */
                for (uint32_t index = 0; index < ARRAY_SIZE(ccgr_disabled_registers); index++) {
                        if(ccgr_disabled_registers[index].reserved != 1){
                                mmio_write_32(0x30384000 + 16 * index, ccgr_disabled_registers[index].value);
                        }
                }


                /* enable syspll1 clock root registers */
                for (uint32_t i = 0; i < ARRAY_SIZE(syspll1_clk_root_disable_registers); i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_disable_registers[i].index);
                        //NOTICE("disable clk root %d\n", syspll1_clk_root_disable_registers[i].index);
                        if(syspll1_clk_root_disable_registers[i].value)
                                mmio_setbits_32(_reg,  1 << 28);
                }

#endif


#if 1
                /* restore clock root registers of clocks using system PLL1 */
                for (uint32_t cmpt_i = 0; cmpt_i < syspll1_clk_root_bypass_registers_count; cmpt_i++) {
                        uint32_t _reg = 0x30388000 + (128 * syspll1_clk_root_bypass_registers[cmpt_i].index);
                        
                        /* restore mux */
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x07000000) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x70) >> 4) << 24));
                        /* restore divider */
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x00070000) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x07) >> 0) << 16));
                        mmio_write_32(_reg,  (mmio_read_32(_reg) & ~0x0000003F) | (((syspll1_clk_root_bypass_registers[cmpt_i].value & 0x3F00) >> 8) << 0));
                }

#endif

#if 1
                for(volatile int i=0;i<50000;i++){} //200us

                /* enable the DRAM AHB ROOT */
                //mmio_write_32(CCM_IP_CLK_ROOT_GEN_TAGET_SET(1), (1 << 28));


		/* clear the PGC bit */
		//mmio_clrbits_32(IMX_GPC_BASE + 0xd40, 0x1);
		/* power up the domain */
		//mmio_setbits_32(IMX_GPC_BASE + PU_PGC_UP_TRG, BIT(9));
		/* wait for power request done */
		//while (mmio_read_32(IMX_GPC_BASE + PU_PGC_UP_TRG) & BIT(9));

                mmio_clrbits_32(0x30390038, 1);
                mmio_clrbits_32(0x3039003c, 1);
                mmio_clrbits_32(0x30390040, 1);

                /* enable ddr clock */
                mmio_setbits_32(0x3038A000, (0x1 << 28));
                mmio_setbits_32(0x3038A080, (0x1 << 28));
#endif
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
                        uint8_t lpa_p2_lpddr4freq;
			/*
			 * when A53 don't enter DSM, only need to
			 * set the system wakeup option.
			 */
                        NOTICE("M7 alive, ddr also in retention \n");

                        refcount = mmio_read_32(CPUCNT) & 0xFF;
                        lpa_low_rate = !(mmio_read_32(CPUCNT) & 0xF0000);
                        refcount = refcount - 1;
                        mmio_clrsetbits_32(CPUCNT, 0xFF, refcount);

                        //mmio_write_32(CPUCNT, refcount);
                        //mmio_write_32(0x30384000+(16*22), 0x2); // Hs
			imx_set_sys_lpm(core_id, true);
                        lpa_p2_lpddr4freq = lpa_low_rate ? 2: 1;
                        lpddr4_swffc(&dram_info, dev_fsp, lpa_p2_lpddr4freq);
                        dev_fsp = (~dev_fsp) & 0x1;
			dram_enter_retention();
                        mmio_write_32(0x30384000 + (16 * 23), 0x2); /* 12C1 */
                        bus_freq_dvfs(true);
			//imx_anamix_override(true);
			//imx_noc_wrapper_pre_suspend(core_id);
			imx_set_sys_wakeup(core_id, true);
                        NOTICE("M7 alive, suspend ok! \n");
                        //mmio_write_32(0x30384000+(16*22), 0x0);
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
                        uint8_t lpa_p2_lpddr4freq;
                        bus_freq_dvfs(false);
			//imx_noc_wrapper_post_resume(core_id);
			//imx_anamix_override(false);
                        //NOTICE("dram_exit_retention_with_target 1 \n");
                        lpa_p2_lpddr4freq = lpa_low_rate ? 2: 1;
			dram_exit_retention_with_target(lpa_p2_lpddr4freq);
                        //NOTICE("dram_exit_retention_with_target 1 done \n");
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

static const plat_psci_ops_t imx_plat_psci_ops = {
	.pwr_domain_on = imx_pwr_domain_on,
	.pwr_domain_on_finish = imx_pwr_domain_on_finish,
	.pwr_domain_off = imx_pwr_domain_off,
	.validate_ns_entrypoint = imx_validate_ns_entrypoint,
	.validate_power_state = imx_validate_power_state,
	.cpu_standby = imx_cpu_standby,
	.pwr_domain_suspend = imx_domain_suspend,
	.pwr_domain_suspend_finish = imx_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = imx_pwr_domain_pwr_down_wfi,
	.get_sys_suspend_power_state = imx_get_sys_suspend_power_state,
	.system_reset = imx_system_reset,
	.system_off = imx_system_off,
};

/* export the platform specific psci ops */
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	/* sec_entrypoint is used for warm reset */
	imx_mailbox_init(sec_entrypoint);

	*psci_ops = &imx_plat_psci_ops;

	return 0;
}
