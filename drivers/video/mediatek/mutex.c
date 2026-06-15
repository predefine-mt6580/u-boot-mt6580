// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Pavel Ivanov <predefine@yandex.ru>
 */

#include <dm.h>
#include <linux/io.h>
#include "mutex.h"

#define DISP_MUTEX_EN_OFFSET(mutex)  (0x20 + (mutex * 0x20))
#define DISP_MUTEX_MOD_OFFSET(mutex) (0x2C + (mutex * 0x20))
#define DISP_MUTEX_SOF_OFFSET(mutex) (0x30 + (mutex * 0x20))

int mtk_mutex_setup(struct udevice *dev, int mode, int mutex_id, int mutex_device)
{
  void __iomem *base;
  // u32 sof;

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  // sof = (mode == MUTEX_MODE_SINGLE) ? 0 : 1;

  writel(0, base + DISP_MUTEX_EN_OFFSET(mutex_id));

  writel(/* mutex ops */ 0x2f140, base + DISP_MUTEX_MOD_OFFSET(mutex_id));
  writel(/*sof*/ 1, base + DISP_MUTEX_SOF_OFFSET(mutex_id));
  writel(1, base + DISP_MUTEX_EN_OFFSET(mutex_id));

  return 0;
}

static int mtk_mutex_probe(struct udevice *dev)
{
  void __iomem *base;

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  // disable all mutex
  for (int i = 0; i < 6; i++)
    writel(0, base + DISP_MUTEX_EN_OFFSET(i));

  return 0;
}

static const struct udevice_id mtk_mutex_ids[] = {
  { .compatible = "mediatek,mt6580-disp-mutex" },
  { }
};

U_BOOT_DRIVER(mediatek_mutex) = {
  .name  = "mediatek_mutex",
  .id          = UCLASS_NOP,
  .of_match = mtk_mutex_ids,
  .probe  = mtk_mutex_probe,
};
