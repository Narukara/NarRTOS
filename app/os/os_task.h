#ifndef OS_TASK
#define OS_TASK

#include "stm32f10x.h"

void os_init();
u32 os_task_create(void (*task)(u32), u32* stack, u32 size, u8 priority);
void os_task_delay(u32 ms);
void os_start();

#endif