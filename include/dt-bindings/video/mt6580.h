#ifndef __DT_BINDINGS_VIDEO_MT6580_H__
#define __DT_BINDINGS_VIDEO_MT6580_H__

#include "mtk.h"

// display mutex
#define MTK_DISP_MUTEX_OVL0 6
#define MTK_DISP_MUTEX_RDMA0 8

// display paths
#define DISP_PATH_OVL0_MOUT_COLOR0 (1 << 0)
#define DISP_PATH_COLOR0_IN_OVL0 1
#define DISP_PATH_DITHER_MOUT_RDMA0 (1 << 0)
#define DISP_PATH_RDMA0_OUT_DSI0 2
#define DISP_PATH_DSI0_IN_RDMA0 1

#endif
