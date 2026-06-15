// SPDX-License-Identifier: GPL-2.0

#include <fdtdec.h>
#include <stdio.h>
#include <asm/global_data.h>
// #include <asm/system.h>
// #include <linux/kernel.h>
// #include <linux/sizes.h>

#include "../cpu.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
    return fdtdec_setup_mem_size_base();
}

int print_cpuinfo(void)
{
  printf("CPU:   MediaTek MT6580\n");

  return 0;
}
