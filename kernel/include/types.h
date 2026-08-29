#pragma once
typedef signed s16 __attribute__((mode(HI)));
typedef signed s32 __attribute__((mode(SI)));
typedef unsigned u8 __attribute__((mode(QI)));
typedef unsigned u16 __attribute__((mode(HI)));
typedef unsigned u32 __attribute__((mode(SI)));
typedef __typeof__(0ULL) u64;
typedef s32 status;
_Static_assert(sizeof(s16)==2,"s16");
_Static_assert(sizeof(s32)==4,"s32");
_Static_assert(sizeof(u8)==1,"u8");
_Static_assert(sizeof(u16)==2,"u16");
_Static_assert(sizeof(u32)==4,"u32");
_Static_assert(sizeof(u64)==8,"u64");
_Static_assert(sizeof(status)==4,"status");
#define STATUS_OK ((status)0)
#define ERR_INVAL ((status)-22)
#define ERR_STATE ((status)-16)
#define ERR_PLATFORM ((status)-95)