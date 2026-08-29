#include<linux/init.h>
#include"leftover.h"
static int adapter_init(void)
{
	(void)leftover_install();
	return(0);
}
late_initcall(adapter_init);
