// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Pavel Ivanov <predefine@yandex.ru>
 */

#include <dm.h>
#include <video_bridge.h>
#include <panel.h>
#include "common.h"

#if CONFIG_IS_ENABLED(VIDEO_MTK_MUTEX)
#include "mutex.h"
#endif

int mtk_video_common_get_mmsys(struct udevice *dev, struct udevice **devp)
{
  int ret = 0;
  struct udevice *bridge = dev;

  while (ret != -ENODEV)
  {
    ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, bridge, 0, -1, &bridge);
    if (ret < 0 && ret != -ENODEV)
      return ret;
  }

  return uclass_get_device_by_endpoint(UCLASS_VIDEO, bridge, 0, -1, devp);
}

int mtk_video_common_get_panel(struct udevice *dev, struct udevice **devp)
{
  int ret = 0;
  int i = 0;
  struct udevice *bridge = dev;

  while (ret != -ENODEV)
  {
    if (device_get_uclass_id(bridge) == UCLASS_VIDEO)
      ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, bridge, 0, -1, &bridge);
    else
      ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, bridge, 1, -1, &bridge);

    if (ret < 0 && ret != -ENODEV)
      return ret;
    i++;
  }

  return uclass_get_device_by_endpoint(UCLASS_PANEL, bridge, 1, -1, devp);
}

#if CONFIG_IS_ENABLED(VIDEO_MTK_MUTEX)
static int mtk_video_common_setup_mutex(struct udevice *dev)
{
  struct ofnode_phandle_args args;
  struct udevice *mutex;
  int ret;

  ret = dev_read_phandle_with_args(dev, "mediatek,mutex", "#mutex-cells",
          0, 0, &args);
  if (ret == -ENOENT)
    return 0;
  if (ret < 0)
    return ret;

  ret = uclass_get_device_by_ofnode(UCLASS_NOP, args.node, &mutex);
  if (ret < 0)
    return ret;

  // DONT-PUSH: auto detect video and cmd mode for panel
  return mtk_mutex_setup(mutex, MUTEX_MODE_DSI0, args.args[0], args.args[1]);
}
#endif

int mtk_video_common_attach(struct udevice *dev)
{
  struct udevice *next_bridge;
  int ret = 0;

  ret = uclass_get_device_by_endpoint(UCLASS_VIDEO_BRIDGE, dev, 1, -1, &next_bridge);
  if (ret < 0 && ret != -ENODEV)
    return ret;

  if(ret == -ENODEV)
  {
    ret = uclass_get_device_by_endpoint(UCLASS_PANEL, dev, 1, -1, &next_bridge);
    if (ret == -ENODEV)
      return ret;
    ret = panel_enable_backlight(next_bridge);
  } else
  {
    ret = video_bridge_attach(next_bridge);
  }
  if (ret < 0 && ret != -ENOSYS)
    return ret;

  return mtk_video_common_setup_mutex(dev);
}

int mtk_video_common_get_display_timing(struct udevice *dev,
                                        struct display_timing *timings)
{
  struct udevice *panel;
  int ret = mtk_video_common_get_panel(dev, &panel);
  if (ret)
    return ret;
  return panel_get_display_timing(panel, timings);
}
