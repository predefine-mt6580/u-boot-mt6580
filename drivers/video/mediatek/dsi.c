#include <dm.h>
#include <video_bridge.h>
#include <clk.h>
#include <linux/io.h>
#include <mipi_dsi.h>
#include <phy-mipi-dphy.h>
#include <panel.h>
#include <div64.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <generic-phy.h>
#include "common.h"

#define DSI_START 0x00
#define DSI_START_BIT BIT(0)

#define DSI_COM_CTRL 0x10
#define DSI_COM_CTRL_RESET BIT(0)

#define DSI_INTSTA 0x0c
#define DSI_INTSTA_BUSY BIT(31)
#define DSI_INTSTA_FRAME_DONE BIT(4)
#define DSI_INTSTA_CMD_DONE BIT(1)

#define DSI_MODE_CON 0x14
#define DSI_TXRX_CTRL 0x18
#define DSI_PSCTRL 0x1c
#define DSI_VSA_NL 0x20
#define DSI_VBP_NL 0x24
#define DSI_VFP_NL 0x28
#define DSI_VACT_NL 0x2c

#define DSI_HSA_WC 0x50
#define DSI_HBP_WC 0x54
#define DSI_HFP_WC 0x58
#define DSI_BLLP_WC 0x5c
#define DSI_CMDQ_CON 0x60

#define DSI_MEM_CONTI 0x90
#define DSI_WMEM_CONTI 0x3c

#define DSI_PHY_LCCON 0x104
#define DSI_PHY_LCCON_HSTX_EN BIT(0)

#define DSI_PHY_TIMCON0 0x110
#define DSI_PHY_TIMCON1 0x114
#define DSI_PHY_TIMCON2 0x118
#define DSI_PHY_TIMCON3 0x11c

#define DSI_VM_CMD_CON 0x130
#define DSI_VM_CMD_CON_TS_VFP_EN BIT(5)
#define DSI_VM_CMD_CON_VM_CMD_EN BIT(0)

#define DSI_CMDQ 0x200

struct mtk_dsi_priv {
  void __iomem *base;

  struct clk_bulk clks;

  struct mipi_dsi_host host;
  struct mipi_dsi_device device;

  struct udevice *panel;
  struct display_timing timings;
  int bpp;

  struct phy phy;
  struct phy_configure_opts_mipi_dphy phy_opts;
};

static int mtk_dsi_of_to_plat(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);
  int ret;

  priv->base = dev_read_addr_ptr(dev);
  if (!priv->base)
    return -EINVAL;

  ret = clk_get_bulk(dev, &priv->clks);
  if (ret)
    return ret;

  ret = generic_phy_get_by_name(dev, "dphy", &priv->phy);
  if (ret < 0)
    return ret;

  return 0;
}

static int mtk_dsi_probe(struct udevice *dev)
{
  return 0;
}

static void mtk_dsi_reset(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);

  setbits_32(priv->base + DSI_COM_CTRL, DSI_COM_CTRL_RESET);
  clrbits_32(priv->base + DSI_COM_CTRL, DSI_COM_CTRL_RESET);
}

static void mtk_dsi_start(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);
  
  clrbits_32(priv->base + DSI_START, DSI_START_BIT);
  setbits_32(priv->base + DSI_START, DSI_START_BIT);
}

static void mtk_dsi_wait_for_not_busy(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);

  while(readl(priv->base + DSI_INTSTA) & DSI_INTSTA_BUSY)
    udelay(10);
}

static void mtk_dsi_wait_int(struct udevice *dev, int flag)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);

  while(!(readl(priv->base + DSI_INTSTA) & flag))
    udelay(10);

  setbits_32(priv->base + DSI_INTSTA, flag);
}

static ssize_t mtk_dsi_host_transfer(struct mipi_dsi_host *host,
               const struct mipi_dsi_msg *msg)
{
  struct udevice *dev = (struct udevice *)host->dev;
  struct mtk_dsi_priv *priv = dev_get_priv(dev);

  mtk_dsi_wait_for_not_busy(dev);

  switch (msg->type)
  {
    case MIPI_DSI_DCS_SHORT_WRITE:
    case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
      writel(
        (msg->tx_len == 1 ? 0 : (((char*)msg->tx_buf)[1] << 24)) |
        (((char*)msg->tx_buf)[0] << 16) |
        (msg->type << 8),
        priv->base + DSI_CMDQ);
      writel(1, priv->base + DSI_CMDQ_CON);
      mtk_dsi_start(dev);
      mtk_dsi_wait_int(dev, DSI_INTSTA_CMD_DONE);
      break;
    default:
      return -EINVAL;
  }

  return 0;
}

static const struct mipi_dsi_host_ops mtk_dsi_host_ops = {
  .transfer = mtk_dsi_host_transfer,
};

static int mtk_dsi_get_phy_config(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);
  unsigned long long ui;
  int ret;

  priv->bpp = mipi_dsi_pixel_format_to_bpp(priv->device.format);
  if (priv->bpp < 0)
    return priv->bpp;

  ret = panel_get_display_timing(priv->panel, &priv->timings);
  if (ret < 0)
    return ret;

  ret = phy_mipi_dphy_get_default_config(priv->timings.pixelclock.typ,
                                          priv->bpp, priv->device.lanes, &priv->phy_opts);
  if (ret < 0)
    return ret;

  ui = ALIGN(PSEC_PER_SEC, priv->phy_opts.hs_clk_rate);
  do_div(ui, priv->phy_opts.hs_clk_rate);

  priv->phy_opts.clk_trail = 96000;
  priv->phy_opts.clk_zero = 400000;

  priv->phy_opts.lpx = 80000;
  priv->phy_opts.ta_get = 5 * priv->phy_opts.lpx;
  priv->phy_opts.ta_go = 4 * priv->phy_opts.lpx;
  priv->phy_opts.ta_sure = priv->phy_opts.lpx * 3 / 2;

  priv->phy_opts.hs_exit = 2 * priv->phy_opts.lpx;
  priv->phy_opts.hs_prepare = 64000 + 5 * ui;
  priv->phy_opts.hs_zero = 136000 + 5 * ui;
  priv->phy_opts.hs_trail = 80000 + 4 * ui;

  ret = phy_mipi_dphy_config_validate(&priv->phy_opts);
  if (ret < 0)
    return ret;

  return 0;
}

static int mtk_dsi_hw_init(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);
  int lane_num_bits;
  int hs_clk_rate_mhz;
  int ret;

  ret = mtk_dsi_get_phy_config(dev);
  if (ret < 0)
    return ret;

  if (!generic_phy_valid(&priv->phy))
    return -EINVAL;

  ret = generic_phy_configure(&priv->phy, &priv->phy_opts);
  if (ret < 0)
    return ret;

  ret = generic_phy_power_on(&priv->phy);
  if (ret < 0)
    return ret;

  hs_clk_rate_mhz = priv->phy_opts.hs_clk_rate / 1000000;

  switch (priv->device.lanes) {
    case 1:
      lane_num_bits = 0x1;
      break;
    case 2:
      lane_num_bits = 0x3;
      break;
    case 3:
      lane_num_bits = 0x7;
      break;
    case 4:
      lane_num_bits = 0xf;
      break;
  }

  writel(lane_num_bits << 2, priv->base + DSI_TXRX_CTRL);
  writel(DSI_WMEM_CONTI, priv->base + DSI_MEM_CONTI);

  clrsetbits_32(priv->base + DSI_VACT_NL, 0xfff, priv->timings.vactive.typ & 0xfff);
  clrsetbits_32(priv->base + DSI_PSCTRL, 0x3fff,
                (priv->timings.hactive.typ * priv->bpp) & 0x3fff);
  clrsetbits_32(priv->base + DSI_PSCTRL, 3 << 16,
                ((3 - priv->device.format) & 3) << 16);

  writel((priv->phy_opts.hs_trail * hs_clk_rate_mhz / 8000000) << 24 |
         (priv->phy_opts.hs_zero * hs_clk_rate_mhz / 8000000) << 16 |
         (priv->phy_opts.hs_prepare * hs_clk_rate_mhz / 8000000) << 8 |
         (priv->phy_opts.lpx * hs_clk_rate_mhz / 8000000) << 0,
          priv->base + DSI_PHY_TIMCON0);
  writel((priv->phy_opts.hs_exit * hs_clk_rate_mhz / 8000000) << 24 |
         (priv->phy_opts.ta_get * hs_clk_rate_mhz / 8000000) << 16 |
         (priv->phy_opts.ta_sure * hs_clk_rate_mhz / 8000000) << 8 |
         (priv->phy_opts.ta_go * hs_clk_rate_mhz / 8000000) << 0,
          priv->base + DSI_PHY_TIMCON1);
  writel((priv->phy_opts.clk_trail * hs_clk_rate_mhz / 8000000) << 24 |
         (priv->phy_opts.clk_zero * hs_clk_rate_mhz / 8000000) << 16 |
         1 << 8 |
         0 << 0,
          priv->base + DSI_PHY_TIMCON2);
  writel((priv->phy_opts.hs_exit * hs_clk_rate_mhz / 8000000) << 16 |
         (priv->phy_opts.clk_post * hs_clk_rate_mhz / 8000000) << 8 |
         (priv->phy_opts.clk_prepare * hs_clk_rate_mhz / 8000000) << 0,
          priv->base + DSI_PHY_TIMCON3);

  if (priv->device.mode_flags & MIPI_DSI_MODE_VIDEO) { 
    clrsetbits_32(priv->base + DSI_VSA_NL, 0xfff, priv->timings.vsync_len.typ & 0xfff);
    clrsetbits_32(priv->base + DSI_VBP_NL, 0xfff, priv->timings.vback_porch.typ & 0xfff);
    clrsetbits_32(priv->base + DSI_VFP_NL, 0xfff, priv->timings.vfront_porch.typ & 0xfff);

    if (priv->device.mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
      clrsetbits_32(priv->base + DSI_HSA_WC, 0xfff,
                  ALIGN(priv->timings.hsync_len.typ * priv->bpp, 4) & 0xfff);
      clrsetbits_32(priv->base + DSI_HBP_WC, 0xfff,
                  ALIGN(priv->timings.hback_porch.typ * priv->bpp, 4) & 0xfff);
    } else {
      clrsetbits_32(priv->base + DSI_HSA_WC, 0xfff,
                  ALIGN(priv->timings.hsync_len.typ * priv->bpp - 4, 4) & 0xfff);
      clrsetbits_32(priv->base + DSI_HBP_WC, 0xfff,
                  ALIGN((priv->timings.hback_porch.typ + priv->timings.hsync_len.typ) * priv->bpp - 10, 4) & 0xfff);
    }
    clrsetbits_32(priv->base + DSI_HFP_WC, 0xfff,
                ALIGN(priv->timings.hfront_porch.typ * priv->bpp, 4) & 0xfff);

    // ???
    clrsetbits_32(priv->base + DSI_BLLP_WC, 0xfff, 0);
    //clrsetbits_32(priv->base + DSI_BLLP_WC, 0xfff,
    //            ALIGN(priv->timings.hfront_porch.typ * priv->bpp - 12, 4) & 0xfff);

    setbits_32(priv->base + DSI_VM_CMD_CON,
              DSI_VM_CMD_CON_TS_VFP_EN | DSI_VM_CMD_CON_VM_CMD_EN);
  }
  //log_crit("DSI timconfig: %.8x %.8x %.8x %.8x %ld\n",
  //     priv->phy_opts.hs_clk_rate);

  return 0;
}

static int mtk_dsi_attach(struct udevice *dev)
{
  struct mtk_dsi_priv *priv = dev_get_priv(dev);
  struct mipi_dsi_device *device = &priv->device;
  struct mipi_dsi_panel_plat *mipi_plat;
  int dsi_mode = 0;
  int ret;

  ret = mtk_video_common_get_panel(dev, &priv->panel);
  if (ret < 0)
    return ret;

  mipi_plat = dev_get_plat(priv->panel);
  mipi_plat->device = device;

  priv->host.dev = (struct device *)dev;
  priv->host.ops = &mtk_dsi_host_ops;

  device->host = &priv->host;
  device->lanes = mipi_plat->lanes;
  device->format = mipi_plat->format;
  device->mode_flags = mipi_plat->mode_flags;

  ret = clk_enable_bulk(&priv->clks);
  if (ret < 0)
    return ret;

  mtk_dsi_reset(dev);

  // switch to command mode
  clrbits_32(priv->base + DSI_MODE_CON, 3);

  ret = mtk_dsi_hw_init(dev);
  if (ret < 0)
    return ret;

  ret = panel_enable_backlight(priv->panel);
  if (ret < 0)
    return ret;

  if (priv->device.mode_flags & MIPI_DSI_MODE_VIDEO_SYNC_PULSE) {
    dsi_mode = 1;
  } else if (priv->device.mode_flags & MIPI_DSI_MODE_VIDEO_BURST) {
    dsi_mode = 3;
  } else if (priv->device.mode_flags & MIPI_DSI_MODE_VIDEO) {
    dsi_mode = 2;
  }

  // switch back to video mode
  clrsetbits_32(priv->base + DSI_MODE_CON, 3, dsi_mode & 3);
  setbits_32(priv->base + DSI_PHY_LCCON, DSI_PHY_LCCON_HSTX_EN);

  mtk_dsi_start(dev);

  return 0;
}

static struct video_bridge_ops mtk_dsi_ops = {
  .attach = mtk_dsi_attach,
  .get_display_timing = mtk_video_common_get_display_timing,
};

static const struct udevice_id mtk_dsi_ids[] = {
  { .compatible = "mediatek,mt6580-dsi" },
  { }
};

U_BOOT_DRIVER(mediatek_dsi) = {
  .name                   = "mediatek_dsi",
  .id                = UCLASS_VIDEO_BRIDGE,
  .ops                      = &mtk_dsi_ops,
  .of_match                  = mtk_dsi_ids,
  .of_to_plat                = mtk_dsi_of_to_plat,
  .probe                   = mtk_dsi_probe,
  .priv_auto = sizeof(struct mtk_dsi_priv),
};
