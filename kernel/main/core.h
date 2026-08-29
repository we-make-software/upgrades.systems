#pragma once
#include<types.h>
#define CORE_READY 1U
#define CORE_STARTING 2U
#define CORE_RUNNING 3U
#define CORE_STOPPING 4U
typedef u32 core_state;
typedef struct core_unit core_unit;
typedef struct core_ctx core_ctx;
typedef status(*core_enter)(void*);
typedef void(*core_leave)(void*);
struct core_unit{
	core_unit*prev,*next;
	core_ctx*owner;
	core_enter enter;
	core_leave leave;
	void*context;
	status result;
	u8 required,active;
};
typedef struct{
	void(*log)(const char*);
}core_ops;
typedef struct{
	const core_ops*ops;
}core_config;
struct core_ctx{
	core_state state;
	const core_ops*ops;
	core_unit*first,*last,*failed;
	status failed_result;
	u8 changing;
};
status core_init(core_ctx*,const core_config*);
status core_unit_add(core_ctx*,core_unit*);
status core_start(core_ctx*);
status core_stop(core_ctx*);
