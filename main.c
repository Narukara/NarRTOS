#include "stm32f10x.h"

#include "os_svc.h"
#include "os_task.h"
#include "os_types.h"

os_stack_t stack1[64];  // 256 bytes
void task1(u32 ID) {
    while (1) {
        os_task_delay(1000);
        os_svc(setC13);
        os_task_delay(1000);
        os_svc(resetC13);
    }
}

os_stack_t stack2[64];  // 256 bytes
void task2(u32 ID) {
    while (1) {
        os_task_delay(500);
        os_svc(setC14);
        os_task_delay(500);
        os_svc(resetC14);
    }
}

int main() {
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;  // stack aligned on double-word
    os_init();
    os_task_create(task1, stack1, 64, 1);
    os_task_create(task2, stack2, 64, 1);
    os_start();
}