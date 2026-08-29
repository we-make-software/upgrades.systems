#pragma once
#include<types.h>
#define NET_NAME_BYTES 16U
#define NET_DRIVER_BYTES 32U
#define NET_BUS_BYTES 32U
#define NET_MAC_BYTES 6U
#define NET_ADDR4_BYTES 4U
#define NET_ADDR6_BYTES 16U
#define NET_CARD_PRESENT 1U
#define NET_CARD_ADMIN_UP 2U
#define NET_CARD_LINK_UP 4U
#define NET_CARD_MAC 8U
#define NET_CARD_ADDR4 16U
#define NET_CARD_ADDR6 32U
#define NET_CARD_GATEWAY4 64U
#define NET_CARD_GATEWAY6 128U
#define NET_PLAN_ADDR4 1U
#define NET_PLAN_ADDR6 2U
#define NET_PLAN_GATEWAY4 4U
#define NET_PLAN_GATEWAY6 8U
#define NET_PLAN_CARD_ID 16U
#define NET_PLAN_ADMIN_UP 32U
#define NET_PLAN_LINK_UP 64U
typedef struct{u8 bytes[NET_ADDR4_BYTES];}net_ip4;
typedef struct{u8 bytes[NET_ADDR6_BYTES];}net_ip6;
typedef struct{net_ip4 ip;u8 prefix;}net_addr4;
typedef struct{net_ip6 ip;u8 prefix;}net_addr6;
typedef struct{
	u32 id;
	char name[NET_NAME_BYTES],driver[NET_DRIVER_BYTES],bus[NET_BUS_BYTES];
	u8 flags,mac[NET_MAC_BYTES];
	net_addr4 addr4;
	net_addr6 addr6;
	net_ip4 gateway4;
	net_ip6 gateway6;
}net_card;
typedef struct{
	u32 card_id;
	char name[NET_NAME_BYTES];
	net_addr4 addr4;
	net_ip4 gateway4;
	net_addr6 addr6;
	net_ip6 gateway6;
	u8 flags;
}net_plan;
typedef struct{
	u32 card_id;
	char name[NET_NAME_BYTES];
	net_addr4 addr4;
	net_ip4 gateway4;
	net_addr6 addr6;
	net_ip6 gateway6;
	u8 flags;
}net_config;
typedef struct{
	u16 cards,invalid,admin_up,link_up,addr4,addr6,gateway4,gateway6,ready4,ready6;
	u8 primary_score;
	net_card primary;
}net_discovery;
void net_card_clear(net_card*);
status net_card_validate(const net_card*);
status net_plan_from_card(net_plan*,const net_card*);
status net_plan_validate(const net_plan*);
u8 net_plan_required_flags(const net_plan*);
status net_plan_match(u8*,const net_card*,const net_plan*);
status net_plan_preflight(u8*,const net_card*,const net_plan*);
status net_config_validate(const net_config*);
status net_config_plan(net_plan*,const net_config*,const net_card*);
void net_discovery_begin(net_discovery*);
status net_discovery_card(net_discovery*,const net_card*);
status net_discovery_end(const net_discovery*);
status net_scan(net_discovery*);
status net_start(void);
status net_check(void);