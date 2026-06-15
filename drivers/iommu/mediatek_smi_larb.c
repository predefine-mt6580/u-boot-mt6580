// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <clk.h>

static int mediatek_smi_larb_probe(struct udevice *dev)
{
  int err;
  struct clk_bulk clks;

  err = clk_get_bulk(dev, &clks);
  if (err)
    return 0;

  err = clk_enable_bulk(&clks);
  if (err)
    return err;

  return 0;
}

static const struct udevice_id mediatek_smi_larb_ids[] = {
  { .compatible = "mediatek,smi-larb" },
  { .compatible = "mediatek,mt6580-smi-larb" },
  { }
};

U_BOOT_DRIVER(mediatek_smi_larb) = {
  .name = "mediatek_smi_larb",
  .id = UCLASS_NOP,
  .of_match = mediatek_smi_larb_ids,
  .probe = mediatek_smi_larb_probe,
};
