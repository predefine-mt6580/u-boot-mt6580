// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <iommu.h>
#include <linux/io.h>
#include <dt-bindings/memory/mtk-memory-port.h>

#define LARB_MMU_EN_OFFSET 0xfc0 

static int mediatek_m4u_connect(struct udevice *dev)
{
  struct ofnode_phandle_args args;
  struct udevice *larb;
  ofnode larb_node;
  int ret, m4u_port, larb_addr;

  ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "iommus",
            "#iommu-cells", 0, 0, &args);
  if (ret < 0)
    return -EINVAL;

  m4u_port = args.args[0];

  larb_node = ofnode_parse_phandle(dev_ofnode(dev->iommu), "mediatek,larbs",
                                    MTK_M4U_TO_LARB(m4u_port));
  if (!ofnode_valid(larb_node))
    return -EINVAL;

  ret = device_get_global_by_ofnode(larb_node, &larb);
  if (ret < 0)
    return ret;

  larb_addr = dev_read_addr(larb);

  // Bypass address translation for m4u port
  clrbits_32(larb_addr + LARB_MMU_EN_OFFSET, BIT(MTK_M4U_TO_PORT(m4u_port)));

  return 0;
}

static int mediatek_m4u_probe(struct udevice *dev)
{
  return 0;
}
static struct iommu_ops mediatek_m4u_ops = {
  .connect = mediatek_m4u_connect,
};

static const struct udevice_id mediatek_m4u_ids[] = {
  { .compatible = "mediatek,m4u" },
  { .compatible = "mediatek,mt6580-m4u" },
  { }
};

U_BOOT_DRIVER(mediatek_m4u) = {
  .name = "mediatek_m4u",
  .id = UCLASS_IOMMU,
  .of_match = mediatek_m4u_ids,
  .ops = &mediatek_m4u_ops,
  .probe = mediatek_m4u_probe,
};
