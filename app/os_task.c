#include "stm32f10x.h"

typedef struct task_handler {
    u32* PSP;
    struct task_handler* next;
} os_task_handler;

static os_task_handler* task_now = 0;

/**
 * Tasks should not return.
 * If so, they will get stuck here.
 */
static void error_return() {
    while (1)
        ;
}

#define MAX_TASK 2
static os_task_handler* os_new_task_handler() {
    static os_task_handler handler[MAX_TASK];
    static u8 num = 0;
    if (num < MAX_TASK) {
        return handler + num++;
    } else {
        return 0;
    }
}

s32 os_task_init(void (*task)(), u32* stack, u32 size) {
    stack[size - 1] = 0x01000000;  // xPSR
    stack[size - 2] = (u32)task;   // PC, gcc will automatically +1 (thumb)
    stack[size - 3] = (u32)error_return;  // LR
    // stack[size - 4] = 0x00000000;         // R12
    // stack[size - 5] = 0x00000000;         // R3
    // stack[size - 6] = 0x00000000;         // R2
    // stack[size - 7] = 0x00000000;         // R1
    // stack[size - 8] = 0x00000000;         // R0
    os_task_handler* p = os_new_task_handler();
    if (p == 0) {
        return -1;
    }
    p->PSP = stack + size - 16;
    __disable_irq();
    if (task_now == 0) {
        task_now = p;
    } else {
        p->next = task_now->next;
    }
    task_now->next = p;
    __enable_irq();
    return 0;
}

void os_start() {
    NVIC_SetPriority(PendSV_IRQn, 0xF0);
    SysTick->LOAD = 90000;  // 0.01s
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03;
    while (1) {
        __DSB();
        __WFE();
    }
}

void SysTick_Handler() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // set PendSV
}

void PendSV_Handler() {
    static u8 init = 1;
    if (init) {
        init = 0;
        goto j;
    }
    __ASM(
        "mrs r0, psp;"
        "stmdb r0!,{r4-r11};");
    __ASM("str r0, %0" : "=m"(task_now->PSP));
    task_now = task_now->next;
j:
    __ASM("ldr r0, %0;" ::"m"(task_now->PSP));
    __ASM(
        "ldmia r0!, {r4-r11};"
        "msr psp, r0;"
        "ldr lr, =0xfffffffd");
}
