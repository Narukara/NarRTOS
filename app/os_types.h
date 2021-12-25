#ifndef OS_TYPES
#define OS_TYPES

#include "stm32f10x.h"

#define TASK_STACK _Alignas(uint64_t) u32

enum task_status {
    READY = 0,
    WAITING,
    ATOMIC,
};

typedef struct TASK_HANDLER {
    struct TASK_HANDLER* next;
    u32* PSP;
    u16 ID;
    u16 timeout;
    u8 priority;
    u8 status;
} TASK_HANDLER;

#endif