#pragma once
#include<types.h>
enum core_state{
	CORE_READY=1,
	CORE_STARTING,
	CORE_RUNNING,
	CORE_STOPPING
};
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
	u8 required:1,active:1;
};
typedef struct{
	void(*log)(const char*);
}core_ops;
typedef struct{
	const core_ops*ops;
}core_config;
struct core_ctx{
	enum core_state state;
	const core_ops*ops;
	core_unit*first,*last,*failed;
	status failed_result;
	u8 changing;
};
status core_init(core_ctx*,const core_config*);
status core_unit_add(core_ctx*,core_unit*);
status core_start(core_ctx*);
status core_stop(core_ctx*);
