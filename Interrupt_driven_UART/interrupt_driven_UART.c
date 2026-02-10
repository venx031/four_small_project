// interrupt_driven_UART.c
//
/* source_code_ASM
.syntax unified
.cpu cortex-m4
.thumb

.equ RCC_BASE,    0x40023800
.equ GPIOA_BASE,  0x40020000
.equ USART2_BASE, 0x40004400
.equ NVIC_ISER1,  0xE000E104

.section .bss
.align 2
rx_buffer: .space 64
head_ptr:  .word 0
tail_ptr:  .word 0

.section .text

@ --- Инициализация ---
.global uart_init
uart_init:
    @ 1. Тактирование GPIOA и USART2
    ldr r0, =RCC_BASE
    ldr r1, [r0, #0x30]    @ RCC_AHB1ENR
    orr r1, #(1 << 0)      @ GPIOAEN
    str r1, [r0, #0x30]
    
    ldr r1, [r0, #0x40]    @ RCC_APB1ENR
    orr r1, #(1 << 17)     @ USART2EN
    str r1, [r0, #0x40]

    @ 2. GPIOA PA2, PA3 (AF7)
    ldr r0, =GPIOA_BASE
    ldr r1, [r0, #0x00]    @ MODER
    bic r1, #(0xF << 4)
    orr r1, #(0xA << 4)    @ Alternate Function
    str r1, [r0, #0x00]

    ldr r1, [r0, #0x24]    @ AFRL
    bic r1, #(0xFF << 8)
    orr r1, #(0x77 << 8)   @ AF7 для PA2 и PA3
    str r1, [r0, #0x24]

    @ 3. USART2 Настройка (9600 при 16MHz)
    ldr r0, =USART2_BASE
    ldr r1, =0x683         @ BRR
    str r1, [r0, #0x08]

    mov r1, #(1 << 13) | (1 << 5) | (1 << 3) | (1 << 2) @ UE, RXNEIE, TE, RE
    str r1, [r0, #0x0C]    @ CR1

    @ 4. NVIC
    ldr r0, =NVIC_ISER1
    mov r1, #(1 << (38 - 32))
    str r1, [r0]
    bx lr

@ --- Обработчик прерывания ---
.global USART2_IRQHandler
.type USART2_IRQHandler, %function
USART2_IRQHandler:
    ldr r0, =USART2_BASE
    ldr r1, [r0, #0x00]    @ SR
    tst r1, #(1 << 5)      @ RXNE?
    it eq
    bxeq lr

    ldr r1, [r0, #0x04]    @ DR (байт данных)
    
    ldr r2, =head_ptr
    ldr r3, [r2]
    ldr r0, =rx_buffer
    strb r1, [r0, r3]      @ Пишем в буфер

    add r3, r3, #1
    and r3, r3, #63        @ Кольцо
    str r3, [r2]
    bx lr
*/

#include <stdint.h>

#define RCC_BASE         0x40023800UL
#define RCC_AHB1ENR     *(volatile uint32_t *)(RCC_BASE + 0x30)
#define RCC_APB1ENR     *(volatile uint32_t *)(RCC_BASE + 0x40)

#define GPIOA_BASE       0x40020000UL
#define GPIOA_MODER     *(volatile uint32_t *)(GPIOA_BASE + 0x00)
#define GPIOA_AFRL      *(volatile uint32_t *)(GPIOA_BASE + 0x24)

#define USART2_BASE      0x40004400UL
#define USART2_SR       *(volatile uint32_t *)(USART2_BASE + 0x00)
#define USART2_DR       *(volatile uint32_t *)(USART2_BASE + 0x04)
#define USART2_BRR      *(volatile uint32_t *)(USART2_BASE + 0x08)
#define USART2_CR1      *(volatile uint32_t *)(USART2_BASE + 0x0C)

#define NVIC_ISER1      *(volatile uint32_t *)(0xE000E104UL)
#define BUF_SIZE         64

volatile uint32_t rx_buffer[BUF_SIZE];
volatile uint32_t head_ptr = 0;
volatile uint32_t tail_ptr = 0;

void uart_init() {
    RCC_AHB1ENR |= (1 << 0); // GPIOAEN
    RCC_APB1ENR |= (1 << 17); // USART2EN

    GPIOA_MODER &= ~(0xF << 4);
    GPIOA_MODER |= (0xA << 4); // Alternate Function

    GPIOA_AFRL &= ~(0xFF << 8);
    GPIOA_AFRL |= (0x77 << 8); // AF7_for_PA2_and_PA3

    USART2_BRR = 0x683; // BRR
    USART2_CR1 = (1 << 13) | (1 << 5) | (1 << 3) | (1 << 2); // UE, RXNEIE, TE, RE
    NVIC_ISER1 |= ((1 << (38 - 32)));

}

void USART2_IRQHandler() {
    if(USART2_SR & (1 << 5)) {
        volatile uint32_t data = USART2_DR; // Read data
        rx_buffer[head_ptr] = data; // Store in buffer
        head_ptr = (head_ptr + 1) & (BUF_SIZE -1); 
    }
}

int main() {
    uart_init();

    while(1) {
        if(tail_ptr != head_ptr) {
            uint32_t data = rx_buffer[tail_ptr];
            tail_ptr = (tail_ptr + 1) & (BUF_SIZE -1);
            // Process data

             while(!(USART2_SR & (1 << 7))); // Wait until TXE
            USART2_DR = data;
        }
    
    }

return 0;
}