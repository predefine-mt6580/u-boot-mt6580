// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <power/pmic.h>
#include <power/mtk_pmic.h>

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "", .driver = "mtk_regulator" },
	{ },
};

static int mtk_pmic_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children;

	// TODO: sysreset

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s regulators subnode not found\n", dev->name);
		return -EINVAL;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s has no children (regulators)\n", dev->name);

	return 0;
}

static u32 mt63xx_vmc_voltages[] = {
	1800000,
	3300000
};

static u32 mt63xx_vmch_voltages[] = {
	3000000,
	3300000
};

static u32 mt63xx_vgp3_voltages[] = {
	1200000,
  1300000,
  1500000,
  1800000
};

static struct mtk_pmic_regulator mt63xx_regulators[] = {
	{
		.name = "vmc",
		.en = MTK_PMIC_REG(0x504, 1, 12),
		.vosel = MTK_PMIC_REG(0x52a, 1, 4),
		.voltages_count = 2,
		.voltages = mt63xx_vmc_voltages,
	},
	{
		.name = "vmch",
		.en = MTK_PMIC_REG(0x506, 1, 14),
		.vosel = MTK_PMIC_REG(0x52c, 1, 7),
		.voltages_count = 2,
		.voltages = mt63xx_vmch_voltages,
	},
  {
    .name = "vgp3",
    .en = MTK_PMIC_REG(0x50e, 1, 15),
    .vosel = MTK_PMIC_REG(0x534, 3, 5),
    .voltages_count = 4,
    .voltages = mt63xx_vgp3_voltages,
  },
	{ },
};

static struct mtk_pmic_pdata mt63xx_data = {
	.regulators = mt63xx_regulators,
	.regs = { },
};

static const struct udevice_id mtk_pmic_match[] = {
	/* mt63xx_data struct is fine for both PMICs */
	{ .compatible = "mediatek,mt6323", .data = (ulong)&mt63xx_data },
	{ .compatible = "mediatek,mt6350", .data = (ulong)&mt63xx_data },
	{ }
};

U_BOOT_DRIVER(mtk_pmic) = {
	.name = "mtk_pmic",
	.id = UCLASS_NOP,
	.of_match = mtk_pmic_match,
	.bind = mtk_pmic_bind,
};
