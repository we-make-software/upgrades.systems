#include"net.h"
static u8 flag_bit(u8 flags,u8 mask){return((u8)((flags&mask)!=0U));}
static void count_flag(u16*count,u8 flags,u8 mask){*count=(u16)(*count+flag_bit(flags,mask));}
static void count_ready(u16*count,u8 flags,u8 mask){*count=(u16)(*count+(u8)((flags&mask)==mask));}
static u8 card_score(const net_card*card)
{
	return((u8)(flag_bit(card->flags,NET_CARD_ADMIN_UP)+flag_bit(card->flags,NET_CARD_LINK_UP)+flag_bit(card->flags,NET_CARD_ADDR4)+flag_bit(card->flags,NET_CARD_ADDR6)+flag_bit(card->flags,NET_CARD_GATEWAY4)+flag_bit(card->flags,NET_CARD_GATEWAY6)));
}
void net_discovery_begin(net_discovery*scan){*scan=(net_discovery){0};}
status net_discovery_card(net_discovery*scan,const net_card*card)
{
	status ret;
	if((ret=net_card_validate(card))){scan->invalid++;return(ret);}
	scan->cards++;
	count_flag(&scan->admin_up,card->flags,NET_CARD_ADMIN_UP);
	count_flag(&scan->link_up,card->flags,NET_CARD_LINK_UP);
	count_flag(&scan->addr4,card->flags,NET_CARD_ADDR4);
	count_flag(&scan->addr6,card->flags,NET_CARD_ADDR6);
	count_flag(&scan->gateway4,card->flags,NET_CARD_GATEWAY4);
	count_flag(&scan->gateway6,card->flags,NET_CARD_GATEWAY6);
	count_ready(&scan->ready4,card->flags,NET_CARD_ADMIN_UP|NET_CARD_LINK_UP|NET_CARD_ADDR4|NET_CARD_GATEWAY4);
	count_ready(&scan->ready6,card->flags,NET_CARD_ADMIN_UP|NET_CARD_LINK_UP|NET_CARD_ADDR6|NET_CARD_GATEWAY6);
	u8 score=card_score(card);
	if(score>scan->primary_score||!scan->primary.id){scan->primary=*card;scan->primary_score=score;}
	return(STATUS_OK);
}
status net_discovery_end(const net_discovery*scan){return(scan->cards&&(scan->primary.flags&(NET_CARD_ADDR4|NET_CARD_ADDR6))?STATUS_OK:ERR_STATE);}