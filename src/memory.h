#ifndef MEMORY_DEF
#define MEMORY_DEF

#include "game.h"

#define ALIGNMENT 8
#define STACK_TOP 1
#define STACK_BOTTOM 0

#define MAIN_STACK STACK_BOTTOM
#define TEMP_STACK STACK_TOP

void *Memory_StackAllocClear(u8 end, u32 size);
void Memory_StackCopy(void *restrict dest, const void *restrict from, u32 size);
void *Memory_StackAlloc(u8 end, u32 size);
void Memory_StackPop(u8 end, u16 num);
void Memory_Close(void);
void Memory_Init(u32 size);

#endif