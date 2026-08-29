#include<linux/inetdevice.h>
#include<linux/netdevice.h>
#include<linux/rtnetlink.h>
#include<net/addrconf.h>
#include<net/ipv6.h>
#include"net.h"
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
	if(!card->present)
		return(ERR_STATE);
	status ret;
	if((ret=net_text_valid(card->name,sizeof(card->name),1U))||
		(ret=net_text_valid(card->driver,sizeof(card->driver),0U))||
		(ret=net_text_valid(card->bus,sizeof(card->bus),0U)))
		return(ret);
	if((card->gw4&&!card->has4)||(card->gw6&&!card->has6))
		return(ERR_INVAL);
	if((card->gw4&&!net_bytes_any(card->gateway4.bytes,sizeof(card->gateway4.bytes)))||
		(card->gw6&&!net_bytes_any(card->gateway6.bytes,sizeof(card->gateway6.bytes))))
		return(ERR_INVAL);
	if((card->has4&&net_addr4_valid(&card->addr4))||
		(card->has6&&net_addr6_valid(&card->addr6)))
		return(ERR_INVAL);
	return(STATUS_OK);
}
status net_plan_from_card(net_plan*plan,const net_card*card)
{
	status ret;
	*plan=(net_plan){0};
	if((ret=net_card_validate(card)))
		return(ret);
	plan->card_id=card->id;
	net_text_copy(plan->name,card->name,sizeof(plan->name));
	plan->admin=card->admin;
	plan->link=card->link;
	plan->has4=card->has4;
	plan->gw4=card->gw4;
	plan->has6=card->has6;
	plan->gw6=card->gw6;
	plan->addr4=card->addr4;
	plan->gateway4=card->gateway4;
	plan->addr6=card->addr6;
	plan->gateway6=card->gateway6;
	return(net_plan_validate(plan));
}
status net_plan_validate(const net_plan*plan)
{
	if(!plan->card_id)
		return(ERR_INVAL);
	if(net_text_valid(plan->name,sizeof(plan->name),1U))
		return(ERR_INVAL);
	if((plan->gw4&&!plan->has4)||(plan->gw6&&!plan->has6))
		return(ERR_INVAL);
	if((plan->gw4&&!net_bytes_any(plan->gateway4.bytes,sizeof(plan->gateway4.bytes)))||
		(plan->gw6&&!net_bytes_any(plan->gateway6.bytes,sizeof(plan->gateway6.bytes))))
		return(ERR_INVAL);
	if((plan->has4&&net_addr4_valid(&plan->addr4))||
		(plan->has6&&net_addr6_valid(&plan->addr6)))
		return(ERR_INVAL);
	return(plan->has4||plan->has6?STATUS_OK:ERR_STATE);
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
	scan->admin_up=(u16)(scan->admin_up+card->admin);
	scan->link_up=(u16)(scan->link_up+card->link);
	scan->addr4=(u16)(scan->addr4+card->has4);
	scan->addr6=(u16)(scan->addr6+card->has6);
	scan->gateway4=(u16)(scan->gateway4+card->gw4);
	scan->gateway6=(u16)(scan->gateway6+card->gw6);
	scan->ready4=(u16)(scan->ready4+(card->admin&&card->link&&card->has4&&card->gw4));
	scan->ready6=(u16)(scan->ready6+(card->admin&&card->link&&card->has6&&card->gw6));
	u8 score=(u8)(card->admin+card->link+card->has4+card->has6+card->gw4+card->gw6);
	if(score>scan->primary_score||!scan->primary.id){
		scan->primary=*card;
		scan->primary_score=score;
	}
	return(STATUS_OK);
}
status net_discovery_end(const net_discovery*scan)
{
	return(scan->cards&&(scan->primary.has4||scan->primary.has6)?STATUS_OK:ERR_STATE);
}
static void fill_addr4(net_card*card,const struct net_device*dev)
{
	struct in_device*in_dev=__in_dev_get_rtnl(dev);
	const struct in_ifaddr*ifa;
	const u8*bytes;
	if(!in_dev)
		return;
	ifa=rtnl_dereference(in_dev->ifa_list);
	if(!ifa)
		return;
	bytes=(const u8*)&ifa->ifa_local;
	for(u8 i=0U;i<sizeof(card->addr4.ip.bytes);i++)
		card->addr4.ip.bytes[i]=bytes[i];
	card->addr4.prefix=ifa->ifa_prefixlen;
	card->has4=1;
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
		if(ipv6_addr_type(&ifa->addr)!=IPV6_ADDR_UNICAST||
			ipv6_addr_src_scope(&ifa->addr)!=IPV6_ADDR_SCOPE_GLOBAL)
			continue;
		for(u8 i=0U;i<sizeof(card->addr6.ip.bytes);i++)
			card->addr6.ip.bytes[i]=a[i];
		card->addr6.prefix=(u8)ifa->prefix_len;
		card->has6=1;
		break;
	}
	read_unlock_bh(&idev->lock);
	in6_dev_put(idev);
}
static void fill_card(net_card*card,const struct net_device*dev)
{
	const struct device*parent;
	net_card_clear(card);
	card->id=(u32)dev->ifindex;
	net_text_copy(card->name,dev->name?dev->name:"",sizeof(card->name));
	card->present=1;
	card->admin=!!(dev->flags&IFF_UP);
	card->link=netif_carrier_ok(dev)?1:0;
	if(dev->addr_len==sizeof(card->mac)){
		for(u8 i=0U;i<sizeof(card->mac);i++)
			card->mac[i]=dev->dev_addr[i];
		card->has_mac=1;
	}
	parent=dev->dev.parent;
	if(parent){
		net_text_copy(card->driver,dev_driver_string(parent),sizeof(card->driver));
		net_text_copy(card->bus,dev_name(parent),sizeof(card->bus));
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
