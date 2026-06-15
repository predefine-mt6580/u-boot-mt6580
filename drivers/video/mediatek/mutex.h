#ifndef _MTK_VIDEO_MUTEX_H_
#define _MTK_VIDEO_MUTEX_H_

enum {
  MUTEX_MODE_SINGLE = 0,
  MUTEX_MODE_DSI0,
  MUTEX_MODE_DSI1,
  MUTEX_MODE_DPI0,
};

int mtk_mutex_setup(struct udevice *dev, int mode, int mutex_id, int mutex_device);

#endif
