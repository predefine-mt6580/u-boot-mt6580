#include <dm.h>
#include <mipi_dsi.h>
#include <panel.h>
#include <power/regulator.h>
#include <asm/gpio.h>
#include <linux/delay.h>

struct k710_hz_jd9366_boe_wxga_ips_101_priv {
  struct udevice *power;

  struct gpio_desc gpio_enn;
  struct gpio_desc gpio_enp;
  struct gpio_desc gpio_rst;
};

static struct display_timing default_timing = {
    .pixelclock.typ   = 70000000,
    .hactive.typ      = 800,
    .hfront_porch.typ = 18,
    .hback_porch.typ  = 18,
    .hsync_len.typ    = 18,
    .vactive.typ      = 1280,
    .vfront_porch.typ = 24,
    .vback_porch.typ  = 8,
    .vsync_len.typ    = 4,
};

static void dcs_write_one(struct mipi_dsi_device *dsi, u8 cmd, u8 data)
{
    mipi_dsi_dcs_write(dsi, cmd, &data, 1);
    mdelay(1);
}

static int k710_hz_jd9366_boe_wxga_ips_101_enable_backlight(struct udevice *dev)
{
  struct k710_hz_jd9366_boe_wxga_ips_101_priv *priv = dev_get_priv(dev);
  struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
  struct mipi_dsi_device *dsi = plat->device;
  int ret;

  dcs_write_one(dsi, 0xE0, 0x00);
  dcs_write_one(dsi, 0xE1, 0x93);
  dcs_write_one(dsi, 0xE2, 0x65);
  dcs_write_one(dsi, 0xE3, 0xF8);
  dcs_write_one(dsi, 0x80, 0x02);
  dcs_write_one(dsi, 0xE0, 0x01);
  dcs_write_one(dsi, 0x00, 0x00);
  dcs_write_one(dsi, 0x01, 0x66);
  dcs_write_one(dsi, 0x0E, 0x01);
  dcs_write_one(dsi, 0x17, 0x00);
  dcs_write_one(dsi, 0x18, 0xBF);
  dcs_write_one(dsi, 0x19, 0x00);
  dcs_write_one(dsi, 0x1A, 0x00);
  dcs_write_one(dsi, 0x1B, 0xBF);
  dcs_write_one(dsi, 0x1C, 0x00);
  dcs_write_one(dsi, 0x1F, 0x3E);
  dcs_write_one(dsi, 0x20, 0x28);
  dcs_write_one(dsi, 0x21, 0x28);
  dcs_write_one(dsi, 0x22, 0x0E);
  dcs_write_one(dsi, 0x37, 0x09);
  dcs_write_one(dsi, 0x38, 0x04);
  dcs_write_one(dsi, 0x39, 0x08);
  dcs_write_one(dsi, 0x3A, 0x12);
  dcs_write_one(dsi, 0x3C, 0x78);
  dcs_write_one(dsi, 0x3D, 0xFF);
  dcs_write_one(dsi, 0x3E, 0xFF);
  dcs_write_one(dsi, 0x3F, 0x7F);
  dcs_write_one(dsi, 0x40, 0x06);
  dcs_write_one(dsi, 0x41, 0xA0);
  dcs_write_one(dsi, 0x55, 0x01);
  dcs_write_one(dsi, 0x56, 0x01);
  dcs_write_one(dsi, 0x57, 0x69);
  dcs_write_one(dsi, 0x58, 0x0A);
  dcs_write_one(dsi, 0x59, 0x0A);
  dcs_write_one(dsi, 0x5A, 0x29);
  dcs_write_one(dsi, 0x5B, 0x15);
  dcs_write_one(dsi, 0x5D, 0x7C);
  dcs_write_one(dsi, 0x5E, 0x65);
  dcs_write_one(dsi, 0x5F, 0x55);
  dcs_write_one(dsi, 0x60, 0x49);
  dcs_write_one(dsi, 0x61, 0x44);
  dcs_write_one(dsi, 0x62, 0x35);
  dcs_write_one(dsi, 0x63, 0x3A);
  dcs_write_one(dsi, 0x64, 0x23);
  dcs_write_one(dsi, 0x65, 0x3D);
  dcs_write_one(dsi, 0x66, 0x3C);
  dcs_write_one(dsi, 0x67, 0x3D);
  dcs_write_one(dsi, 0x68, 0x5D);
  dcs_write_one(dsi, 0x69, 0x4D);
  dcs_write_one(dsi, 0x6A, 0x56);
  dcs_write_one(dsi, 0x6B, 0x48);
  dcs_write_one(dsi, 0x6C, 0x45);
  dcs_write_one(dsi, 0x6D, 0x38);
  dcs_write_one(dsi, 0x6E, 0x25);
  dcs_write_one(dsi, 0x6F, 0x00);
  dcs_write_one(dsi, 0x70, 0x7C);
  dcs_write_one(dsi, 0x71, 0x65);
  dcs_write_one(dsi, 0x72, 0x55);
  dcs_write_one(dsi, 0x73, 0x49);
  dcs_write_one(dsi, 0x74, 0x44);
  dcs_write_one(dsi, 0x75, 0x35);
  dcs_write_one(dsi, 0x76, 0x3A);
  dcs_write_one(dsi, 0x77, 0x23);
  dcs_write_one(dsi, 0x78, 0x3D);
  dcs_write_one(dsi, 0x79, 0x3C);
  dcs_write_one(dsi, 0x7A, 0x3D);
  dcs_write_one(dsi, 0x7B, 0x5D);
  dcs_write_one(dsi, 0x7C, 0x4D);
  dcs_write_one(dsi, 0x7D, 0x56);
  dcs_write_one(dsi, 0x7E, 0x48);
  dcs_write_one(dsi, 0x7F, 0x45);
  dcs_write_one(dsi, 0x80, 0x38);
  dcs_write_one(dsi, 0x81, 0x25);
  dcs_write_one(dsi, 0x82, 0x00);
  dcs_write_one(dsi, 0xE0, 0x02);
  dcs_write_one(dsi, 0x00, 0x1E);
  dcs_write_one(dsi, 0x01, 0x1E);
  dcs_write_one(dsi, 0x02, 0x41);
  dcs_write_one(dsi, 0x03, 0x41);
  dcs_write_one(dsi, 0x04, 0x43);
  dcs_write_one(dsi, 0x05, 0x43);
  dcs_write_one(dsi, 0x06, 0x1F);
  dcs_write_one(dsi, 0x07, 0x1F);
  dcs_write_one(dsi, 0x08, 0x1F);
  dcs_write_one(dsi, 0x09, 0x1F);
  dcs_write_one(dsi, 0x0A, 0x1E);
  dcs_write_one(dsi, 0x0B, 0x1E);
  dcs_write_one(dsi, 0x0C, 0x1F);
  dcs_write_one(dsi, 0x0D, 0x47);
  dcs_write_one(dsi, 0x0E, 0x47);
  dcs_write_one(dsi, 0x0F, 0x45);
  dcs_write_one(dsi, 0x10, 0x45);
  dcs_write_one(dsi, 0x11, 0x4B);
  dcs_write_one(dsi, 0x12, 0x4B);
  dcs_write_one(dsi, 0x13, 0x49);
  dcs_write_one(dsi, 0x14, 0x49);
  dcs_write_one(dsi, 0x15, 0x1F);
  dcs_write_one(dsi, 0x16, 0x1E);
  dcs_write_one(dsi, 0x17, 0x1E);
  dcs_write_one(dsi, 0x18, 0x40);
  dcs_write_one(dsi, 0x19, 0x40);
  dcs_write_one(dsi, 0x1A, 0x42);
  dcs_write_one(dsi, 0x1B, 0x42);
  dcs_write_one(dsi, 0x1C, 0x1F);
  dcs_write_one(dsi, 0x1D, 0x1F);
  dcs_write_one(dsi, 0x1E, 0x1F);
  dcs_write_one(dsi, 0x1F, 0x1F);
  dcs_write_one(dsi, 0x20, 0x1E);
  dcs_write_one(dsi, 0x21, 0x1E);
  dcs_write_one(dsi, 0x22, 0x1F);
  dcs_write_one(dsi, 0x23, 0x46);
  dcs_write_one(dsi, 0x24, 0x46);
  dcs_write_one(dsi, 0x25, 0x44);
  dcs_write_one(dsi, 0x26, 0x44);
  dcs_write_one(dsi, 0x27, 0x4A);
  dcs_write_one(dsi, 0x28, 0x4A);
  dcs_write_one(dsi, 0x29, 0x48);
  dcs_write_one(dsi, 0x2A, 0x48);
  dcs_write_one(dsi, 0x2B, 0x1F);
  dcs_write_one(dsi, 0x58, 0x10);
  dcs_write_one(dsi, 0x59, 0x00);
  dcs_write_one(dsi, 0x5A, 0x00);
  dcs_write_one(dsi, 0x5B, 0x30);
  dcs_write_one(dsi, 0x5C, 0x02);
  dcs_write_one(dsi, 0x5D, 0x40);
  dcs_write_one(dsi, 0x5E, 0x01);
  dcs_write_one(dsi, 0x5F, 0x02);
  dcs_write_one(dsi, 0x60, 0x30);
  dcs_write_one(dsi, 0x61, 0x01);
  dcs_write_one(dsi, 0x62, 0x02);
  dcs_write_one(dsi, 0x63, 0x6A);
  dcs_write_one(dsi, 0x64, 0x6A);
  dcs_write_one(dsi, 0x65, 0x05);
  dcs_write_one(dsi, 0x66, 0x12);
  dcs_write_one(dsi, 0x67, 0x74);
  dcs_write_one(dsi, 0x68, 0x04);
  dcs_write_one(dsi, 0x69, 0x6A);
  dcs_write_one(dsi, 0x6A, 0x6A);
  dcs_write_one(dsi, 0x6B, 0x08);
  dcs_write_one(dsi, 0x6C, 0x00);
  dcs_write_one(dsi, 0x6D, 0x06);
  dcs_write_one(dsi, 0x6E, 0x00);
  dcs_write_one(dsi, 0x6F, 0x88);
  dcs_write_one(dsi, 0x70, 0x00);
  dcs_write_one(dsi, 0x71, 0x00);
  dcs_write_one(dsi, 0x72, 0x06);
  dcs_write_one(dsi, 0x73, 0x7B);
  dcs_write_one(dsi, 0x74, 0x00);
  dcs_write_one(dsi, 0x75, 0x07);
  dcs_write_one(dsi, 0x76, 0x00);
  dcs_write_one(dsi, 0x77, 0x5D);
  dcs_write_one(dsi, 0x78, 0x17);
  dcs_write_one(dsi, 0x79, 0x1F);
  dcs_write_one(dsi, 0x7A, 0x00);
  dcs_write_one(dsi, 0x7B, 0x00);
  dcs_write_one(dsi, 0x7C, 0x00);
  dcs_write_one(dsi, 0x7D, 0x03);
  dcs_write_one(dsi, 0x7E, 0x7B);
  dcs_write_one(dsi, 0xE0, 0x04);
  dcs_write_one(dsi, 0x2D, 0x03);
  dcs_write_one(dsi, 0x2E, 0x44);
  dcs_write_one(dsi, 0x09, 0x11);
  dcs_write_one(dsi, 0x2D, 0x03);
  dcs_write_one(dsi, 0xE0, 0x03);
  dcs_write_one(dsi, 0x98, 0x3F);
  dcs_write_one(dsi, 0xE0, 0x00);
  dcs_write_one(dsi, 0xE6, 0x02);
  dcs_write_one(dsi, 0xE7, 0x02);

  ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
  if (ret)
      return ret;

  mdelay(120);

  ret = mipi_dsi_dcs_set_display_on(dsi);
  if (ret)
      return ret;

  mdelay(20);
  mdelay(100);
  ret = dm_gpio_set_value(&priv->gpio_enp, 1);
    if (ret)
    return ret;


  return 0;
}

static int k710_hz_jd9366_boe_wxga_ips_101_timings(struct udevice *dev,
                                            struct display_timing *timing)
{
  memcpy(timing, &default_timing, sizeof(*timing));
  return 0;
}

static const struct panel_ops k710_hz_jd9366_boe_wxga_ips_101_ops = {
    .enable_backlight	= k710_hz_jd9366_boe_wxga_ips_101_enable_backlight,
    //.set_backlight		= k710_hz_jd9366_boe_wxga_ips_101_set_backlight,
    .get_display_timing	= k710_hz_jd9366_boe_wxga_ips_101_timings,
};

static int k710_hz_jd9366_boe_wxga_ips_101_hw_init(struct udevice *dev)
{
  struct k710_hz_jd9366_boe_wxga_ips_101_priv *priv = dev_get_priv(dev);
  int ret;

  ret = dm_gpio_set_value(&priv->gpio_enn, 1);
  if (ret)
    return ret;

  mdelay(50);

  ret = dm_gpio_set_value(&priv->gpio_enp, 1);
  if (ret)
    return ret;

  ret = regulator_set_value(priv->power, 1800000);
  if (ret)
    return ret;

  ret = regulator_set_enable_if_allowed(priv->power, 1);
  if (ret)
    return ret;

  mdelay(80);

  ret = dm_gpio_set_value(&priv->gpio_rst, 1);
  if (ret)
    return ret;

  mdelay(50);
  ret = dm_gpio_set_value(&priv->gpio_rst, 0);
  if (ret)
    return ret;

  mdelay(50);

  ret = dm_gpio_set_value(&priv->gpio_rst, 1);
  if (ret)
    return ret;


  mdelay(80);

  return 0;
}

static int k710_hz_jd9366_boe_wxga_ips_101_of_to_plat(struct udevice *dev)
{
  struct k710_hz_jd9366_boe_wxga_ips_101_priv *priv = dev_get_priv(dev);
  int ret;

  ret = device_get_supply_regulator(dev, "power-supply", &priv->power);
  if (ret)
    return ret;

  ret = gpio_request_by_name(dev, "enable-gpios", 0,
                  &priv->gpio_enn, GPIOD_IS_OUT);
  if (ret)
    return ret;

  ret = gpio_request_by_name(dev, "enable-gpios", 1,
                  &priv->gpio_enp, GPIOD_IS_OUT);
  if (ret)
    return ret;

  ret = gpio_request_by_name(dev, "reset-gpios", 0,
                  &priv->gpio_rst, GPIOD_IS_OUT);
  if (ret)
    return ret;

  return 0;
}

static int k710_hz_jd9366_boe_wxga_ips_101_probe(struct udevice *dev)
{
  struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);

  plat->lanes = 3;
  plat->format = MIPI_DSI_FMT_RGB888;
  plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE;

  return k710_hz_jd9366_boe_wxga_ips_101_hw_init(dev);
}

static const struct udevice_id k710_hz_jd9366_boe_wxga_ips_101_ids[] = {
  { .compatible = "k710_hz_jd9366_boe_wxga_ips_101" },
  { }
};

U_BOOT_DRIVER(k710_hz_jd9366_boe_wxga_ips_101) = {
  .name = "k710_hz_jd9366_boe_wxga_ips_101",
  .id = UCLASS_PANEL,
  .of_match = k710_hz_jd9366_boe_wxga_ips_101_ids,
  .ops = &k710_hz_jd9366_boe_wxga_ips_101_ops,
  .probe = k710_hz_jd9366_boe_wxga_ips_101_probe,
  .of_to_plat	= k710_hz_jd9366_boe_wxga_ips_101_of_to_plat,
  .plat_auto = sizeof(struct mipi_dsi_panel_plat),
  .priv_auto = sizeof(struct k710_hz_jd9366_boe_wxga_ips_101_priv),
};
