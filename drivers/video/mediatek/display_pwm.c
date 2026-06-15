// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <dm.h>
#include <backlight.h>
#include <log.h>
#include <clk.h>
#include <linux/io.h>

struct mediatek_displaypwm_priv {
    void __iomem *regs;

    struct clk_bulk clks;

    u32 levels;
};

#define DISP_PWM_EN_OFF 0x0
#define DISP_PWM_COMMIT_OFF 0x8
#define DISP_PWM_CON_0_OFF 0x10
#define DISP_PWM_CON_1_OFF 0x14


static int mediatek_displaypwm_enable(struct udevice *dev)
{
    return 0;
}

static int mediatek_displaypwm_set_brightness(struct udevice *dev, int percent)
{
    struct mediatek_displaypwm_priv *priv = dev_get_priv(dev);

    if (percent == BACKLIGHT_DEFAULT)
        percent = 100;
    else if (percent == BACKLIGHT_OFF)
        percent = 0;

    writel(0, priv->regs + DISP_PWM_COMMIT_OFF);

    if (percent == 0)
    {
        writel(0, priv->regs + DISP_PWM_EN_OFF);
    } else {
        writel(
            ((priv->levels * percent) / 100) << 16 |
            priv->levels, priv->regs + DISP_PWM_CON_1_OFF
        );
        writel(1, priv->regs + DISP_PWM_EN_OFF);
    }

    writel(1, priv->regs + DISP_PWM_COMMIT_OFF);
    writel(0, priv->regs + DISP_PWM_COMMIT_OFF);

    return 0;
}

static int mediatek_displaypwm_probe(struct udevice *dev)
{
    struct mediatek_displaypwm_priv *priv = dev_get_priv(dev);
    int ret;

    ret = clk_get_bulk(dev, &priv->clks);
    if (ret)
        return ret;

    ret = clk_enable_bulk(&priv->clks);
    if (ret)
        return ret;

    writel(0, priv->regs + DISP_PWM_COMMIT_OFF);

    // reset pwm config
    writel(0, priv->regs + DISP_PWM_CON_0_OFF);

    writel(0, priv->regs + DISP_PWM_EN_OFF);

    writel(1, priv->regs + DISP_PWM_COMMIT_OFF);
    writel(0, priv->regs + DISP_PWM_COMMIT_OFF);

    return 0;
}

static int mediatek_displaypwm_of_to_plat(struct udevice *dev)
{
    struct mediatek_displaypwm_priv *priv = dev_get_priv(dev);

    priv->regs = dev_read_addr_ptr(dev);
    if(!priv->regs)
        return -EINVAL;

    priv->levels = dev_read_u32_default(dev, "levels", 0xff);
    if (priv->levels >= 0x1000)
        return -EINVAL;

    return 0;
}

static const struct backlight_ops mediatek_displaypwm_ops = {
    .enable = mediatek_displaypwm_enable,
    .set_brightness = mediatek_displaypwm_set_brightness,
};

static const struct udevice_id mediatek_displaypwm_ids[] = {
    { .compatible = "mediatek,mt6580-display-pwm" },
    { }
};

U_BOOT_DRIVER(mediatek_displaypwm) = {
    .name		= "mediatek_displaypwm",
    .id		= UCLASS_PANEL_BACKLIGHT,
    .of_match	= mediatek_displaypwm_ids,
    .of_to_plat	= mediatek_displaypwm_of_to_plat,
    .probe		= mediatek_displaypwm_probe,
    .ops		= &mediatek_displaypwm_ops,
    .priv_auto	= sizeof(struct mediatek_displaypwm_priv),
};
