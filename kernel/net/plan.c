#include"net.h"
#include"util.h"
status net_plan_from_card(net_plan*plan,const net_card*card)
{
	*plan=(net_plan){0};
	status ret;
	if((ret=net_card_validate(card)))
		return(ret);
	plan->card_id=card->id;
	net_text_copy(plan->name,card->name,NET_NAME_BYTES);
	plan->flags=((card->flags&NET_CARD_ADMIN_UP)?NET_PLAN_ADMIN_UP:0U)|
		((card->flags&NET_CARD_LINK_UP)?NET_PLAN_LINK_UP:0U)|
		((card->flags&NET_CARD_ADDR4)?NET_PLAN_ADDR4:0U)|
		((card->flags&NET_CARD_GATEWAY4)?NET_PLAN_GATEWAY4:0U)|
		((card->flags&NET_CARD_ADDR6)?NET_PLAN_ADDR6:0U)|
		((card->flags&NET_CARD_GATEWAY6)?NET_PLAN_GATEWAY6:0U);
	plan->addr4=card->addr4;
	plan->gateway4=card->gateway4;
	plan->addr6=card->addr6;
	plan->gateway6=card->gateway6;
	return(net_plan_validate(plan));
}
status net_plan_validate(const net_plan*plan)
{
	if(!plan->card_id||
		net_flags_without(plan->flags,NET_PLAN_ADDR4|NET_PLAN_ADDR6|NET_PLAN_GATEWAY4|NET_PLAN_GATEWAY6|NET_PLAN_CARD_ID|NET_PLAN_ADMIN_UP|NET_PLAN_LINK_UP)||
		net_text_valid(plan->name,NET_NAME_BYTES,1U)||
		((plan->flags&NET_PLAN_GATEWAY4)&&!(plan->flags&NET_PLAN_ADDR4))||
		((plan->flags&NET_PLAN_GATEWAY6)&&!(plan->flags&NET_PLAN_ADDR6))||
		((plan->flags&NET_PLAN_GATEWAY4)&&!net_bytes_any(plan->gateway4.bytes,NET_ADDR4_BYTES))||
		((plan->flags&NET_PLAN_GATEWAY6)&&!net_bytes_any(plan->gateway6.bytes,NET_ADDR6_BYTES))||
		((plan->flags&NET_PLAN_ADDR4)&&net_addr4_valid(&plan->addr4))||
		((plan->flags&NET_PLAN_ADDR6)&&net_addr6_valid(&plan->addr6)))
		return(ERR_INVAL);
	return(plan->flags&(NET_PLAN_ADDR4|NET_PLAN_ADDR6)?STATUS_OK:ERR_STATE);
}
