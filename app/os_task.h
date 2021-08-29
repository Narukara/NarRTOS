#ifndef OS_TASK
#define OS_TASK

#include "stm32f10x.h"

s32 os_task_init(void (*task)(), u32* stack, u32 size);
void os_start();

#endif