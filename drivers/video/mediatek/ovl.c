#include <dm.h>
#include <video.h>
#include <video_bridge.h>
#include <clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "common.h"

#define DISP_OVL_INTEN 0x0

#define DISP_OVL_EN 0xC
#define DISP_OVL_RST 0x14

#define DISP_OVL_ROI_SIZE 0x20
#define DISP_OVL_ROI_SIZE_H_OFFSET 16
#define DISP_OVL_ROI_SIZE_W_OFFSET 0

#define DISP_OVL_DATAPATH_CON 0x24
#define DISP_OVL_DATAPATH_CON_LAYER_SMI_ID_EN BIT(0)

#define DISP_OVL_ROI_BGCLR 0x28
#define DISP_OVL_SRC_CON 0x2C

#define DISP_OVL_L0_CON 0x30
#define DISP_OVL_L0_CON_CFMT_ABGR8888 3

#define DISP_OVL_L0_SRC_SIZE 0x38
#define DISP_OVL_L0_OFFSET 0x3C
#define DISP_OVL_L0_PITCH 0x44
#define DISP_OVL_L0_ADDR 0xF40

#define DISP_OVL_RDMA0_CTRL 0xC0
#define DISP_OVL_RDMA0_MEM_GMC_SETTING 0xC8


static void mtk_ovl_layer0_config(void __iomem *base, ulong fb_addr,
                u32 width, u32 height, u32 pitch)
{
  u32 l0_con = (DISP_OVL_L0_CON_CFMT_ABGR8888 << 12);

  writel(1, base + DISP_OVL_RDMA0_CTRL);
  writel(l0_con, base + DISP_OVL_L0_CON);
  writel(0, base + DISP_OVL_L0_OFFSET);
  writel(fb_addr, base + DISP_OVL_L0_ADDR);
  writel((height << 16) | width, base + DISP_OVL_L0_SRC_SIZE);
  writel(pitch, base + DISP_OVL_L0_PITCH);
  writel(0x6070, base + DISP_OVL_RDMA0_MEM_GMC_SETTING);
  writel(BIT(0), base + DISP_OVL_SRC_CON);
}

static int mtk_ovl_probe(struct udevice *dev)
{
  int ret;
  struct clk clk;

  ret = clk_get_by_index(dev, 0, &clk);
  if (ret)
    return ret;

  ret = clk_enable(&clk);
  if (ret)
    return ret;

  return 0;
}

static int mtk_ovl_attach(struct udevice *dev)
{
  int ret;
  void __iomem *base;
  struct udevice *mmsys;
  struct video_uc_plat *mmsys_plat;
  struct video_priv *mmsys_uc_priv;
  u32 pitch;

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  ret = mtk_video_common_get_mmsys(dev, &mmsys);
  if (ret < 0)
    return ret;

  mmsys_plat = dev_get_uclass_plat(mmsys);
  mmsys_uc_priv = dev_get_uclass_priv(mmsys);

  if (!mmsys_plat->base)
    return -EINVAL;

  setbits_32(base + DISP_OVL_RST, 1);
  clrbits_32(base + DISP_OVL_RST, 1);
  mdelay(10);

  writel(mmsys_uc_priv->xsize << DISP_OVL_ROI_SIZE_W_OFFSET |
         mmsys_uc_priv->ysize << DISP_OVL_ROI_SIZE_H_OFFSET, base + DISP_OVL_ROI_SIZE);
  writel(0xff000000, base + DISP_OVL_ROI_BGCLR);

  ret = mtk_video_common_attach(dev);
  if (ret < 0)
    return ret;


  pitch = mmsys_uc_priv->xsize * VNBYTES(mmsys_uc_priv->bpix);

  mtk_ovl_layer0_config(base, mmsys_plat->base, mmsys_uc_priv->xsize,
              mmsys_uc_priv->ysize, pitch);

  writel(0xE, base + DISP_OVL_INTEN);
  writel(1, base + DISP_OVL_EN);
  setbits_32(base + DISP_OVL_DATAPATH_CON, DISP_OVL_DATAPATH_CON_LAYER_SMI_ID_EN);

  return 0;
}

static struct video_bridge_ops mtk_ovl_ops = {
  .attach = mtk_ovl_attach,
  //.set_backlight = mtk_video_common_set_backlight,
  .get_display_timing = mtk_video_common_get_display_timing,
};

static const struct udevice_id mtk_ovl_ids[] = {
  { .compatible = "mediatek,mt6580-disp-ovl" },
  { }
};

U_BOOT_DRIVER(mediatek_ovl) = {
  .name    = "mediatek_ovl",
  .id = UCLASS_VIDEO_BRIDGE,
  .ops       = &mtk_ovl_ops,
  .of_match   = mtk_ovl_ids,
  .probe    = mtk_ovl_probe,
};
