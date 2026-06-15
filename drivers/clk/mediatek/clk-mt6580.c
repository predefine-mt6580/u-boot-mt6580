// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <log.h>
#include <asm/arch-mediatek/reset.h>
#include <asm/io.h>
#include <dt-bindings/clock/mt6580-clk.h>
#include <linux/bitops.h>

#include "clk-mtk.h"

#define MT6580_PLL_FMAX			(1800UL * MHZ)
#define MT6580_CON0_RST_BAR		BIT(27)

static const ulong ext_clock_rates[] = {
	[CLK_XTAL] = 26 * MHZ,
};

#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	\
	    _pd_shift, _pcw_reg, _pcw_shift) {				\
		.id = _id,						\
		.reg = _reg,						\
		.pwr_reg = _pwr_reg,					\
		.en_mask = _en_mask,					\
		.rst_bar_mask = MT6580_CON0_RST_BAR,			\
		.fmax = MT6580_PLL_FMAX,				\
		.flags = _flags,					\
		.pcwbits = _pcwbits,					\
		.pd_reg = _pd_reg,					\
		.pd_shift = _pd_shift,					\
		.pcw_reg = _pcw_reg,					\
		.pcw_shift = _pcw_shift,				\
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x100, 0x110, 0x11, 0,
			21, 0x104, 24, 0x104, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x120, 0x130, 0x11, CLK_PLL_HAVE_RST_BAR,
			21, 0x124, 24, 0x124, 0),
	PLL(CLK_APMIXED_UNIVPLL, 0x140, 0x150, 0x30000011, CLK_PLL_HAVE_RST_BAR,
			7, 0x144, 24, 0x144, 0),
	PLL(CLK_APMIXED_MCUPLL, 0x220, 0x230, 0x11, 0,
			21, 0x224, 24, 0x224, 0),
	PLL(CLK_APMIXED_WHPLL, 0x240, 0x250, 0x11, 0,
			21, 0x244, 24, 0x244, 0),
	PLL(CLK_APMIXED_WPLL, 0x260, 0x270, 0x11, 0,
			10, 0x264, 24, 0x264, 0),
};

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK(CLK_TOP_13M, CLK_XTAL, CLK_PARENT_TOPCKGEN, 13 * MHZ),
};

#define FACTOR0(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define FACTOR1(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

static const struct mtk_fixed_factor top_fixed_divs[] = {
	FACTOR0(CLK_TOP_MPLL, CLK_APMIXED_MAINPLL, 1, 1),
	FACTOR1(CLK_TOP_MPLL_D6, CLK_TOP_MPLL, 1, 6),
	FACTOR1(CLK_TOP_MPLL_D7, CLK_TOP_MPLL, 1, 7),
	FACTOR1(CLK_TOP_MPLL_D8, CLK_TOP_MPLL, 1, 8),
	FACTOR1(CLK_TOP_MPLL_D10, CLK_TOP_MPLL, 1, 10),
	FACTOR1(CLK_TOP_MPLL_D12, CLK_TOP_MPLL, 1, 12),
	
	FACTOR0(CLK_TOP_UPLL, CLK_APMIXED_UNIVPLL, 1, 1),
	FACTOR1(CLK_TOP_UPLL_D6, CLK_TOP_UPLL, 1, 6),
	FACTOR1(CLK_TOP_UPLL_D7, CLK_TOP_UPLL, 1, 7),
	FACTOR1(CLK_TOP_UPLL_D12, CLK_TOP_UPLL, 1, 12),
	FACTOR1(CLK_TOP_UPLL_D24, CLK_TOP_UPLL, 1, 24),
};

static const struct mtk_parent  uart_parents[] = {
	EXT_PARENT(CLK_XTAL),
	TOP_PARENT(CLK_TOP_UPLL_D24)
};

static const struct mtk_parent  msdc_parents[] = {
	TOP_PARENT(CLK_TOP_MPLL_D12),
	TOP_PARENT(CLK_TOP_MPLL_D10),
	TOP_PARENT(CLK_TOP_MPLL_D8),
	TOP_PARENT(CLK_TOP_UPLL_D7),
	TOP_PARENT(CLK_TOP_MPLL_D7),
	TOP_PARENT(CLK_TOP_MPLL_D8),
	EXT_PARENT(CLK_XTAL),
	TOP_PARENT(CLK_TOP_UPLL_D6),
};

static const struct mtk_parent  mmsys_pwm_parents[] = {
	EXT_PARENT(CLK_XTAL),
	TOP_PARENT(CLK_TOP_UPLL_D12),
};

static const struct mtk_composite top_muxes[] = {
	MUX(CLK_TOP_UART0_SEL, uart_parents, 0x0, 0, 1),
	MUX(CLK_TOP_MSDC0_SEL, msdc_parents, 0x0, 11, 3),
	MUX(CLK_TOP_UART1_SEL, uart_parents, 0x0, 19, 1),
	MUX(CLK_TOP_MSDC1_SEL, msdc_parents, 0x0, 20, 3),
	MUX(CLK_TOP_MMSYS_PWM_SEL, mmsys_pwm_parents, 0x0, 18, 1),
	// TODO: add more muxes
};

static const struct mtk_gate_regs top0_cg_regs = {
	.sta_ofs = 0x20,
	.set_ofs = 0x50,
	.clr_ofs = 0x80,
};

static const struct mtk_gate_regs top1_cg_regs = {
	.sta_ofs = 0x24,
	.set_ofs = 0x54,
	.clr_ofs = 0x84,
};

#define GATE_TOPx_FLAGS(_id, _parent, _shift, _flags, _regs) {		\
		.id = _id,					\
		.parent = _parent,				\
		.regs = _regs,				\
		.shift = _shift,				\
		.flags = _flags,				\
	}

#define GATE_TOP0(_id, _parent, _shift) \
	GATE_TOPx_FLAGS(_id, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN, \
			&top0_cg_regs)

#define GATE_TOP1_FLAGS(_id, _parent, _shift, _flags) \
	GATE_TOPx_FLAGS(_id, _parent, _shift, _flags, &top1_cg_regs)

#define GATE_TOP1(_id, _parent, _shift) \
	GATE_TOP1_FLAGS(_id, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN)

#define GATE_TOP1_XTAL(_id, _parent, _shift) \
	GATE_TOP1_FLAGS(_id, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_EXT)

static const struct mtk_gate top_cgs[] = {
	GATE_TOP0(CLK_TOP_MMSYS_PWM, CLK_TOP_MMSYS_PWM_SEL, 0),

	GATE_TOP1_XTAL(CLK_TOP_I2C0, CLK_XTAL, 3),
	GATE_TOP1_XTAL(CLK_TOP_I2C1, CLK_XTAL, 4),
	GATE_TOP1(CLK_TOP_UART0, CLK_TOP_UART0_SEL, 10),
	GATE_TOP1(CLK_TOP_UART1, CLK_TOP_UART1_SEL, 11),
	GATE_TOP1_XTAL(CLK_TOP_I2C2, CLK_XTAL, 16),
	GATE_TOP1(CLK_TOP_MSDC0, CLK_TOP_MSDC0_SEL, 17),
	GATE_TOP1(CLK_TOP_MSDC1, CLK_TOP_MSDC1_SEL, 18),
	GATE_TOP1(CLK_TOP_APXGPT, CLK_TOP_13M, 24),
	// TODO: add more gates
};

static const struct mtk_gate_regs mmsys0_cg_regs = {
	.sta_ofs = 0x100,
	.set_ofs = 0x104,
	.clr_ofs = 0x108,
};

static const struct mtk_gate_regs mmsys1_cg_regs = {
	.sta_ofs = 0x110,
	.set_ofs = 0x114,
	.clr_ofs = 0x118,
};

#define GATE_MMSYS0_FLAGS(_id, _parent, _shift, _flags) {		\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &mmsys0_cg_regs,				\
		.shift = _shift,				\
		.flags = _flags,				\
	}

#define GATE_MMSYS0_XTAL(_id, _parent, _shift) \
	GATE_MMSYS0_FLAGS(_id, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_EXT)

#define GATE_MMSYS1_FLAGS(_id, _parent, _shift, _flags) {		\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &mmsys1_cg_regs,				\
		.shift = _shift,				\
		.flags = _flags,				\
	}

#define GATE_MMSYS1_XTAL(_id, _parent, _shift) \
	GATE_MMSYS1_FLAGS(_id, _parent, _shift, CLK_GATE_SETCLR | CLK_PARENT_EXT)

static const struct mtk_gate mmsys_cgs[] = {
  GATE_MMSYS0_XTAL(CLK_MMSYS_SMI_COMMON, CLK_XTAL, 0),
  GATE_MMSYS0_XTAL(CLK_MMSYS_SMI_LARB0, CLK_XTAL, 1),
  GATE_MMSYS0_XTAL(CLK_MMSYS_OVL0, CLK_XTAL, 10),
  GATE_MMSYS0_XTAL(CLK_MMSYS_RDMA0, CLK_XTAL, 11),

  GATE_MMSYS1_XTAL(CLK_MMSYS_PWM_MM, CLK_XTAL, 0),
  GATE_MMSYS1_XTAL(CLK_MMSYS_PWM_26M, CLK_XTAL, 1),
  GATE_MMSYS1_XTAL(CLK_MMSYS_DSI_ENGINE, CLK_XTAL, 2),
  GATE_MMSYS1_XTAL(CLK_MMSYS_DSI_DIGITAL, CLK_XTAL, 3),
};

static const struct mtk_clk_tree mt6580_clk_tree = {
	.pll_parent = EXT_PARENT(CLK_XTAL),
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.fdivs_offs = CLK_TOP_MPLL,
	.muxes_offs = CLK_TOP_UART0_SEL,
	.plls = apmixed_plls,
	.fclks = top_fixed_clks,
	.fdivs = top_fixed_divs,
	.muxes = top_muxes,
	.num_plls = ARRAY_SIZE(apmixed_plls),
	.num_fclks = ARRAY_SIZE(top_fixed_clks),
	.num_fdivs = ARRAY_SIZE(top_fixed_divs),
	.num_muxes = ARRAY_SIZE(top_muxes),
};

static int mt6580_apmixedsys_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6580_clk_tree);
}

static int mt6580_topckgen_probe(struct udevice *dev)
{
	return mtk_common_clk_init(dev, &mt6580_clk_tree);
}

static int mt6580_topckgen_cg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt6580_clk_tree, top_cgs, ARRAY_SIZE(top_cgs), 0);
}

static int mt6580_mmsys_cg_probe(struct udevice *dev)
{
	return mtk_common_clk_gate_init(dev, &mt6580_clk_tree, mmsys_cgs, ARRAY_SIZE(mmsys_cgs), 0);
}

static const struct udevice_id mt6580_apmixed_compat[] = {
	{ .compatible = "mediatek,mt6580-apmixedsys" },
	{ }
};

static const struct udevice_id mt6580_topckgen_compat[] = {
	{ .compatible = "mediatek,mt6580-topckgen" },
	{ }
};

static const struct udevice_id mt6580_topckgen_cg_compat[] = {
	{ .compatible = "mediatek,mt6580-topckgen-cg" },
	{ }
};

static const struct udevice_id mt6580_mmsys_cg_compat[] = {
	{ .compatible = "mediatek,mt6580-mmsys-cg" },
	{ }
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt6580-clock-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt6580_apmixed_compat,
	.probe = mt6580_apmixedsys_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt6580-clock-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt6580_topckgen_compat,
	.probe = mt6580_topckgen_probe,
	.priv_auto	= sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen_cg) = {
	.name = "mt6580-topckgen-cg",
	.id = UCLASS_CLK,
	.of_match = mt6580_topckgen_cg_compat,
	.probe = mt6580_topckgen_cg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_mmsys_cg) = {
	.name = "mt6580-mmsys-cg",
	.id = UCLASS_CLK,
	.of_match = mt6580_mmsys_cg_compat,
	.probe = mt6580_mmsys_cg_probe,
	.priv_auto	= sizeof(struct mtk_cg_priv),
	.ops = &mtk_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
