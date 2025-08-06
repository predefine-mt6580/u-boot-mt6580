#include <dm.h>
#include <video.h>
#include <video_bridge.h>
#include <clk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include "common.h"

#define DISP_OVL_EN 0xC

#define DISP_OVL_RST 0x14

#define DISP_OVL_ROI_SIZE 0x20
#define DISP_OVL_ROI_SIZE_H_OFFSET 16
#define DISP_OVL_ROI_SIZE_W_OFFSET 0

#define DISP_OVL_DATAPATH_CON 0x24
#define DISP_OVL_DATAPATH_CON_LAYER_SMI_ID_EN BIT(0)

#define DISP_OVL_ROI_BGCLR 0x28

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

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  ret = mtk_video_common_get_mmsys(dev, &mmsys);
  if (ret < 0)
    return ret;

  mmsys_plat = dev_get_uclass_plat(mmsys);
  mmsys_uc_priv = dev_get_uclass_priv(mmsys);

  // reset ovl
  setbits_32(base + DISP_OVL_RST, 1);
  clrbits_32(base + DISP_OVL_RST, 1);
  mdelay(10); // TODO: remove?

  //clrbits_32(base + DISP_RDMA_SIZE_CON_0, DISP_RDMA_SIZE_CON_0_MATRIX_ENABLE);
  //writel(mmsys_uc_priv->xsize * VNBYTES(mmsys_uc_priv->bpix),
  //        base + DISP_RDMA_MEM_SRC_PITCH);


  // configure ovl roi
  writel(mmsys_uc_priv->xsize << DISP_OVL_ROI_SIZE_W_OFFSET |
         mmsys_uc_priv->ysize << DISP_OVL_ROI_SIZE_H_OFFSET, base + DISP_OVL_ROI_SIZE);
  writel(0xff00ffff, base + DISP_OVL_ROI_BGCLR);

  // configure ovl layer 0

  ret = mtk_video_common_attach(dev);
  if (ret < 0)
    return ret;

  // configure ovl
  // seems useless without handling interrupts
  //writel(0xE, base + DISP_OVL_INTEN);
  writel(1, base + DISP_OVL_EN);
  setbits_32(base + DISP_OVL_DATAPATH_CON, DISP_OVL_DATAPATH_CON_LAYER_SMI_ID_EN);

  return 0;//mtk_video_common_attach(dev);
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
