#include "stm32f10x.h"

#include "os_svc.h"
#include "os_task.h"
#include "os_types.h"

TASK_STACK stack1[32];  // 128b
void task1(u16 ID) {
    while (0) {
        __SVC(0);
        __SVC(1);
    }
}

TASK_STACK stack2[32];  // 128b
void task2(u16 ID) {
    while (1) {
        __SVC(2);
        __SVC(3);
    }
}

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

TASK_STACK stack3[64];  // 256b
void task3(u16 ID) {
    GPIO_Init(GPIOC, &(GPIO_InitTypeDef){.GPIO_Pin = GPIO_Pin_15,
                                         .GPIO_Mode = GPIO_Mode_Out_PP,
                                         .GPIO_Speed = GPIO_Speed_2MHz});
    while (1) {
        GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_SET);
        GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_RESET);
    }
}

int main() {
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;  // stack aligned on double-word
    os_init();
    task_create(task1, stack1, 32, 0);
    task_create(task2, stack2, 32, 0);
    task_create(task3, stack3, 64, 0);
    os_start();
}