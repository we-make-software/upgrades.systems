#include<linux/err.h>
#include<linux/file.h>
#include<linux/fs.h>
#include"leftover.h"
static const char root[]="/var/lib/upgrades/kernel-build/";
static status leftover_live(const char*path)
{
	struct file*file=filp_open(path,O_RDONLY|O_LARGEFILE|O_NOFOLLOW,0);
	if(IS_ERR(file))
		return(ERR_PLATFORM);
	fput(file);
	return(STATUS_OK);
}
static status leftover_join(char*out,u32 max,const char*dir,const char*name)
{
	u32 n=0U,i=0U;
	if(!out||!dir||!name)
		return(ERR_INVAL);
	for(;dir[n];n++){
		if(n+1U>=max)
			return(ERR_INVAL);
		out[n]=dir[n];
	}
	for(;name[i];i++){
		if(n+1U>=max)
			return(ERR_INVAL);
		out[n++]=name[i];
	}
	out[n]=0;
	return(STATUS_OK);
}
status leftover_src(char*out,u32 max)
{
	static const char*names[]={"vmlinuz","bzImage"};
	u32 i=0U;
	if(!out)
		return(ERR_INVAL);
	for(;i<2U;i++){
		if(leftover_join(out,max,root,names[i]))
			return(ERR_INVAL);
		if(!leftover_live(out))
			return(STATUS_OK);
	}
	return(ERR_PLATFORM);
}
