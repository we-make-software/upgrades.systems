#pragma once
#include"net.h"
#define NET_OP_ADDR4 1U
#define NET_OP_ADDR6 2U
#define NET_OP_GATEWAY4 4U
#define NET_OP_GATEWAY6 8U
#define NET_OP_ADMIN_UP 16U
#define NET_OP_LINK_UP 32U
#define NET_OP_GATEWAY6_REACH 64U
#define NET_STEP_MAX 7U
typedef struct{u8 op;}net_step;
status net_plan_ops(u8*,const net_card*,const net_plan*);
status net_plan_steps(net_step*,u8*,u8,const net_card*,const net_plan*);