#pragma once
#include<types.h>
typedef struct{
	u8 bytes[4];
}net_ip4;
typedef struct{
	u8 bytes[16];
}net_ip6;
typedef struct{
	net_ip4 ip;
	u8 prefix;
}net_addr4;
typedef struct{
	net_ip6 ip;
	u8 prefix;
}net_addr6;
typedef struct{
	u32 id;
	char name[16],driver[32],bus[32];
	u8 present:1,admin:1,link:1,has_mac:1,has4:1,has6:1,gw4:1,gw6:1;
	u8 mac[6];
	net_addr4 addr4;
	net_addr6 addr6;
	net_ip4 gateway4;
	net_ip6 gateway6;
}net_card;
typedef struct{
	u32 card_id;
	char name[16];
	u8 has4:1,has6:1,gw4:1,gw6:1,admin:1,link:1;
	net_addr4 addr4;
	net_ip4 gateway4;
	net_addr6 addr6;
	net_ip6 gateway6;
}net_plan;
typedef struct{
	u8 cards,invalid,admin_up,link_up,addr4,addr6,gateway4,gateway6,ready4,ready6,primary_score;
	net_card primary;
}net_discovery;
void net_card_clear(net_card*);
status net_card_validate(const net_card*);
status net_plan_from_card(net_plan*,const net_card*);
status net_plan_validate(const net_plan*);
void net_discovery_begin(net_discovery*);
status net_discovery_card(net_discovery*,const net_card*);
status net_discovery_end(const net_discovery*);
status net_scan(net_discovery*);
status net_start(void);
