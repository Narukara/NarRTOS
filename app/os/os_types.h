#ifndef OS_TYPES
#define OS_TYPES

#include "stm32f10x.h"

#define os_stack_t _Alignas(uint64_t) u32

typedef struct os_task_handler{
    u32* PSP;
    struct os_task_handler* next;
    u32 ID;
} os_task_handler;

#endif