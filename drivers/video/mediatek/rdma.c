#include <dm.h>
#include <video.h>
#include <video_bridge.h>
#include <clk.h>
#include <linux/io.h>
#include "common.h"

#define DISP_RDMA_INT_ENABLE 0x00

#define DISP_RDMA_GLOBAL_CON 0x10
#define DISP_RDMA_GLOBAL_CON_ENGINE_EN BIT(0)
#define DISP_RDMA_GLOBAL_CON_MODE_SEL  BIT(1)

#define DISP_RDMA_SIZE_CON_0 0x14
#define DISP_RDMA_SIZE_CON_0_OUTPUT_FRAME_WIDTH_OFFSET 0
#define DISP_RDMA_SIZE_CON_0_OUTPUT_FRAME_WIDTH_MASK 0xfff
#define DISP_RDMA_SIZE_CON_0_MATRIX_ENABLE BIT(17)
#define DISP_RDMA_SIZE_CON_0_MATRIX_INT_MTX_SEL_MASK (0xf << 20)

#define DISP_RDMA_SIZE_CON_1 0x18
#define DISP_RDMA_SIZE_CON_1_OUTPUT_FRAME_HEIGHT_OFFSET 0
#define DISP_RDMA_SIZE_CON_1_OUTPUT_FRAME_HEIGHT_MASK 0xfff

#define DISP_RDMA_MEM_CON 0x24
#define DISP_RDMA_MEM_CON_MODE_INPUT_FORMAT_OFFSET 4
#define DISP_RDMA_MEM_CON_MODE_INPUT_FORMAT_MASK (0xf << 4)
#define DISP_RDMA_MEM_CON_MODE_INPUT_SWAP BIT(8)

#define DISP_RDMA_MEM_SRC_PITCH 0x2c

#define DISP_RDMA_MEM_GMC_SETTING_0 0x30

#define DISP_RDMA_FIFO_CON 0x40
#define DISP_RDMA_FIFO_CON_OUTPUT_VALID_FIFO_THRESHOLD_OFFSET 0
#define DISP_RDMA_FIFO_CON_OUTPUT_VALID_FIFO_THRESHOLD_MASK 0x3ff
#define DISP_RDMA_FIFO_CON_UNDERFLOW_EN BIT(31)

#define DISP_RDMA_MEM_START_ADDR 0xf00

#define DISP_RDMA_OUTPUT_FRAME_WIDTH_MAX 1920
#define DISP_RDMA_OUTPUT_FRAME_HEIGHT_MAX 1080

static int mtk_rdma_probe(struct udevice *dev)
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

static int mtk_rdma_attach(struct udevice *dev)
{
  int ret;
  void __iomem *base;
  struct udevice *mmsys;
  struct video_priv *mmsys_uc_priv;

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  ret = mtk_video_common_get_mmsys(dev, &mmsys);
  if (ret < 0)
    return ret;

  mmsys_uc_priv = dev_get_uclass_priv(mmsys);

  clrbits_32(base + DISP_RDMA_SIZE_CON_0, DISP_RDMA_SIZE_CON_0_MATRIX_ENABLE);
  clrbits_32(base + DISP_RDMA_SIZE_CON_0, DISP_RDMA_SIZE_CON_0_MATRIX_INT_MTX_SEL_MASK);
  // direct link mode
  clrbits_32(base + DISP_RDMA_GLOBAL_CON, DISP_RDMA_GLOBAL_CON_MODE_SEL);
  
  clrbits_32(base + DISP_RDMA_MEM_CON, DISP_RDMA_MEM_CON_MODE_INPUT_FORMAT_MASK);
  clrbits_32(base + DISP_RDMA_MEM_CON, DISP_RDMA_MEM_CON_MODE_INPUT_SWAP);

  writel(0, base + DISP_RDMA_MEM_START_ADDR);
  writel(0, base + DISP_RDMA_MEM_SRC_PITCH);
  clrsetbits_32(base + DISP_RDMA_SIZE_CON_0, DISP_RDMA_SIZE_CON_0_OUTPUT_FRAME_WIDTH_MASK,
            mmsys_uc_priv->xsize << DISP_RDMA_SIZE_CON_0_OUTPUT_FRAME_WIDTH_OFFSET);
  clrsetbits_32(base + DISP_RDMA_SIZE_CON_1, DISP_RDMA_SIZE_CON_1_OUTPUT_FRAME_HEIGHT_MASK,
            mmsys_uc_priv->ysize << DISP_RDMA_SIZE_CON_1_OUTPUT_FRAME_HEIGHT_OFFSET);
  
  writel(0x1a01356b, base + DISP_RDMA_MEM_GMC_SETTING_0); // ??? imported from lk source

  clrsetbits_32(base + DISP_RDMA_FIFO_CON,
                DISP_RDMA_FIFO_CON_OUTPUT_VALID_FIFO_THRESHOLD_MASK,
                ((mmsys_uc_priv->xsize * VNBYTES(mmsys_uc_priv->bpix) * 125) / (16 * 1000)) << DISP_RDMA_FIFO_CON_OUTPUT_VALID_FIFO_THRESHOLD_OFFSET);
  setbits_32(base + DISP_RDMA_FIFO_CON, DISP_RDMA_FIFO_CON_UNDERFLOW_EN);
  writel(0x1f, base + DISP_RDMA_INT_ENABLE);

  setbits_32(base + DISP_RDMA_GLOBAL_CON, DISP_RDMA_GLOBAL_CON_ENGINE_EN);

  return mtk_video_common_attach(dev);
}

static struct video_bridge_ops mtk_rdma_ops = {
  .attach = mtk_rdma_attach,
  //.set_backlight = mtk_video_common_set_backlight,
  .get_display_timing = mtk_video_common_get_display_timing,
};

static const struct udevice_id mtk_rdma_ids[] = {
  { .compatible = "mediatek,mt6580-disp-rdma" },
  { }
};

U_BOOT_DRIVER(mediatek_rdma) = {
  .name    = "mediatek_rdma",
  .id  = UCLASS_VIDEO_BRIDGE,
  .ops       = &mtk_rdma_ops,
  .of_match   = mtk_rdma_ids,
  .probe    = mtk_rdma_probe,
};
