#include"net.h"
#include"util.h"
static status match_bits(u8*missing,const net_card*card,const net_plan*plan,u8 flags)
{
	if(card->id!=plan->card_id)
		*missing|=NET_PLAN_CARD_ID;
	if((flags&NET_PLAN_ADMIN_UP)&&!(card->flags&NET_CARD_ADMIN_UP))
		*missing|=NET_PLAN_ADMIN_UP;
	if((flags&NET_PLAN_LINK_UP)&&!(card->flags&NET_CARD_LINK_UP))
		*missing|=NET_PLAN_LINK_UP;
	if((flags&NET_PLAN_ADDR4)&&(!(card->flags&NET_CARD_ADDR4)||card->addr4.prefix!=plan->addr4.prefix||!net_bytes_same(card->addr4.ip.bytes,plan->addr4.ip.bytes,NET_ADDR4_BYTES)))
		*missing|=NET_PLAN_ADDR4;
	if((flags&NET_PLAN_ADDR6)&&(!(card->flags&NET_CARD_ADDR6)||card->addr6.prefix!=plan->addr6.prefix||!net_bytes_same(card->addr6.ip.bytes,plan->addr6.ip.bytes,NET_ADDR6_BYTES)))
		*missing|=NET_PLAN_ADDR6;
	if((flags&NET_PLAN_GATEWAY4)&&(!(card->flags&NET_CARD_GATEWAY4)||!net_bytes_same(card->gateway4.bytes,plan->gateway4.bytes,NET_ADDR4_BYTES)))
		*missing|=NET_PLAN_GATEWAY4;
	if((flags&NET_PLAN_GATEWAY6)&&(!(card->flags&NET_CARD_GATEWAY6)||!net_bytes_same(card->gateway6.bytes,plan->gateway6.bytes,NET_ADDR6_BYTES)))
		*missing|=NET_PLAN_GATEWAY6;
	return(*missing?ERR_STATE:STATUS_OK);
}
u8 net_plan_required_flags(const net_plan*plan)
{
	return(NET_PLAN_ADMIN_UP|NET_PLAN_LINK_UP|((plan->flags&NET_PLAN_ADDR4)&&((plan->flags&(NET_PLAN_ADDR6|NET_PLAN_GATEWAY6))!=(NET_PLAN_ADDR6|NET_PLAN_GATEWAY6))?NET_PLAN_GATEWAY4:0U)|((plan->flags&NET_PLAN_ADDR6)&&((plan->flags&(NET_PLAN_ADDR4|NET_PLAN_GATEWAY4))!=(NET_PLAN_ADDR4|NET_PLAN_GATEWAY4))?NET_PLAN_GATEWAY6:0U));
}
status net_plan_from_card(net_plan*plan,const net_card*card)
{
	*plan=(net_plan){0};
	status ret;
	if((ret=net_card_validate(card)))
		return(ret);
	plan->card_id=card->id;
	net_text_copy(plan->name,card->name,NET_NAME_BYTES);
	plan->flags=((card->flags&NET_CARD_ADMIN_UP)?NET_PLAN_ADMIN_UP:0U)|((card->flags&NET_CARD_LINK_UP)?NET_PLAN_LINK_UP:0U)|((card->flags&NET_CARD_ADDR4)?NET_PLAN_ADDR4:0U)|((card->flags&NET_CARD_GATEWAY4)?NET_PLAN_GATEWAY4:0U)|((card->flags&NET_CARD_ADDR6)?NET_PLAN_ADDR6:0U)|((card->flags&NET_CARD_GATEWAY6)?NET_PLAN_GATEWAY6:0U);
	plan->addr4=card->addr4;plan->gateway4=card->gateway4;plan->addr6=card->addr6;plan->gateway6=card->gateway6;
	return(net_plan_validate(plan));
}
status net_plan_validate(const net_plan*plan)
{
	if(!plan->card_id||net_flags_without(plan->flags,NET_PLAN_ADDR4|NET_PLAN_ADDR6|NET_PLAN_GATEWAY4|NET_PLAN_GATEWAY6|NET_PLAN_CARD_ID|NET_PLAN_ADMIN_UP|NET_PLAN_LINK_UP)||net_text_valid(plan->name,NET_NAME_BYTES,1U)||((plan->flags&NET_PLAN_GATEWAY4)&&!(plan->flags&NET_PLAN_ADDR4))||((plan->flags&NET_PLAN_GATEWAY6)&&!(plan->flags&NET_PLAN_ADDR6))||((plan->flags&NET_PLAN_GATEWAY4)&&!net_bytes_any(plan->gateway4.bytes,NET_ADDR4_BYTES))||((plan->flags&NET_PLAN_GATEWAY6)&&!net_bytes_any(plan->gateway6.bytes,NET_ADDR6_BYTES))||((plan->flags&NET_PLAN_ADDR4)&&net_addr4_valid(&plan->addr4))||((plan->flags&NET_PLAN_ADDR6)&&net_addr6_valid(&plan->addr6)))
		return(ERR_INVAL);
	return(plan->flags&(NET_PLAN_ADDR4|NET_PLAN_ADDR6)?STATUS_OK:ERR_STATE);
}
status net_plan_match(u8*missing,const net_card*card,const net_plan*plan)
{
	*missing=0U;
	status ret;
	return((ret=net_card_validate(card))||(ret=net_plan_validate(plan))?ret:match_bits(missing,card,plan,plan->flags));
}
status net_plan_preflight(u8*missing,const net_card*card,const net_plan*plan)
{
	*missing=0U;
	status ret;
	if((ret=net_card_validate(card))||(ret=net_plan_validate(plan)))
		return(ret);
	u8 required=net_plan_required_flags(plan);
	*missing=required&~plan->flags;
	return(match_bits(missing,card,plan,plan->flags|required));
}