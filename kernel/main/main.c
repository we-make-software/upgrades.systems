#include<linux/init.h>
#include"main.h"
#include"net.h"
static void main_log(const char*message)
{
	(void)message;
}
static status net_enter(void*context)
{
	(void)context;
	return(net_start());
}
static void net_leave(void*context)
{
	(void)context;
}
static core_ctx core;
static core_unit net_unit={.enter=net_enter,.leave=net_leave,.required=1U};
static int main_init(void)
{
	static const core_ops ops={.log=main_log};
	static const core_config config={.ops=&ops};
	status ret=core_init(&core,&config);
	if(ret||(ret=core_check())||(ret=net_check())||(ret=core_unit_add(&core,&net_unit)))
		return(0);
	(void)core_start(&core);
	return(0);
}
late_initcall(main_init);