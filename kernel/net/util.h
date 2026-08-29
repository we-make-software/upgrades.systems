#pragma once
#include"net.h"
status net_text_valid(const char*,u32,u32);
void net_text_copy(char*,const char*,u32);
status net_addr4_valid(const net_addr4*);
status net_addr6_valid(const net_addr6*);
status net_bytes_same(const u8*,const u8*,u32);
u32 net_bytes_any(const u8*,u32);
static inline u8 net_flags_without(u8 flags,u8 mask)
{
	return((u8)(flags&(u8)~mask));
}
