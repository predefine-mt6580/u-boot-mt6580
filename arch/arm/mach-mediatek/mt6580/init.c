#include <config.h>
#include <init.h>
#include <asm/global_data.h>
//#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/arch/misc.h>

DECLARE_GLOBAL_DATA_PTR;

/*int dram_init(void)
{
  int ret;

  ret = fdtdec_setup_memory_banksize();
  if (ret)
    return ret;

  return fdtdec_setup_mem_size_base();
}*/

int dram_init(void)
{
  // HACK: u-boot crashes before probing driver after allocating ram for framebuffer
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, SZ_1G) - 0x02000000;
	return 0;
}

int print_cpuinfo(void)
{
	printf("SoC:   MediaTek MT6580\n");
	return 0;
}
