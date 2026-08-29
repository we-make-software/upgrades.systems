#include<linux/init.h>
#include"main.h"
#include"net.h"
static status core_lock(core_ctx*core)
{
	return(__atomic_exchange_n(&core->changing,1,__ATOMIC_ACQUIRE)?ERR_STATE:STATUS_OK);
}
static void core_unlock(core_ctx*core)
{
	__atomic_store_n(&core->changing,0,__ATOMIC_RELEASE);
}
static void core_unwind(core_unit*unit)
{
	for(;unit;unit=unit->prev)
		if(unit->active){
			if(unit->leave)
				unit->leave(unit->context);
			unit->active=0;
		}
}
static u8 core_stoppable(const core_ctx*core)
{
	for(const core_unit*unit=core->first;unit;unit=unit->next)
		if(unit->active&&!unit->leave)
			return(0);
	return(1);
}
static status core_enter_unit(core_unit*unit)
{
	unit->result=unit->enter(unit->context);
	return(unit->result&&unit->required?unit->result:(unit->active=(u8)!unit->result,STATUS_OK));
}
status core_init(core_ctx*core,const core_config*config)
{
	status ret=!core||!config||!config->ops||!config->ops->log?ERR_INVAL:core_lock(core);
	if(ret)
		return(ret);
	if(core->state){
		core_unlock(core);
		return(ERR_STATE);
	}
	core->ops=config->ops;
	core->first=0;
	core->last=0;
	core->failed=0;
	core->failed_result=STATUS_OK;
	core->state=CORE_READY;
	core->ops->log("core ready");
	core_unlock(core);
	return(STATUS_OK);
}
status core_unit_add(core_ctx*core,core_unit*unit)
{
	status ret=!core||!unit||!unit->enter?ERR_INVAL:core_lock(core);
	if(ret)
		return(ret);
	if((core->state!=CORE_READY&&core->state!=CORE_RUNNING)||unit->owner||unit->prev||unit->next){
		core_unlock(core);
		return(ERR_STATE);
	}
	unit->result=ERR_STATE;
	unit->active=0;
	unit->prev=core->last;
	unit->owner=core;
	if(core->last)
		core->last->next=unit;
	else
		core->first=unit;
	core->last=unit;
	core_unlock(core);
	return(STATUS_OK);
}
status core_start(core_ctx*core)
{
	status ret=core?core_lock(core):ERR_INVAL;
	if(ret)
		return(ret);
	if(core->state!=CORE_READY){
		core_unlock(core);
		return(ERR_STATE);
	}
	core->state=CORE_STARTING;
	core->failed=0;
	core->failed_result=STATUS_OK;
	for(core_unit*unit=core->first;unit;unit=unit->next){
		ret=core_enter_unit(unit);
		if(ret){
			core->failed=unit;
			core->failed_result=ret;
			core_unwind(unit->prev);
			core->state=CORE_READY;
			core_unlock(core);
			return(ret);
		}
	}
	core->state=CORE_RUNNING;
	core->ops->log("core running");
	core_unlock(core);
	return(STATUS_OK);
}
status core_stop(core_ctx*core)
{
	status ret=core?core_lock(core):ERR_INVAL;
	if(ret)
		return(ret);
	if(core->state!=CORE_RUNNING||!core_stoppable(core)){
		core_unlock(core);
		return(ERR_STATE);
	}
	core->state=CORE_STOPPING;
	core_unwind(core->last);
	core->state=CORE_READY;
	core->ops->log("core stopped");
	core_unlock(core);
	return(STATUS_OK);
}
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
static core_unit net_unit={.enter=net_enter,.required=1};
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
