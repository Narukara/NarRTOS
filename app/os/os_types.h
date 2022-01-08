#ifndef OS_TYPES
#define OS_TYPES

#include "stm32f10x.h"

#define os_stack_t _Alignas(uint64_t) u32

enum task_status {
    ready,
    suspend,
};

typedef struct os_task_handler {
    u32* PSP;
    struct os_task_handler* next;
    u32 ID;
    u32 timeout;  // 0 stands for infinity
    u8 status;
    u8 priority;  // must > 0
} os_task_handler;

#endif