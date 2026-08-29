#include<linux/inetdevice.h>
#include<linux/netdevice.h>
#include<linux/rtnetlink.h>
#include<net/addrconf.h>
#include<net/ipv6.h>
#include"net.h"
#include"util.h"
static void copy_text(char*dst,u32 size,const char*src)
{
	net_text_copy(dst,src?src:"",size);
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
	copy_text(card->name,NET_NAME_BYTES,dev->name);
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
		copy_text(card->driver,NET_DRIVER_BYTES,dev_driver_string(parent));
		copy_text(card->bus,NET_BUS_BYTES,dev_name(parent));
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
