#include<linux/reboot.h>
#include"leftover.h"
status leftover_restart(u32 build)
{
	(void)build;
	kernel_restart(0);
	return(ERR_PLATFORM);
}
