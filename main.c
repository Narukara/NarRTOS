#include "stm32f10x.h"

#include "os_svc.h"
#include "os_task.h"
#include "os_types.h"

STACK stack1[64];  // 256b
void task1() {
    while (1) {
        __SVC(0);
        __SVC(1);
    }
}

STACK stack2[64];  // 256b
void task2() {
    while (1) {
        __SVC(2);
        __SVC(3);
    }
}

int main() {
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;  // stack aligned on double-word
    os_svc_init();
    if (os_task_init(task1, stack1, 64) == -1) {
        while (1)
            ;
    }
    if (os_task_init(task2, stack2, 64) == -1) {
        while (1)
            ;
    }
    os_start();
}