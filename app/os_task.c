#include "stm32f10x.h"

#include "os_types.h"

static TASK_HANDLER *task_top, *task_now, *task_next;

/**
 * Tasks should not return. If so, they will get stuck here.
 * If all tasks are waiting, this one will be executed.
 */
static TASK_STACK idle_task_stack[16];
static void idle_task() {
    while (1) {
        __DSB();
        __WFI();
    }
}

#define MAX_TASK 4
static TASK_HANDLER* os_new_task_handler() {
    static TASK_HANDLER handler[MAX_TASK];
    static u8 num = 0;
    if (num < MAX_TASK) {
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
 * @param priority The priority of task. The larger the number, the higher the
 * priority.
 * @return ID of the task. 0 if failed.
 */
u16 task_create(void (*task)(u16), u32* stack, u32 size, u8 priority) {
    static u16 ID = 0;
    __disable_irq();
    TASK_HANDLER* p = os_new_task_handler();  // malloc
    if (p == 0) {
        __enable_irq();
        return 0;
    }
    ID++;
    stack[size - 1] = 0x01000000;      // xPSR
    stack[size - 2] = (u32)task;       // PC, gcc will automatically +1 (thumb)
    stack[size - 3] = (u32)idle_task;  // LR
    // stack[size - 4] = 0x00000000;         // R12
    // stack[size - 5] = 0x00000000;         // R3
    // stack[size - 6] = 0x00000000;         // R2
    // stack[size - 7] = 0x00000000;         // R1
    stack[size - 8] = ID;  // R0, the parameter passed to the task.
    p->PSP = stack + size - 16;
    p->ID = ID;
    p->priority = priority;
    p->status = READY;
    p->timeout = 0;

    TASK_HANDLER* now = task_top;
    TASK_HANDLER* last = &task_top;
    while (now != 0 && now->priority > priority) {
        last = now;
        now = now->next;
    }
    p->next = now;
    last->next = p;

    __enable_irq();
    return ID;
}

void os_init() {
    os_svc_init();
    task_create(idle_task, idle_task_stack, 16, 0);
}

/**
 * Make sure that at least one task has been created.
 * This function never returns.
 */
void os_start() {
    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    SysTick->LOAD = 9000;  // 1ms
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03;
    __ASM("ldr r0, %0;" ::"m"(task_top->PSP));
    __ASM(
        "msr psp, r0;"
        "ldr r0, =0x03;"
        "msr control, r0;"  // use PSP
        "isb;"
        "pop {r4-r11};"
        "pop {r0-r3, r12, lr};"
        "pop {r4, r5};"  // r4 = PC, r5 = xPSR
        "msr xPSR, r5;"
        "mov pc, r4");  // jump to task 1
}

void SysTick_Handler() {
    // timeout - 1

    // Task scheduling
    if (task_now->status == ATOMIC) {
        // get up
        return;
    }
    u8 now_priority = task_now->priority;
    task_next = task_top;
    while (1) {
        if (task_next->status == READY) {
            if (task_next->priority != now_priority) {
                break;
            } else {
                if (task_now->next == 0 ||
                    task_now->next->priority < now_priority) {
                    break;
                } else {
                    // TODO
                }
            }
        } else {
            task_next = task_next->next;
        }
    }
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // set PendSV
}

void PendSV_Handler() {
    __ASM(
        "mrs r0, psp;"
        "stmdb r0!,{r4-r11};");
    __ASM("str r0, %0" : "=m"(task_now->PSP));
    task_now = task_next;
    __ASM("ldr r0, %0;" ::"m"(task_now->PSP));
    __ASM(
        "ldmia r0!, {r4-r11};"
        "msr psp, r0");
}
