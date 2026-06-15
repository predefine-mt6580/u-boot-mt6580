// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Pavel Ivanov <predefine@yandex.ru>
 */

#include <dm.h>
#include <generic-phy.h>
#include <phy-mipi-dphy.h>
#include <linux/delay.h>
#include <asm/io.h>

#define MIPITX_DSI_CON 0x00
#define MIPITX_DSI_CON_CKG_LDOOUT_EN BIT(1)
#define MIPITX_DSI_CON_LDOOUT_EN BIT(0)

#define MIPITX_DSI_CLOCK_LANE 0x04
#define MIPITX_DSI_LANE0 0x08
#define MIPITX_DSI_LANE1 0x0C
#define MIPITX_DSI_LANE2 0x10
#define MIPITX_DSI_LANE3 0x14
#define MIPITX_DSI_CLOCK_LANE_LDOOUT_EN BIT(0)

#define MIPITX_DSI_TOP_CON 0x40
#define MIPITX_DSI_TOP_CON_PAD_TIE_LOW_EN BIT(11)
#define MIPITX_DSI_TOP_CON_LNT_HS_BIAS_EN BIT(1)

#define MIPITX_DSI_BG_CON 0x44
#define MIPITX_DSI_BG_CON_CKEN BIT(1)
#define MIPITX_DSI_BG_CON_CORE_EN BIT(0)

#define MIPITX_DSI_PLL_CON0 0x50
#define MIPITX_DSI_PLL_CON0_TXDIV1_MASK 3
#define MIPITX_DSI_PLL_CON0_TXDIV1_SHIFT 5
#define MIPITX_DSI_PLL_CON0_TXDIV0_MASK 3
#define MIPITX_DSI_PLL_CON0_TXDIV0_SHIFT 3
#define MIPITX_DSI_PLL_CON0_PREDIV_MASK 3
#define MIPITX_DSI_PLL_CON0_PREDIV_SHIFT 1
#define MIPITX_DSI_PLL_CON0_PLL_EN BIT(0)

#define MIPITX_DSI_PLL_CON1 0x54
#define MIPITX_DSI_PLL_CON1_SDM_SSC_EN BIT(2)
#define MIPITX_DSI_PLL_CON1_SDM_FRA_EN BIT(0)

#define MIPITX_DSI_PLL_CON2 0x58
#define MIPITX_DSI_PLL_CON2_PCW_H_MASK 0x7f
#define MIPITX_DSI_PLL_CON2_PCW_H_SHIFT 24

#define MIPITX_DSI_PLL_CHG 0x60
#define MIPITX_DSI_PLL_CHG_PCW BIT(0)

#define MIPITX_DSI_PLL_PWR 0x68
#define MIPITX_DSI_PLL_PWR_SDM_ISO_EN BIT(1)
#define MIPITX_DSI_PLL_PWR_SDM_PWR_ON BIT(0)

struct mtk_dphy_priv {
  void __iomem *base;
  struct phy_configure_opts_mipi_dphy config;
};

static int mtk_dphy_configure(struct phy *phy, void *params)
{
  struct udevice *dev = phy->dev;
  struct mtk_dphy_priv *priv = dev_get_priv(dev);
  int hs_clk_rate_mhz;
  int txdiv, txdiv0, txdiv1;
  int pcw;
  int ret;

	priv->base = dev_read_addr_ptr(dev);
  if (!priv->base)
    return -EINVAL;

  memcpy(&priv->config, params, sizeof(priv->config));

  ret = phy_mipi_dphy_config_validate(&priv->config);
  if (ret)
    return ret;

  hs_clk_rate_mhz = priv->config.hs_clk_rate / 1000000;
  if (hs_clk_rate_mhz > 1250 || hs_clk_rate_mhz < 50)
    return -EINVAL;

  if (hs_clk_rate_mhz >= 500) {
    txdiv = 1;
    txdiv0 = 0;
    txdiv1 = 0;
  } else if (hs_clk_rate_mhz >= 250) {
    txdiv = 2;
    txdiv0 = 1;
    txdiv1 = 0;
  } else if (hs_clk_rate_mhz >= 125) {
    txdiv = 4;
    txdiv0 = 2;
    txdiv1 = 0;
  } else if (hs_clk_rate_mhz >= 62) {
    txdiv = 8;
    txdiv0 = 2;
    txdiv1 = 1;
  } else if (hs_clk_rate_mhz >= 50) {
    txdiv = 16;
    txdiv0 = 2;
    txdiv1 = 2;
  }

  clrsetbits_32(priv->base + MIPITX_DSI_PLL_CON0,
                MIPITX_DSI_PLL_CON0_TXDIV0_MASK << MIPITX_DSI_PLL_CON0_TXDIV0_SHIFT,
                txdiv0 << MIPITX_DSI_PLL_CON0_TXDIV0_SHIFT);
  clrsetbits_32(priv->base + MIPITX_DSI_PLL_CON0,
                MIPITX_DSI_PLL_CON0_TXDIV1_MASK << MIPITX_DSI_PLL_CON0_TXDIV1_SHIFT,
                txdiv1 << MIPITX_DSI_PLL_CON0_TXDIV1_SHIFT);
  clrbits_32(priv->base + MIPITX_DSI_PLL_CON0,
                MIPITX_DSI_PLL_CON0_PREDIV_MASK << MIPITX_DSI_PLL_CON0_PREDIV_SHIFT);
  setbits_32(priv->base + MIPITX_DSI_PLL_CON1, MIPITX_DSI_PLL_CON1_SDM_FRA_EN);

  pcw = (hs_clk_rate_mhz * txdiv / 13) & MIPITX_DSI_PLL_CON2_PCW_H_MASK;
  clrsetbits_32(priv->base + MIPITX_DSI_PLL_CON2,
                MIPITX_DSI_PLL_CON2_PCW_H_MASK << MIPITX_DSI_PLL_CON2_PCW_H_SHIFT,
                pcw << MIPITX_DSI_PLL_CON2_PCW_H_SHIFT);
  return 0;
}

static int mtk_dphy_power_on(struct phy *phy)
{
  struct udevice *dev = phy->dev;
  struct mtk_dphy_priv *priv = dev_get_priv(dev);

  setbits_32(priv->base + MIPITX_DSI_BG_CON,
              MIPITX_DSI_BG_CON_CORE_EN | MIPITX_DSI_BG_CON_CKEN);
  udelay(30);

  setbits_32(priv->base + MIPITX_DSI_TOP_CON, MIPITX_DSI_TOP_CON_LNT_HS_BIAS_EN);
  setbits_32(priv->base + MIPITX_DSI_CON,
            MIPITX_DSI_CON_CKG_LDOOUT_EN | MIPITX_DSI_CON_LDOOUT_EN);

  clrsetbits_32(priv->base + MIPITX_DSI_PLL_PWR,
            MIPITX_DSI_PLL_PWR_SDM_ISO_EN, MIPITX_DSI_PLL_PWR_SDM_PWR_ON);
  clrbits_32(priv->base + MIPITX_DSI_PLL_CON1, MIPITX_DSI_PLL_CON1_SDM_SSC_EN);
  setbits_32(priv->base + MIPITX_DSI_CLOCK_LANE, MIPITX_DSI_CLOCK_LANE_LDOOUT_EN);

  if (priv->config.lanes > 0)
    setbits_32(priv->base + MIPITX_DSI_LANE0, MIPITX_DSI_CLOCK_LANE_LDOOUT_EN);
  if (priv->config.lanes > 1)
    setbits_32(priv->base + MIPITX_DSI_LANE1, MIPITX_DSI_CLOCK_LANE_LDOOUT_EN);
  if (priv->config.lanes > 2)
    setbits_32(priv->base + MIPITX_DSI_LANE2, MIPITX_DSI_CLOCK_LANE_LDOOUT_EN);
  if (priv->config.lanes > 3)
    setbits_32(priv->base + MIPITX_DSI_LANE3, MIPITX_DSI_CLOCK_LANE_LDOOUT_EN);

  setbits_32(priv->base + MIPITX_DSI_PLL_CON0, MIPITX_DSI_PLL_CON0_PLL_EN);

  udelay(20);

  clrbits_32(priv->base + MIPITX_DSI_PLL_CHG, MIPITX_DSI_PLL_CHG_PCW);
  setbits_32(priv->base + MIPITX_DSI_PLL_CHG, MIPITX_DSI_PLL_CHG_PCW);

  clrbits_32(priv->base + MIPITX_DSI_TOP_CON, MIPITX_DSI_TOP_CON_PAD_TIE_LOW_EN);

  udelay(200);  

  return 0;
}

static const struct phy_ops mtk_dphy_ops = {
  .configure = mtk_dphy_configure,
  .power_on = mtk_dphy_power_on,
};

static const struct udevice_id mtk_dphy_ids[] = {
  { .compatible = "mediatek,mt6580-mipi-tx" },
  { }
};

U_BOOT_DRIVER(mediatek_dphy) = {
  .name = "mediatek_dphy",
  .id = UCLASS_PHY,
  .of_match = mtk_dphy_ids,
  .ops = &mtk_dphy_ops,
  .priv_auto = sizeof(struct mtk_dphy_priv),
  //.probe = mtk_dphy_probe,
};
