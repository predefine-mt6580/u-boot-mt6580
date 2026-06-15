// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Pavel Ivanov <predefine@yandex.ru>
 */

#ifndef _MTK_VIDEO_COMMON_H_
#define _MTK_VIDEO_COMMON_H_

int mtk_video_common_get_mmsys(struct udevice *dev, struct udevice **devp);
int mtk_video_common_get_panel(struct udevice *dev, struct udevice **devp);

// video_bridge ops
int mtk_video_common_attach(struct udevice *dev);
int mtk_video_common_get_display_timing(struct udevice *dev,
                                        struct display_timing *timings);

#endif
