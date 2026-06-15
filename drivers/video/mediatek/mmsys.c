// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Pavel Ivanov <predefine@yandex.ru>
 */

#include <dm.h>
#include <video.h>
#include <video_bridge.h>
#include <clk.h>
#include <linux/io.h>
#include <dt-bindings/video/mtk.h>
#include "common.h"

#define MTK_MMSYS_REG(_offset, _mask, _shift) (struct mtk_mmsys_reg){.offset = _offset, .mask = _mask, .shift = _shift}

struct mtk_mmsys_reg {
  u32 offset;
  u32 mask;
  u32 shift;
};

struct mtk_mmsys_driver_data {
  struct mtk_mmsys_reg path_regs[DISP_PATH_NR];
};

static int mtk_mmsys_connect_path(struct udevice *dev)
{
  int ret;
  int path_size;
  void __iomem *base;
  struct mtk_mmsys_reg reg;
  struct mtk_mmsys_driver_data *ddata = (void*) dev_get_driver_data(dev);

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  path_size = dev_read_size(dev, "mediatek,path");
  if (path_size < 0)
    return path_size;
  path_size /= sizeof(u32);

  u32 path[path_size];

  ret = dev_read_u32_array(dev, "mediatek,path", &path[0], path_size);
  if (ret < 0)
    return ret;

  for (int i = 0; i < path_size; i += 2)
  {
    if (path[i] >= DISP_PATH_NR)
      return -EINVAL;

    reg = ddata->path_regs[path[i]];
    clrsetbits_32(base + reg.offset, reg.mask << reg.shift, (path[i + 1] & reg.mask) << reg.shift);
  }

  return 0;
}

static int mtk_mmsys_probe(struct udevice *dev)
{
  struct video_uc_plat *plat;
  struct video_priv *uc_priv;
  struct clk clk;
  struct display_timing timings;
  struct udevice *bridge;
  int ret;

  uc_priv = dev_get_uclass_priv(dev);
  plat = dev_get_uclass_plat(dev);

  ret = clk_get_by_index(dev, 0, &clk);
  if (!ret)
  {
    ret = clk_enable(&clk);
    if (ret)
      return ret;
  }

  ret = mtk_video_common_get_display_timing(dev, &timings);
  if (ret)
    return ret;

  uc_priv->xsize = timings.hactive.typ;
  uc_priv->ysize = timings.vactive.typ;

  uc_priv->rot = 0;

  uc_priv->bpix = VIDEO_BPP32;
  uc_priv->format = VIDEO_RGBA8888;
  uc_priv->flush_dcache = true;

  plat->size = uc_priv->xsize * uc_priv->ysize * VNBYTES(uc_priv->bpix);

  ret = mtk_mmsys_connect_path(dev);
  if (ret < 0)
    return ret;

  ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, dev, 0, -1, &bridge);
  if (!ret)
    video_bridge_attach(bridge);

  return 0;
}

static int mtk_mmsys_bind(struct udevice *dev)
{
  struct video_uc_plat *plat = dev_get_uclass_plat(dev);

  plat->size = 1920 * 1080 * VNBYTES(VIDEO_BPP32);
  
  return dm_scan_fdt_dev(dev);
}

static const struct mtk_mmsys_driver_data mt6580_data = {
  .path_regs = {
    [DISP_PATH_OVL0_MOUT] = MTK_MMSYS_REG(0x30, 3, 0),
    [DISP_PATH_DITHER_MOUT] = MTK_MMSYS_REG(0x38, 7, 0),
    [DISP_PATH_COLOR0_IN] = MTK_MMSYS_REG(0x58, 1, 0),
    [DISP_PATH_DSI0_IN] = MTK_MMSYS_REG(0x64, 3, 0),
    [DISP_PATH_RDMA0_OUT] = MTK_MMSYS_REG(0x6C, 3, 0),
  },
};

static const struct udevice_id mtk_mmsys_ids[] = {
  { .compatible = "mediatek,mt6580-mmsys", .data = (ulong)&mt6580_data },
  { }
};

U_BOOT_DRIVER(mediatek_mmsys) = {
  .name   = "mediatek_mmsys",
  .id         = UCLASS_VIDEO,
  .of_match  = mtk_mmsys_ids,
  .probe   = mtk_mmsys_probe,
  .bind     = mtk_mmsys_bind,
  .flags = DM_FLAG_PRE_RELOC,
};
