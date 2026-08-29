#include<linux/err.h>
#include<linux/inetdevice.h>
#include<linux/netdevice.h>
#include<linux/rcupdate.h>
#include<linux/rtnetlink.h>
#include<net/addrconf.h>
#include<net/ip6_fib.h>
#include<net/ip6_route.h>
#include<net/ipv6.h>
#include<net/route.h>
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
	return(!card||!card->id?ERR_INVAL:(!card->present?ERR_STATE:
		(net_text_valid(card->name,sizeof(card->name),1U)||
		net_text_valid(card->driver,sizeof(card->driver),0U)||
		net_text_valid(card->bus,sizeof(card->bus),0U)||
		(card->gw4&&!card->has4)||(card->gw6&&!card->has6)||
		(card->gw4&&!net_bytes_any(card->gateway4.bytes,sizeof(card->gateway4.bytes)))||
		(card->gw6&&!net_bytes_any(card->gateway6.bytes,sizeof(card->gateway6.bytes)))||
		(card->has4&&net_addr4_valid(&card->addr4))||
		(card->has6&&net_addr6_valid(&card->addr6))?ERR_INVAL:STATUS_OK)));
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
	return(!plan->card_id||net_text_valid(plan->name,sizeof(plan->name),1U)||
		(plan->gw4&&!plan->has4)||(plan->gw6&&!plan->has6)||
		(plan->gw4&&!net_bytes_any(plan->gateway4.bytes,sizeof(plan->gateway4.bytes)))||
		(plan->gw6&&!net_bytes_any(plan->gateway6.bytes,sizeof(plan->gateway6.bytes)))||
		(plan->has4&&net_addr4_valid(&plan->addr4))||
		(plan->has6&&net_addr6_valid(&plan->addr6))?ERR_INVAL:
		(plan->has4||plan->has6?STATUS_OK:ERR_STATE));
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
	scan->admin_up=(u8)(scan->admin_up+card->admin);
	scan->link_up=(u8)(scan->link_up+card->link);
	scan->addr4=(u8)(scan->addr4+card->has4);
	scan->addr6=(u8)(scan->addr6+card->has6);
	scan->gateway4=(u8)(scan->gateway4+card->gw4);
	scan->gateway6=(u8)(scan->gateway6+card->gw6);
	scan->ready4=(u8)(scan->ready4+(card->admin&&card->link&&card->has4&&card->gw4));
	scan->ready6=(u8)(scan->ready6+(card->admin&&card->link&&card->has6&&card->gw6));
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
	card->addr4.ip.bytes[0]=bytes[0];
	card->addr4.ip.bytes[1]=bytes[1];
	card->addr4.ip.bytes[2]=bytes[2];
	card->addr4.ip.bytes[3]=bytes[3];
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
		card->addr6.ip.bytes[0]=a[0];
		card->addr6.ip.bytes[1]=a[1];
		card->addr6.ip.bytes[2]=a[2];
		card->addr6.ip.bytes[3]=a[3];
		card->addr6.ip.bytes[4]=a[4];
		card->addr6.ip.bytes[5]=a[5];
		card->addr6.ip.bytes[6]=a[6];
		card->addr6.ip.bytes[7]=a[7];
		card->addr6.ip.bytes[8]=a[8];
		card->addr6.ip.bytes[9]=a[9];
		card->addr6.ip.bytes[10]=a[10];
		card->addr6.ip.bytes[11]=a[11];
		card->addr6.ip.bytes[12]=a[12];
		card->addr6.ip.bytes[13]=a[13];
		card->addr6.ip.bytes[14]=a[14];
		card->addr6.ip.bytes[15]=a[15];
		card->addr6.prefix=(u8)ifa->prefix_len;
		card->has6=1;
		break;
	}
	read_unlock_bh(&idev->lock);
	in6_dev_put(idev);
}
static __be32 ipv4(u8 a,u8 b,u8 c,u8 d)
{
	return(htonl(((u32)a<<24)|((u32)b<<16)|((u32)c<<8)|d));
}
static void fill_route4(net_card*card,const struct net_device*dev)
{
	if(!card->has4)
		return;
	struct flowi4 fl4={
		.flowi4_oif=dev->ifindex,
		.flowi4_scope=RT_SCOPE_UNIVERSE,
		.daddr=ipv4(1U,1U,1U,1U),
		.saddr=ipv4(card->addr4.ip.bytes[0],card->addr4.ip.bytes[1],card->addr4.ip.bytes[2],card->addr4.ip.bytes[3])
	};
	struct rtable*rt=ip_route_output_flow(dev_net(dev),&fl4,0);
	if(IS_ERR(rt))
		return;
	if(rt->rt_uses_gateway&&rt->rt_gw_family==AF_INET){
		const u8*bytes=(const u8*)&rt->rt_gw4;
		card->gateway4.bytes[0]=bytes[0];
		card->gateway4.bytes[1]=bytes[1];
		card->gateway4.bytes[2]=bytes[2];
		card->gateway4.bytes[3]=bytes[3];
		card->gw4=1;
	}
	ip_rt_put(rt);
}
static void fill_route6(net_card*card,const struct net_device*dev)
{
	if(!card->has6)
		return;
	struct in6_addr probe;
	probe.s6_addr[0]=38U;
	probe.s6_addr[1]=6U;
	probe.s6_addr[2]=71U;
	probe.s6_addr[3]=0U;
	probe.s6_addr[4]=71U;
	probe.s6_addr[5]=0U;
	probe.s6_addr[6]=0U;
	probe.s6_addr[7]=0U;
	probe.s6_addr[8]=0U;
	probe.s6_addr[9]=0U;
	probe.s6_addr[10]=0U;
	probe.s6_addr[11]=0U;
	probe.s6_addr[12]=0U;
	probe.s6_addr[13]=0U;
	probe.s6_addr[14]=17U;
	probe.s6_addr[15]=17U;
	struct flowi6 fl6={.flowi6_oif=dev->ifindex,.daddr=probe};
	fl6.saddr.s6_addr[0]=card->addr6.ip.bytes[0];
	fl6.saddr.s6_addr[1]=card->addr6.ip.bytes[1];
	fl6.saddr.s6_addr[2]=card->addr6.ip.bytes[2];
	fl6.saddr.s6_addr[3]=card->addr6.ip.bytes[3];
	fl6.saddr.s6_addr[4]=card->addr6.ip.bytes[4];
	fl6.saddr.s6_addr[5]=card->addr6.ip.bytes[5];
	fl6.saddr.s6_addr[6]=card->addr6.ip.bytes[6];
	fl6.saddr.s6_addr[7]=card->addr6.ip.bytes[7];
	fl6.saddr.s6_addr[8]=card->addr6.ip.bytes[8];
	fl6.saddr.s6_addr[9]=card->addr6.ip.bytes[9];
	fl6.saddr.s6_addr[10]=card->addr6.ip.bytes[10];
	fl6.saddr.s6_addr[11]=card->addr6.ip.bytes[11];
	fl6.saddr.s6_addr[12]=card->addr6.ip.bytes[12];
	fl6.saddr.s6_addr[13]=card->addr6.ip.bytes[13];
	fl6.saddr.s6_addr[14]=card->addr6.ip.bytes[14];
	fl6.saddr.s6_addr[15]=card->addr6.ip.bytes[15];
	struct fib6_result res={0};
	rcu_read_lock();
	if(fib6_lookup(dev_net(dev),dev->ifindex,&fl6,&res,RT6_LOOKUP_F_IFACE)){
		rcu_read_unlock();
		return;
	}
	if(res.nh&&res.nh->fib_nh_dev==dev&&res.nh->fib_nh_gw_family==AF_INET6){
		card->gateway6.bytes[0]=res.nh->fib_nh_gw6.s6_addr[0];
		card->gateway6.bytes[1]=res.nh->fib_nh_gw6.s6_addr[1];
		card->gateway6.bytes[2]=res.nh->fib_nh_gw6.s6_addr[2];
		card->gateway6.bytes[3]=res.nh->fib_nh_gw6.s6_addr[3];
		card->gateway6.bytes[4]=res.nh->fib_nh_gw6.s6_addr[4];
		card->gateway6.bytes[5]=res.nh->fib_nh_gw6.s6_addr[5];
		card->gateway6.bytes[6]=res.nh->fib_nh_gw6.s6_addr[6];
		card->gateway6.bytes[7]=res.nh->fib_nh_gw6.s6_addr[7];
		card->gateway6.bytes[8]=res.nh->fib_nh_gw6.s6_addr[8];
		card->gateway6.bytes[9]=res.nh->fib_nh_gw6.s6_addr[9];
		card->gateway6.bytes[10]=res.nh->fib_nh_gw6.s6_addr[10];
		card->gateway6.bytes[11]=res.nh->fib_nh_gw6.s6_addr[11];
		card->gateway6.bytes[12]=res.nh->fib_nh_gw6.s6_addr[12];
		card->gateway6.bytes[13]=res.nh->fib_nh_gw6.s6_addr[13];
		card->gateway6.bytes[14]=res.nh->fib_nh_gw6.s6_addr[14];
		card->gateway6.bytes[15]=res.nh->fib_nh_gw6.s6_addr[15];
		card->gw6=1;
	}
	rcu_read_unlock();
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
		card->mac[0]=dev->dev_addr[0];
		card->mac[1]=dev->dev_addr[1];
		card->mac[2]=dev->dev_addr[2];
		card->mac[3]=dev->dev_addr[3];
		card->mac[4]=dev->dev_addr[4];
		card->mac[5]=dev->dev_addr[5];
		card->has_mac=1;
	}
	parent=dev->dev.parent;
	if(parent){
		net_text_copy(card->driver,dev_driver_string(parent),sizeof(card->driver));
		net_text_copy(card->bus,dev_name(parent),sizeof(card->bus));
	}
	fill_addr4(card,dev);
	fill_addr6(card,dev);
	fill_route4(card,dev);
	fill_route6(card,dev);
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
