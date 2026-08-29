#include<linux/err.h>
#include<linux/file.h>
#include<linux/fs.h>
#include"leftover.h"
static status leftover_write(const char*dst,const char*src)
{
	struct file*in,*out;
	u8 buf[4096];
	loff_t rpos=0,wpos=0;
	u8 any=0U;
	in=filp_open(src,O_RDONLY|O_LARGEFILE|O_NOFOLLOW,0);
	if(IS_ERR(in))
		return(ERR_PLATFORM);
	out=filp_open(dst,O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE|O_NOFOLLOW,0600);
	if(IS_ERR(out)){
		fput(in);
		return(ERR_PLATFORM);
	}
	for(;;){
		ssize_t got=kernel_read(in,buf,sizeof(buf),&rpos);
		if(got<=0){
			fput(in),fput(out);
			return(got?ERR_PLATFORM:(any?STATUS_OK:ERR_STATE));
		}
		if(kernel_write(out,buf,(size_t)got,&wpos)!=got){
			fput(in),fput(out);
			return(ERR_PLATFORM);
		}
		any=1U;
	}
}
status leftover_copy_boot(const char*src)
{
	return(leftover_write("/boot/upgrades.vmlinuz",src));
}
