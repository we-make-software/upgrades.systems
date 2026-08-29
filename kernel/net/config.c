#include"net.h"
#include"util.h"
status net_config_validate(const net_config*config)
{
	if(net_flags_without(config->flags,NET_PLAN_CARD_ID|NET_PLAN_ADMIN_UP|NET_PLAN_LINK_UP|NET_PLAN_ADDR4|NET_PLAN_ADDR6|NET_PLAN_GATEWAY4|NET_PLAN_GATEWAY6)||((config->flags&NET_PLAN_CARD_ID)&&!config->card_id)||(!(config->flags&NET_PLAN_CARD_ID)&&!config->name[0])||(config->name[0]&&net_text_valid(config->name,NET_NAME_BYTES,1U))||((config->flags&NET_PLAN_GATEWAY4)&&!(config->flags&NET_PLAN_ADDR4))||((config->flags&NET_PLAN_GATEWAY6)&&!(config->flags&NET_PLAN_ADDR6))||((config->flags&NET_PLAN_GATEWAY4)&&!net_bytes_any(config->gateway4.bytes,NET_ADDR4_BYTES))||((config->flags&NET_PLAN_GATEWAY6)&&!net_bytes_any(config->gateway6.bytes,NET_ADDR6_BYTES))||((config->flags&NET_PLAN_ADDR4)&&net_addr4_valid(&config->addr4))||((config->flags&NET_PLAN_ADDR6)&&net_addr6_valid(&config->addr6)))
		return(ERR_INVAL);
	return(config->flags&(NET_PLAN_ADDR4|NET_PLAN_ADDR6)?STATUS_OK:ERR_STATE);
}
status net_config_plan(net_plan*plan,const net_config*config,const net_card*card)
{
	*plan=(net_plan){0}; status ret;
	if((ret=net_config_validate(config))||(ret=net_card_validate(card))) return(ret);
	if(((config->flags&NET_PLAN_CARD_ID)&&config->card_id!=card->id)||(config->name[0]&&!net_text_same(config->name,card->name,NET_NAME_BYTES))) return(ERR_STATE);
	plan->card_id=card->id; net_text_copy(plan->name,card->name,NET_NAME_BYTES);
	plan->flags=net_flags_without(config->flags,NET_PLAN_CARD_ID);
	plan->addr4=config->addr4;plan->gateway4=config->gateway4;plan->addr6=config->addr6;plan->gateway6=config->gateway6;
	return(net_plan_validate(plan));
}