#ifndef OS_SVC
#define OS_SVC

#include "stm32f10x.h"

void os_svc_init();

// arg must be const
#define __SVC(arg) __ASM volatile("svc %0" ::"i"(arg))

#endif