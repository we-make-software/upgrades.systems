#include"net.h"
#include"util.h"
void net_card_clear(net_card*card)
{
	if(card)
		*card=(net_card){0};
}
status net_card_validate(const net_card*card)
{
	if(!card||!card->id)
		return(ERR_INVAL);
	if(!(card->flags&NET_CARD_PRESENT))
		return(ERR_STATE);
	status ret;
	if((ret=net_text_valid(card->name,NET_NAME_BYTES,1U))||
		(ret=net_text_valid(card->driver,NET_DRIVER_BYTES,0U))||
		(ret=net_text_valid(card->bus,NET_BUS_BYTES,0U)))
		return(ret);
	return(((card->flags&NET_CARD_GATEWAY4)&&!(card->flags&NET_CARD_ADDR4))||
		((card->flags&NET_CARD_GATEWAY6)&&!(card->flags&NET_CARD_ADDR6))||
		((card->flags&NET_CARD_GATEWAY4)&&!net_bytes_any(card->gateway4.bytes,NET_ADDR4_BYTES))||
		((card->flags&NET_CARD_GATEWAY6)&&!net_bytes_any(card->gateway6.bytes,NET_ADDR6_BYTES))||
		((card->flags&NET_CARD_ADDR4)&&net_addr4_valid(&card->addr4))||
		((card->flags&NET_CARD_ADDR6)&&net_addr6_valid(&card->addr6))?
		ERR_INVAL:STATUS_OK);
}