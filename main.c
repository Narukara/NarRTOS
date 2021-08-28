#include "stm32f10x.h"

_Alignas(uint64_t) u32 stack[64];  // 256b

void task(u32 a) {
    u32 i = a;
    while (1) {
        SysTick->LOAD = i++;
    }
}

int main() {
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
    __NOP();
    stack[63] = 0x01000000;  // xPSR
    stack[62] = (u32)task;   // PC, gcc will automatically +1 (thumb)
    stack[61] = 0x00000000;  // LR
    stack[60] = 0x00000000;  // R12
    stack[59] = 0x00000000;  // R3
    stack[58] = 0x00000000;  // R2
    stack[57] = 0x00000000;  // R1
    stack[56] = 0x00000010;  // R0
    __NOP();
    SysTick->LOAD = 900000;  // 0.1s
    SysTick->VAL = 0;
    SysTick->CTRL = 0x03;
    while (1)
        ;
}

void SysTick_Handler() {
    SysTick->CTRL = 0;
    __NOP();
    __ASM volatile("msr psp, %0" ::"r"(stack + 56));
    __ASM volatile("ldr lr, =0xfffffffd");
}