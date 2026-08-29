#include<linux/init.h>
#include"core.h"
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
static core_ctx core;
static core_unit net_unit={.enter=net_enter,.required=1U};
static int main_init(void)
{
	static const core_ops ops={.log=main_log};
	static const core_config config={.ops=&ops};
	status ret=core_init(&core,&config);
	if(ret||(ret=core_unit_add(&core,&net_unit)))
		return(0);
	(void)core_start(&core);
	return(0);
}
late_initcall(main_init);
