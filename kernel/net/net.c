#include<linux/inetdevice.h>
#include<linux/netdevice.h>
#include<linux/rtnetlink.h>
#include<net/addrconf.h>
#include<net/ipv6.h>
#include"net.h"
static inline u8 net_flags_without(u8 flags,u8 mask)
{
	return((u8)(flags&(u8)~mask));
}
static status net_text_valid(const char*text,u32 size,u32 required)
{
	if(!size||(required&&!text[0]))
		return(ERR_INVAL);
	for(u32 i=0U;i<size;i++)
		if(!text[i])
			return(STATUS_OK);
	return(ERR_INVAL);
}
static void net_text_copy(char*dst,const char*src,u32 size)
{
	u32 i=0U;
	if(!size)
		return;
	for(;i+1U<size&&src[i];i++)
		dst[i]=src[i];
	dst[i]=0;
}
static status net_addr4_valid(const net_addr4*addr)
{
	return(addr->prefix>32U?ERR_INVAL:STATUS_OK);
}
static status net_addr6_valid(const net_addr6*addr)
{
	return(addr->prefix>128U?ERR_INVAL:STATUS_OK);
}
static u32 net_bytes_any(const u8*bytes,u32 size)
{
	for(u32 i=0U;i<size;i++)
		if(bytes[i])
			return(1U);
	return(0U);
}
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
	plan->addr4=card->addr4,plan->gateway4=card->gateway4,plan->addr6=card->addr6,plan->gateway6=card->gateway6;
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
static u8 flag_bit(u8 flags,u8 mask)
{
	return((u8)((flags&mask)!=0U));
}
static void count_flag(u16*count,u8 flags,u8 mask)
{
	*count=(u16)(*count+flag_bit(flags,mask));
}
static void count_ready(u16*count,u8 flags,u8 mask)
{
	*count=(u16)(*count+(u8)((flags&mask)==mask));
}
static u8 card_score(const net_card*card)
{
	return((u8)(flag_bit(card->flags,NET_CARD_ADMIN_UP)+flag_bit(card->flags,NET_CARD_LINK_UP)+
		flag_bit(card->flags,NET_CARD_ADDR4)+flag_bit(card->flags,NET_CARD_ADDR6)+
		flag_bit(card->flags,NET_CARD_GATEWAY4)+flag_bit(card->flags,NET_CARD_GATEWAY6)));
}
void net_discovery_begin(net_discovery*scan)
{
	*scan=(net_discovery){0};
}
status net_discovery_card(net_discovery*scan,const net_card*card)
{
	status ret;
	if((ret=net_card_validate(card))){
		scan->invalid++;
		return(ret);
	}
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
	if(score>scan->primary_score||!scan->primary.id){
		scan->primary=*card;
		scan->primary_score=score;
	}
	return(STATUS_OK);
}
status net_discovery_end(const net_discovery*scan)
{
	return(scan->cards&&(scan->primary.flags&(NET_CARD_ADDR4|NET_CARD_ADDR6))?STATUS_OK:ERR_STATE);
}
static void fill_addr4(net_card*card,const struct net_device*dev)
{
	struct in_device*in_dev=__in_dev_get_rtnl(dev);
	if(!in_dev)
		return;
	const struct in_ifaddr*ifa=rtnl_dereference(in_dev->ifa_list);
	if(!ifa)
		return;
	const u8*bytes=(const u8*)&ifa->ifa_local;
	for(u8 i=0U;i<NET_ADDR4_BYTES;i++)
		card->addr4.ip.bytes[i]=bytes[i];
	card->addr4.prefix=ifa->ifa_prefixlen;
	card->flags|=NET_CARD_ADDR4;
}
static void fill_addr6(net_card*card,const struct net_device*dev)
{
	struct inet6_dev*idev=in6_dev_get(dev);
	struct inet6_ifaddr*ifa;
	if(!idev)
		return;
	read_lock_bh(&idev->lock);
	list_for_each_entry(ifa,&idev->addr_list,if_list){
		const u8*a=ifa->addr.s6_addr;
		if(ifa->flags&IFA_F_DADFAILED)
			continue;
		if(ipv6_addr_type(&ifa->addr)!=IPV6_ADDR_UNICAST||ipv6_addr_src_scope(&ifa->addr)!=IPV6_ADDR_SCOPE_GLOBAL)
			continue;
		for(u8 i=0U;i<NET_ADDR6_BYTES;i++)
			card->addr6.ip.bytes[i]=a[i];
		card->addr6.prefix=(u8)ifa->prefix_len;
		card->flags|=NET_CARD_ADDR6;
		break;
	}
	read_unlock_bh(&idev->lock);
	in6_dev_put(idev);
}
static void fill_card(net_card*card,const struct net_device*dev)
{
	net_card_clear(card);
	card->id=(u32)dev->ifindex;
	net_text_copy(card->name,dev->name?dev->name:"",NET_NAME_BYTES);
	card->flags=NET_CARD_PRESENT;
	if(dev->flags&IFF_UP)
		card->flags|=NET_CARD_ADMIN_UP;
	if(netif_carrier_ok(dev))
		card->flags|=NET_CARD_LINK_UP;
	if(dev->addr_len==NET_MAC_BYTES){
		for(u8 i=0U;i<NET_MAC_BYTES;i++)
			card->mac[i]=dev->dev_addr[i];
		card->flags|=NET_CARD_MAC;
	}
	const struct device*parent=dev->dev.parent;
	if(parent){
		net_text_copy(card->driver,dev_driver_string(parent),NET_DRIVER_BYTES);
		net_text_copy(card->bus,dev_name(parent),NET_BUS_BYTES);
	}
	fill_addr4(card,dev);
	fill_addr6(card,dev);
}
status net_scan(net_discovery*out)
{
	struct net_device*dev;
	net_discovery_begin(out);
	rtnl_lock();
	for_each_netdev(&init_net,dev){
		net_card card;
		fill_card(&card,dev);
		(void)net_discovery_card(out,&card);
	}
	rtnl_unlock();
	return(net_discovery_end(out));
}
status net_start(void)
{
	net_discovery scan;
	return(net_scan(&scan));
}
