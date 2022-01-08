#include "stm32f10x.h"

#include "os_svc.h"
#include "os_types.h"

static os_task_handler* task_head;
static os_task_handler *task_now, *task_last;

static os_stack_t idle_task_stack[16];
static u32 idle_PSP = (u32)&idle_task_stack;
static os_task_handler* idle_handler = (void*)&idle_PSP;  // fake task handler
static void idle_task() {
    while (1) {
        __DSB();
        __WFI();
    }
}

/**
 * Tasks should not return.
 * If so, they will get stuck here.
 */
static void error_return() {
    while (1)
        ;
}

void os_init() {
    NVIC_SetPriority(PendSV_IRQn, 0xFF);
    os_svc_init();
    // create idle task
    idle_task_stack[15] = 0x01000000;      // xPSR
    idle_task_stack[14] = (u32)idle_task;  // PC
}

#define MAX_TASK_NUM 8
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
 * @param stack At least 16 words (64 bytes). must be Double Word Aligned
 * @param size The number of words in the stack, must >= 16
 * @param priority priority of task, must >= 1
 * @return ID of the task. 0 if failed.
 */
u32 os_task_create(void (*task)(u32), u32* stack, u32 size, u8 priority) {
    static u32 ID = 0;
    __disable_irq();
    os_task_handler* p = os_new_task_handler();
    if (p == 0) {
        __enable_irq();
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
    p->timeout = 0;
    p->status = ready;
    p->priority = priority;
    if (task_head == 0) {
        task_head = p;
    } else {
        p->next = task_head->next;
    }
    task_head->next = p;
    __enable_irq();
    return ID;
}

/**
 * @brief delay a task
 * @note delay time is imprecise
 * @param ms milliseconds to delay. 0 stands for infinity
 */
void os_task_delay(u32 ms) {
    __disable_irq();
    task_now->status = suspend;
    task_now->timeout = ms;
    __enable_irq();
    volatile u8* status = &(task_now->status);
    while (*status == suspend) {
        __WFE();
        __DSB();
    }
}

/**
 * Make sure that at least one task has been created.
 * This function never return.
 */
void os_start() {
    task_now = task_head;
    SysTick->LOAD = 9000;  // 1ms
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03;
    __ASM("ldr r0, %0;" ::"m"(task_head->PSP));
    __ASM(
        "msr psp, r0;"
#ifdef OS_PRIVILEGED_TASK
        "ldr r0, =0x02;"  // run tasks at privileged level
#else
        "ldr r0, =0x03;"  // run tasks at unprivileged level
#endif
        "msr control, r0;"  // use PSP
        "isb;"
        "pop {r4-r11};"
        "pop {r0-r3, r12, lr};"
        "pop {r4, r5};"  // r4 = PC, r5 = xPSR
        "msr xPSR, r5;"
        "mov pc, r4");  // jump to task 1
    // no return
}

void SysTick_Handler() {
    task_last = task_now;
    u8 max_priority = 0;
    os_task_handler* stop = task_head;
    do {
        if (task_head->status == suspend) {
            if (task_head->timeout > 0) {
                task_head->timeout--;
                if (task_head->timeout == 0) {
                    task_head->status = ready;
                }
            }
        }
        if (task_head->status == ready) {
            if (task_head->priority > max_priority) {
                max_priority = task_head->priority;
                task_now = task_head;
            }
        }
        task_head = task_head->next;
    } while (task_head != stop);

    if (max_priority > 0) {
        // next task found, move task_head
        task_head = task_head->next;
    } else {
        // not found, keep task_head still
        task_now = idle_handler;
    }

    if (task_last != task_now) {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // set PendSV
    }
}

/**
 * Be careful with this function!
 * It shouldn't use push/pop, that would mess up the context.
 * It is recommended to check the disassembled code.
 */
void PendSV_Handler() {
    __ASM(
        "mrs r0, psp;"
        "stmdb r0!,{r4-r11};");  // push r4-r11 on PSP
    __ASM("str r0, %0" : "=m"(task_last->PSP));
    __ASM("ldr r0, %0;" ::"m"(task_now->PSP));
    __ASM(
        "ldmia r0!, {r4-r11};"  // pop r4-r11 from PSP
        "msr psp, r0");
}
