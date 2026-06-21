// SPDX-License-Identifier: GPL-2.0

#include <fdtdec.h>
#include <dm.h>
#include <ram.h>
#include <stdio.h>
#include <hang.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <clk.h>
#include <dt-bindings/clock/mt6580-clk.h>

DECLARE_GLOBAL_DATA_PTR;

#define DRV_Reg32(x)           (*(volatile unsigned int *)(x))
#define DRV_WriteReg32(x, y)    (*(volatile unsigned int *)(x) = (unsigned int)(y))


#define CKSYS_BASE (0x10000000)
#define CLK_MUX_SEL0                (CKSYS_BASE + 0x000)
#define CLK_MUX_SEL1                (CKSYS_BASE + 0x004)
#define CLK_GATING_CTRL0    	(CKSYS_BASE + 0x020)
#define CLK_GATING_CTRL1    	(CKSYS_BASE + 0x024)
#define INFRABUS_DCMCTL1            (CKSYS_BASE + 0x02C)
#define CLK_GATING_CTRL2    	(CKSYS_BASE + 0x03C)

#define INFRACFG_AO_BASE (0x10001000)
#define TOP_CKMUXSEL            (INFRACFG_AO_BASE + 0x000)
#define TOPAXI_PROT_EN          (INFRACFG_AO_BASE + 0x220)
#define TOPAXI_PROT_STA1        (INFRACFG_AO_BASE + 0x228)
#define PMIC_WRAP_SEL               (INFRACFG_AO_BASE + 0x520)
#define DISP_PROT_MASK       0x0802

#define MCUSYS_CONFIG_BASE   (0x10200000)
#define ACLKEN_DIV           (MCUSYS_CONFIG_BASE+0x060)
#define PCLKEN_DIV           (MCUSYS_CONFIG_BASE+0x064)

#define APMIXED_BASE (0x10018000)
#define AP_PLL_CON0          (APMIXED_BASE + 0x0000)
#define AP_PLL_CON1          (APMIXED_BASE + 0x0004)
#define AP_PLL_CON2          (APMIXED_BASE + 0x0008)
#define ARMPLL_CON1          (APMIXED_BASE + 0x0104)
#define ARMPLL_CON0          (APMIXED_BASE + 0x0100)
#define ARMPLL_PWR_CON0      (APMIXED_BASE + 0x0110)
#define MAINPLL_CON0         (APMIXED_BASE + 0x0120)
#define MAINPLL_CON1         (APMIXED_BASE + 0x0124)
#define MAINPLL_PWR_CON0     (APMIXED_BASE + 0x0130)
#define UNIVPLL_CON0         (APMIXED_BASE + 0x0140)
#define UNIVPLL_CON1         (APMIXED_BASE + 0x0144)
#define UNIVPLL_PWR_CON0     (APMIXED_BASE + 0x0150)

#define MFGCFG_BASE (0x13000000)
#define MFG_CG_CLR (MFGCFG_BASE + 0x8)

#define MMSYS_CONFIG_BASE (0x14000000)
#define MMSYS_CG_CON0 (MMSYS_CONFIG_BASE + 0x100)
#define MMSYS_CG_CON1 (MMSYS_CONFIG_BASE + 0x110)

#define AUDIO_BASE (0x11140000)
#define AUDIO_TOP_CON0 (AUDIO_BASE + 0x0)

#define SPM_PROJECT_CODE    0xb16
#define SPM_BASE (0x10006000)
#define SPM_POWERON_CONFIG_SET               (SPM_BASE + 0x000)
#define SPM_DIS_PWR_CON                      (SPM_BASE + 0x23c)
#define SPM_PWR_STATUS                      (SPM_BASE + 0x60c)
#define SPM_PWR_STATUS_2ND          (SPM_BASE + 0x610)

#define DIS_SRAM_ACK        (0x1 << 12)
#define PWR_CLK_DIS             (1U << 4)
#define PWR_ON_2ND              (1U << 3)
#define PWR_ON                  (1U << 2)
#define PWR_ISO                 (1U << 1)
#define PWR_RST_B               (1U << 0)

#define SRAM_PDN            (0xf << 8)

#define DIS_PWR_STA_MASK    (0x1 << 3)

void mt_pll_init(void)
{
  unsigned int temp;

  DRV_WriteReg32(AP_PLL_CON0, (DRV_Reg32(AP_PLL_CON0) | 0x1)); //[0] CLKSQ_EN = 1

  udelay(100);	//wait 100us
  DRV_WriteReg32(AP_PLL_CON0, (DRV_Reg32(AP_PLL_CON0) | 0x2)); //[1] CLKSQ_LPF_EN =1

  /*************
   * xPLL PWR ON
   **************/
  DRV_WriteReg32(ARMPLL_PWR_CON0, (DRV_Reg32(ARMPLL_PWR_CON0) | 0x1)); 	//[0]ARMPLL_PWR_ON = 1
  DRV_WriteReg32(MAINPLL_PWR_CON0, (DRV_Reg32(MAINPLL_PWR_CON0) | 0x1)); 	//[0]MAINPLL_PWR_ON = 1
  DRV_WriteReg32(UNIVPLL_PWR_CON0, (DRV_Reg32(UNIVPLL_PWR_CON0) | 0x1)); 	//[0]UNIVPLL_PWR_ON = 1

  /*************
   * Wait PWR ready(30ns)
   **************/
  udelay(30);

  /******************
   * xPLL ISO Disable
   *******************/
  DRV_WriteReg32(ARMPLL_PWR_CON0,	(DRV_Reg32(ARMPLL_PWR_CON0) & 0xFFFFFFFD));//[2]ARMPLL_ISO_EN = 0
  DRV_WriteReg32(MAINPLL_PWR_CON0, (DRV_Reg32(MAINPLL_PWR_CON0) & 0xFFFFFFFD)); //[2]MAINPLL_ISO_EN = 0
  DRV_WriteReg32(UNIVPLL_PWR_CON0, (DRV_Reg32(UNIVPLL_PWR_CON0) & 0xFFFFFFFD)); //[2]UNIVPLL_ISO_EN = 0

  /********************
   * xPLL Frequency Set
   *********************/
  DRV_WriteReg32(ARMPLL_CON1, 0x8009a000);  // 1000Mhz
  DRV_WriteReg32(MAINPLL_CON1, 0x800e7000);
  DRV_WriteReg32(UNIVPLL_CON1, 0x81000060);

  /***********************
   * xPLL Frequency Enable
   ************************/
  DRV_WriteReg32(ARMPLL_CON0, (DRV_Reg32(ARMPLL_CON0) | 0x1)); //[0] ARMPLL_EN = 1
  DRV_WriteReg32(MAINPLL_CON0, (DRV_Reg32(MAINPLL_CON0) | 0x1)); //[0]MAINPLL_EN = 1
  DRV_WriteReg32(UNIVPLL_CON0, (DRV_Reg32(UNIVPLL_CON0) | 0x1)); //[0] UNIVPLL_EN = 1

  /*************
   * Wait PWR ready(20ns)
   **************/
  udelay(20); // wait for PLL stable (min delay is 20us)


  /***************
   * xPLL DIV RSTB
   ****************/
  DRV_WriteReg32(MAINPLL_CON0, (DRV_Reg32(MAINPLL_CON0) | 0x08000000));//[27]MAINPLL_DIV_RSTB = 1
  DRV_WriteReg32(UNIVPLL_CON0, (DRV_Reg32(UNIVPLL_CON0) | 0x08000000));//]27]UNIVPLL_DIV_RSTB = 1

  /*****************
   * AXI BUS Init
   ******************/
  DRV_WriteReg32(INFRABUS_DCMCTL1, 0x80000000);
  DRV_WriteReg32(CLK_MUX_SEL0, 0x0B60b446);

  /**************
   * INFRA CLKMUX
   ***************/
  DRV_WriteReg32(ACLKEN_DIV, 0x12); // CPU BUS clock freq is divided by 2
  DRV_WriteReg32(PCLKEN_DIV, 0x15); // CPU debug clock freq is divided by 5

  temp = DRV_Reg32(TOP_CKMUXSEL) & ~0xC;
  DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x4); // switch CPU clock to ARMPLL

  /************
   * TOP CLKMUX
   *************/
  DRV_WriteReg32(CLK_MUX_SEL0, 0x27653446);
  DRV_WriteReg32(CLK_MUX_SEL1, 0x01291008);


  /*************/
  /*for MTCMOS
  **************/
  DRV_WriteReg32(SPM_POWERON_CONFIG_SET, (SPM_PROJECT_CODE << 16) | (1U << 0));
  DRV_WriteReg32(CLK_GATING_CTRL0, 0xBFCFFDFA);
  DRV_WriteReg32(CLK_GATING_CTRL1, (DRV_Reg32(CLK_GATING_CTRL1) | 0x800000));
  DRV_WriteReg32(CLK_GATING_CTRL2, (DRV_Reg32(CLK_GATING_CTRL2) | 0x10));

  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) | PWR_ON);
  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) | PWR_ON_2ND);

  while (!(DRV_Reg32(SPM_PWR_STATUS) & DIS_PWR_STA_MASK)
    || !(DRV_Reg32(SPM_PWR_STATUS_2ND) & DIS_PWR_STA_MASK)) {
    }

  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) & ~PWR_CLK_DIS);
  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) & ~PWR_ISO);
  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) | PWR_RST_B);

  DRV_WriteReg32(SPM_DIS_PWR_CON, DRV_Reg32(SPM_DIS_PWR_CON) & ~SRAM_PDN);

  int count = 0;
  while ((DRV_Reg32(SPM_DIS_PWR_CON) & DIS_SRAM_ACK)) {
    count++;
    if(count>1000)
      break;
  }

  DRV_WriteReg32(TOPAXI_PROT_EN, DRV_Reg32(TOPAXI_PROT_EN) & ~DISP_PROT_MASK);
  while (DRV_Reg32(TOPAXI_PROT_STA1) & DISP_PROT_MASK) {
  }


  /*************/
  /*for CG
  / *************/
  DRV_WriteReg32(MMSYS_CG_CON0,(DRV_Reg32(MMSYS_CG_CON0) & 0xfffffffC));
  DRV_WriteReg32(MMSYS_CG_CON1,0x0);
  DRV_WriteReg32(MFG_CG_CLR,0x1);
  DRV_WriteReg32(AUDIO_TOP_CON0,(DRV_Reg32(AUDIO_TOP_CON0) | 0x3000044));
  DRV_WriteReg32(PMIC_WRAP_SEL,(DRV_Reg32(PMIC_WRAP_SEL) & 0xfffffffd));

}

void mtk_pll_post_init(void)
{
  DRV_WriteReg32(AP_PLL_CON1, (DRV_Reg32(AP_PLL_CON1) & 0xFCFCEFCC)); // Main, ARM PLL HW Control
  DRV_WriteReg32(AP_PLL_CON2, (DRV_Reg32(AP_PLL_CON2) & 0xFFFFFFFC)); // Main, ARM PLL HW Control
}

int mtk_pll_early_init(void)
{
  struct udevice *dev;
  int ret;

  ret = uclass_get_device_by_driver(UCLASS_CLK,
                                    DM_DRIVER_GET(mtk_clk_apmixedsys), &dev);
  if (ret)
    return ret;

//   /* configure default rate then enable apmixedsys */
//   for (i = 0; i < ARRAY_SIZE(pll_rates); i++) {
//     struct clk clk = { .id = i, .dev = dev };
//
//     ret = clk_set_rate(&clk, pll_rates[i]);
//     if (ret)
//       return ret;
//
//     ret = clk_enable(&clk);
//     if (ret)
//       return ret;
//   }


  ret = uclass_get_device_by_driver(UCLASS_CLK,
                                    DM_DRIVER_GET(mtk_clk_topckgen), &dev);
  if (ret) {
    return ret;
  }

  ret = uclass_get_device_by_driver(UCLASS_CLK,
                                    DM_DRIVER_GET(mtk_clk_topckgen_cg), &dev);
  if (ret) {
    return ret;
  }

  return 0;
}

int dram_init(void)
{
  struct ram_info ram;
  struct udevice *dev;
  int ret;

  ret = uclass_first_device_err(UCLASS_RAM, &dev);
  if (ret)
    return ret;

  ret = ram_get_info(dev, &ram);
  if (ret)
    return ret;

  gd->ram_base = ram.base;
  gd->ram_size = ram.size;
  printf("RAM init base = 0x%lx, size = 0x%zx\n", gd->ram_base, ram.size);

  return 0;
}


int print_cpuinfo(void)
{
  printf("CPU:   MediaTek MT6580\n");

  return 0;
}

#ifdef CONFIG_SPL_BUILD

int mtk_soc_early_init(void)
{
  int ret;

  mt_pll_init();

  ret = mtk_pll_early_init();
  if (ret)
    return ret;

  ret = dram_init();
  if (ret)
    return ret;

  mtk_pll_post_init();
  return 0;
}

void reset_cpu(void)
{
  // writel(WDOG_RESTART_KEY, WDOG_RESTART);
  // writel(WDOG_SWRST_KEY, WDOG_SWRST);
#warning "reset_cpu unimplemented!!!!"
  printf("TODO: reset_cpu\n");
  hang();
}
#endif
