#include "stm32f10x.h"

typedef struct task_handler {
    u32* PSP;
    struct task_handler* next;
    u32 ID;
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

#define MAX_TASK_NUM 4
/**
 * @brief get a new handler
 */
static os_task_handler* os_new_task_handler() {
    static os_task_handler handler[MAX_TASK_NUM];
    static u8 num = 0;
    if (num < MAX_TASK_NUM) {
        return handler + num++;
    } else {
        return 0;
    }
}

/**
 * @param task Task to be executed. The parameter passed to the task is the ID
 * of it.
 * @param stack At least 64 bytes (16 words)
 * @param size The number of words in the stack, must >= 16
 * @return ID of the task. 0 if failed.
 */
u32 os_task_create(void (*task)(u32), u32* stack, u32 size) {
    static u32 ID = 0;
    __disable_irq();
    os_task_handler* p = os_new_task_handler();
    if (p == 0) {
        return 0;
    }
    ID++;
    stack[size - 1] = 0x01000000;  // xPSR
    stack[size - 2] = (u32)task;   // PC, gcc will automatically +1 (thumb)
    stack[size - 3] = (u32)error_return;  // LR
    // stack[size - 4] = 0x00000000;         // R12
    // stack[size - 5] = 0x00000000;         // R3
    // stack[size - 6] = 0x00000000;         // R2
    // stack[size - 7] = 0x00000000;         // R1
    stack[size - 8] = ID;  // R0, the parameter passed to the task.
    p->PSP = stack + size - 16;
    p->ID = ID;
    if (task_now == 0) {
        task_now = p;
    } else {
        p->next = task_now->next;
    }
    task_now->next = p;
    __enable_irq();
    return ID;
}

/**
 * Make sure that at least one task has been created.
 * This function never return.
 */
void os_start() {
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    SysTick->LOAD = 9000;  // 1ms
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03;
    __ASM("ldr r0, %0;" ::"m"(task_now->PSP));
    __ASM(
        "msr psp, r0;"
        "ldr r0, =0x02;"
        "msr control, r0;"  // use PSP
        "pop {r4-r11};"
        "pop {r0-r3, r12, lr};"
        "pop {r4, r5};"  // r4 = PC, r5 = xPSR
        "msr xPSR, r5;"
        "mov pc, r4");  // jump to task 1
}

void SysTick_Handler() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // set PendSV
}

void PendSV_Handler() {
    __ASM(
        "mrs r0, psp;"
        "stmdb r0!,{r4-r11};"); // push r4-r11 on PSP
    __ASM("str r0, %0" : "=m"(task_now->PSP));
    task_now = task_now->next;
    __ASM("ldr r0, %0;" ::"m"(task_now->PSP));
    __ASM(
        "ldmia r0!, {r4-r11};"  // pop r4-r11 from PSP
        "msr psp, r0");
}
