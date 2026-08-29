#include"util.h"
status net_text_valid(const char*text,u32 size,u32 required)
{
	if(!size||(required&&!text[0]))
		return(ERR_INVAL);
	for(u32 i=0U;i<size;i++)
		if(!text[i])
			return(STATUS_OK);
	return(ERR_INVAL);
}
status net_text_same(const char*a,const char*b,u32 size)
{
	for(u32 i=0U;i<size;i++){
		if(a[i]!=b[i])
			return(0);
		if(!a[i])
			return(1);
	}
	return(1);
}
void net_text_copy(char*dst,const char*src,u32 size)
{
	u32 i=0U;
	if(!size)
		return;
	for(;i+1U<size&&src[i];i++)
		dst[i]=src[i];
	dst[i]=0;
}
status net_addr4_valid(const net_addr4*addr)
{
	return(addr->prefix>32U?ERR_INVAL:STATUS_OK);
}
status net_addr6_valid(const net_addr6*addr)
{
	return(addr->prefix>128U?ERR_INVAL:STATUS_OK);
}
status net_bytes_same(const u8*a,const u8*b,u32 size)
{
	for(u32 i=0U;i<size;i++)
		if(a[i]!=b[i])
			return(0);
	return(1);
}
u32 net_bytes_any(const u8*bytes,u32 size)
{
	for(u32 i=0U;i<size;i++)
		if(bytes[i])
			return(1U);
	return(0U);
}