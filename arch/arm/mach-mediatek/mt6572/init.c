#include <config.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/arch/misc.h>
// #include <asm/io.h>
// #include <vsprintf.h>
// #include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* Might be required to call early when running as 1st stage BL */
/*
#define INFRACFG_BASE       0x10001000
#define APMIXED_BASE        0x10205000

#define INFRA_CKMUXSEL      (INFRACFG_BASE + 0x000)
#define ARMPLL_CON1         (APMIXED_BASE + 0x104)

int mtk_set_arm_clk(void)
{
	unsigned int reg_val;

	reg_val = readl(INFRA_CKMUXSEL);
	reg_val = 0;
	writel(INFRA_CKMUXSEL, reg_val);

  writel(ARMPLL_CON1, 0x8009A000);
  udelay(100);

  reg_val = readl(INFRA_CKMUXSEL);
  reg_val |= 0x4;
  writel(INFRA_CKMUXSEL, reg_val);

	return 0;
}

int board_early_init_f(void)
{
	puts("!!! mtk_set_arm_clk start !!!\n");
	mtk_set_arm_clk();
	puts("!!! mtk_set_arm_clk end !!!\n");
	return 0;
}
*/

int print_cpuinfo(void)
{
	printf("SoC:   MediaTek MT6572\n");
	return 0;
}