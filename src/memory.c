#include <malloc.h>
#include <assert.h>
#include "memory.h"

#define ALIGN_UP(T, offset) (T)(((uintptr_t)offset + ALIGNMENT - 1) & ~(ALIGNMENT-1))
#define METADATA_SIZE ALIGNMENT
// #define METADATA_SIZE ALIGN_UP(u32, sizeof(u32))

static void *memory;
static void *stackEnds[2];
static void *stack_caps[2];

void Memory_Init(u32 size){

	size = ALIGN_UP(u32, size) - ALIGNMENT;
	
	memory = malloc(size + ALIGNMENT);

	stack_caps[STACK_BOTTOM] = ALIGN_UP(void *, memory);
	stack_caps[STACK_TOP] = ALIGN_UP(void *, memory + size);

	stackEnds[STACK_BOTTOM] = stack_caps[STACK_BOTTOM];
	stackEnds[STACK_TOP] = stack_caps[STACK_TOP];
}

void Memory_Close(void){

	free(memory);
}

void Memory_StackPop(u8 end, u16 num){

	u32 k;
	
	if(end){

		if(stackEnds[STACK_TOP] == stack_caps[STACK_TOP]) return;
	
		for(k = 0; k < num; k++)
			if(stackEnds[STACK_TOP] < stack_caps[STACK_TOP])
				stackEnds[STACK_TOP] += *((u32 *)stackEnds[STACK_TOP]);

	} else {

		if(stackEnds[STACK_BOTTOM] == stack_caps[STACK_BOTTOM]) return;

		for(k = 0; k < num; k++)
			if(stackEnds[STACK_BOTTOM] > stack_caps[STACK_BOTTOM])
				stackEnds[STACK_BOTTOM] -= *((u32 *)(stackEnds[STACK_BOTTOM] - METADATA_SIZE));
	}

}

void *Memory_StackAlloc(u8 end, u32 size){

	size = ALIGN_UP(u32, size) + METADATA_SIZE;

#ifdef MEMORY_DEBUG
	assert((uintptr_t)stackEnds[STACK_BOTTOM] + size < (uintptr_t)stackEnds[STACK_TOP]);
	assert((uintptr_t)stackEnds[STACK_TOP] - size > (uintptr_t)stackEnds[STACK_BOTTOM]);
#endif

	void *mem = NULL;

	if(end){

		stackEnds[STACK_TOP] -= size;
		mem = stackEnds[STACK_TOP] + METADATA_SIZE;
		*((u32 *)stackEnds[STACK_TOP]) = size;

	} else {
		mem = stackEnds[STACK_BOTTOM];
		stackEnds[STACK_BOTTOM] += size - METADATA_SIZE;
		*((u32 *)stackEnds[STACK_BOTTOM]) = size;
		stackEnds[STACK_BOTTOM] += METADATA_SIZE;

	}

	return mem;
}

void *Memory_StackAllocClear(u8 end, u32 size){

	u8 *mem = (u8 *)Memory_StackAlloc(end, size);

	u32 k;
	for(k = 0; k < size; k++)
		mem[k] = 0;

	return mem;
}

void Memory_StackCopy(void *restrict dest, const void *restrict from, u32 size){

	u8 *dest8 = (u8 *)dest;
	u8 *from8 = (u8 *)from;

	u32 k;
	for(k = 0; k < size; k++)
		dest8[k] = from8[k];
}