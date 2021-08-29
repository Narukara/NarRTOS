#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void os_svc_init() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_Init(GPIOC, &(GPIO_InitTypeDef){
                         .GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14,
                         .GPIO_Mode = GPIO_Mode_Out_PP,
                         .GPIO_Speed = GPIO_Speed_2MHz,
                     });
}

void SVC_Handler() {
    u8 svc_arg;
    __ASM(
        "tst lr, #4;"
        "ite eq;"
        "mrseq r0, msp;"
        "mrsne r0, psp;"
        "ldr r0, [r0, #24];"
        "ldrb %0, [r0, #-2]"
        : "=r"(svc_arg));
    switch (svc_arg) {
        case 0:
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
            break;
        case 1:
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            break;
        case 2:
            GPIO_SetBits(GPIOC, GPIO_Pin_14);
            break;
        default:
            GPIO_ResetBits(GPIOC, GPIO_Pin_14);
            break;
    }
}