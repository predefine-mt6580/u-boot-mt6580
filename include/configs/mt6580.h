// SPDX-License-Identifier: GPL-2.0+

#ifndef __MT6580_H
#define __MT6580_H

#include <linux/sizes.h>
#define DEBUG

#define CFG_SYS_INIT_RAM_ADDR    0x100000
#define CFG_SYS_INIT_RAM_SIZE    0x10000

#define CFG_SYS_UBOOT_BASE 0x81e00000
#define CONFIG_SPL_BSS_START_ADDR 0x80800000

#define CFG_SYS_SDRAM_BASE 0x80000000

#endif
