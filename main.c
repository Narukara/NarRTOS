#include "stm32f10x.h"

#include "os_svc.h"
#include "os_task.h"
#include "os_types.h"

os_stack_t stack1[32];  // 128b
void task1(u32 ID) {
    while (0) {
        __SVC(0);
        __SVC(1);
    }
}

os_stack_t stack2[32];  // 128b
void task2(u32 ID) {
    while (1) {
        __SVC(2);
        __SVC(3);
    }
}

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

os_stack_t stack3[64];  // 256b
void task3(u32 ID) {
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
    os_svc_init();
    os_task_create(task1, stack1, 32);
    os_task_create(task2, stack2, 32);
    os_task_create(task3, stack3, 64);
    os_start();
}