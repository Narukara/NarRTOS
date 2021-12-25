#ifndef OS_TASK
#define OS_TASK

#include "stm32f10x.h"

void os_init();
u32 os_task_create(void (*task)(u32), u32* stack, u32 size);
void os_start();

#endif