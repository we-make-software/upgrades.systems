#include"apply.h"
#include"util.h"
typedef struct{u8 plan,op;}step_rule;
static const step_rule step_rules[NET_STEP_MAX]={{NET_PLAN_ADMIN_UP,NET_OP_ADMIN_UP},{NET_PLAN_ADDR4,NET_OP_ADDR4},{NET_PLAN_ADDR6,NET_OP_ADDR6},{NET_PLAN_GATEWAY4,NET_OP_GATEWAY4},{NET_PLAN_GATEWAY6,NET_OP_GATEWAY6_REACH},{NET_PLAN_GATEWAY6,NET_OP_GATEWAY6},{NET_PLAN_LINK_UP,NET_OP_LINK_UP}};
static u8 prefix_same(const net_ip6*a,const net_ip6*b,u8 prefix)
{
	u8 bytes=(u8)(prefix>>3U),bits=(u8)(prefix&7U);
	if(bytes&&!net_bytes_same(a->bytes,b->bytes,bytes)) return(0U);
	return((u8)(!bits||!((a->bytes[bytes]^b->bytes[bytes])&(u8)(0xffU<<(8U-bits)))));
}
static u8 gateway_onlink(const net_addr6*source,const net_ip6*gateway)
{
	return((u8)((gateway->bytes[0]==0xfeU&&(gateway->bytes[1]&0xc0U)==0x80U)||prefix_same(&source->ip,gateway,source->prefix)));
}
status net_plan_ops(u8*ops,const net_card*card,const net_plan*plan)
{
	*ops=0U; status ret;
	if((ret=net_plan_validate(plan))||(ret=net_card_validate(card))) return(ret);
	if(net_plan_required_flags(plan)&~plan->flags) return(ERR_STATE);
	u8 missing=0U;
	switch(ret=net_plan_preflight(&missing,card,plan)){case STATUS_OK:case ERR_STATE:break;default:return(ret);}
	if(missing&NET_PLAN_CARD_ID) return(ERR_STATE);
	for(u8 i=0U;i<NET_STEP_MAX;i++) if(missing&step_rules[i].plan) *ops|=step_rules[i].op;
	if((*ops&NET_OP_GATEWAY6)&&gateway_onlink(&plan->addr6,&plan->gateway6)) *ops=net_flags_without(*ops,NET_OP_GATEWAY6_REACH);
	return(*ops?ERR_STATE:STATUS_OK);
}
status net_plan_steps(net_step*steps,u8*count,u8 max,const net_card*card,const net_plan*plan)
{
	*count=0U; u8 ops=0U; status ret;
	switch(ret=net_plan_ops(&ops,card,plan)){case STATUS_OK:return(ops?ERR_STATE:STATUS_OK);case ERR_STATE:if(!ops) return(ERR_STATE);break;default:return(ret);}
	for(u8 i=0U;i<NET_STEP_MAX;i++){
		if(!(ops&step_rules[i].op)) continue;
		if(*count>=max){*count=0U;return(ERR_INVAL);}
		steps[*count].op=step_rules[i].op;*count+=1U;
	}
	return(ERR_STATE);
}