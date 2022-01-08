#ifndef OS_SVC
#define OS_SVC

#include "stm32f10x.h"

void os_svc_init();

enum SVC_type {
    setC13,
    resetC13,
    setC14,
    resetC14,
};

// arg must be const
#define os_svc(arg) __ASM volatile("svc %0" ::"i"(arg))

#endif