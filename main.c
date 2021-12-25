#include "stm32f10x.h"

#include "os_svc.h"
#include "os_task.h"
#include "os_types.h"

static void delay(u32);

os_stack_t stack1[32];  // 128 bytes
void task1(u32 ID) {
    while (1) {
        delay(3600000);
        __SVC(0);  // PC13 = 1
        delay(3600000);
        __SVC(1);  // PC13 = 0
    }
}

os_stack_t stack2[32];  // 128 bytes
void task2(u32 ID) {
    while (1) {
        delay(1800000);
        __SVC(2);  // PC14 = 1
        delay(1800000);
        __SVC(3);  // PC14 = 0
    }
}

int main() {
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;  // stack aligned on double-word
    os_init();
    os_task_create(task1, stack1, 32);
    os_task_create(task2, stack2, 32);
    os_start();
}

#pragma GCC push_options
#pragma GCC optimize("O0")

static void delay(u32 time) {
    while (time > 0)
        time--;
}

#pragma GCC pop_options