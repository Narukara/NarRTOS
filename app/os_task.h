#ifndef OS_TASK
#define OS_TASK

#include "stm32f10x.h"

void os_init();
u32 task_create(void (*task)(u32), u32* stack, u32 size, u8 priority);
void os_start();

#endif