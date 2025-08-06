#ifndef __DT_BINDINGS_VIDEO_MTK_H__
#define __DT_BINDINGS_VIDEO_MTK_H__

#define DISP_PATH(what, to) DISP_PATH_##what DISP_PATH_##what##_##to

#define DISP_PATH_OVL0_MOUT   0
#define DISP_PATH_COLOR0_IN   1
#define DISP_PATH_DITHER_MOUT 2
#define DISP_PATH_RDMA0_OUT   3
#define DISP_PATH_DSI0_IN     4
#define DISP_PATH_NR 5

#endif
