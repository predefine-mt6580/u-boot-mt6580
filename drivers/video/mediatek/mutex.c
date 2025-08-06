#include <dm.h>
#include <linux/io.h>
#include "mutex.h"

#define DISP_MUTEX_EN_OFFSET(mutex)  (0x20 + (mutex * 0x20))
#define DISP_MUTEX_MOD_OFFSET(mutex) (0x2C + (mutex * 0x20))
#define DISP_MUTEX_SOF_OFFSET(mutex) (0x30 + (mutex * 0x20))

int mtk_mutex_setup(struct udevice *dev, int mode, int mutex_id, int mutex_device)
{
  void __iomem *base;

  base = dev_read_addr_ptr(dev);
  if (!base)
    return -EINVAL;

  writel(0, base + DISP_MUTEX_EN_OFFSET(mutex_id));

  clrsetbits_32(base + DISP_MUTEX_MOD_OFFSET(mutex_id), 1, mode & 1);
  setbits_32(base + DISP_MUTEX_SOF_OFFSET(mutex_id), 0x140); //BIT(mutex_device));
  writel(1, base + DISP_MUTEX_EN_OFFSET(mutex_id));

  return -EINVAL;
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
