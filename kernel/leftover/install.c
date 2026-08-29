#include"leftover.h"
status leftover_install(void)
{
	char src[128];
	status ret=leftover_src(src,sizeof(src));
	if(ret)
		return(STATUS_OK);
	if((ret=leftover_copy_boot(src)))
		return(ret);
	if((ret=leftover_grub(0U)))
		return(ret);
	return(leftover_restart(0U));
}
