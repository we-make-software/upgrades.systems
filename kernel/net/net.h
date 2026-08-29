#pragma once
#include <types.h>
typedef struct {
	u8 bytes[4];
} net_ip4;
typedef struct {
	u8 bytes[16];
} net_ip6;
typedef struct {
	net_ip4 ip;
	u8 prefix;
} net_addr4;
typedef struct {
	net_ip6 ip;
	u8 prefix;
} net_addr6;
typedef struct {
	u32 id;
	char name[16];
	char driver[32];
	char bus[32];
	u8 present : 1;
	u8 admin : 1;
	u8 link : 1;
	u8 has_mac : 1;
	u8 has4 : 1;
	u8 has6 : 1;
	u8 gw4 : 1;
	u8 gw6 : 1;
	u8 mac[6];
	net_addr4 addr4;
	net_addr6 addr6;
	net_ip4 gateway4;
	net_ip6 gateway6;
} net_card;
typedef struct {
	u32 card_id;
	char name[16];
	u8 has4 : 1;
	u8 has6 : 1;
	u8 gw4 : 1;
	u8 gw6 : 1;
	u8 admin : 1;
	u8 link : 1;
	net_addr4 addr4;
	net_ip4 gateway4;
	net_addr6 addr6;
	net_ip6 gateway6;
} net_plan;
typedef struct {
	u16 cards;
	u16 invalid;
	u16 admin_up;
	u16 link_up;
	u16 addr4;
	u16 addr6;
	u16 gateway4;
	u16 gateway6;
	u16 ready4;
	u16 ready6;
	u8 primary_score;
	net_card primary;
} net_discovery;
void net_card_clear(net_card *);
status net_card_validate(const net_card *);
status net_plan_from_card(net_plan *, const net_card *);
status net_plan_validate(const net_plan *);
void net_discovery_begin(net_discovery *);
status net_discovery_card(net_discovery *, const net_card *);
status net_discovery_end(const net_discovery *);
status net_scan(net_discovery *);
status net_start(void);
