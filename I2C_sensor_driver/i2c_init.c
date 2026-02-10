// I2C_sensor_driver.c
//
/*source_code_ASM
.syntax unified
.cpu cortex-m4
.thumb

.equ RCC_BASE,    0x40023800
.equ GPIOB_BASE,  0x40020400
.equ I2C1_BASE,   0x40005400

.section .bss
.align 2
raw_data:    .space 2
filtered:    .word 0
prev_val:    .word 0

.section .text

@ --- I2C1 Init (Standard Mode 100kHz @ 16MHz) ---
.global i2c_init
i2c_init:
    ldr r0, =RCC_BASE
    ldr r1, [r0, #0x30]    @ AHB1ENR
    orr r1, #(1 << 1)      @ GPIOBEN
    str r1, [r0, #0x30]
    
    ldr r1, [r0, #0x40]    @ APB1ENR
    orr r1, #(1 << 21)     @ I2C1EN
    str r1, [r0, #0x40]

    ldr r0, =GPIOB_BASE
    @ PB8, PB9: Alternate Function (4), Open Drain (1), High Speed (3)
    ldr r1, [r0, #0x00]    @ MODER
    bic r1, #(0xF << 16)
    orr r1, #(0xA << 16)
    str r1, [r0, #0x00]

    ldr r1, [r0, #0x04]    @ OTYPER
    orr r1, #(0x3 << 8)
    str r1, [r0, #0x04]

    ldr r1, [r0, #0x24]    @ AFRH
    bic r1, #(0xFF << 0)
    orr r1, #(0x44 << 0)   @ AF4
    str r1, [r0, #0x24]

    ldr r0, =I2C1_BASE
    mov r1, #16            @ 16MHz PCLK
    str r1, [r0, #0x04]    @ CR2
    mov r1, #80            @ CCR = 100kHz
    str r1, [r0, #0x1C]
    mov r1, #17            @ TRISE
    str r1, [r0, #0x20]
    mov r1, #(1 << 0)      @ PE (Enable)
    str r1, [r0, #0x00]    @ CR1
    bx lr

*/

#include <stdint.h>

#define RCC_BASE     0x40023800
#define RCC_AHB1ENR  *(volatile uint32_t *) (RCC_BASE + 0x30)
#define RCC_APB1ENR  *(volatile uint32_t *) (RCC_BASE + 0x40)
#define RCC_I2C1EN   *(volatile uint32_t *) (RCC_BASE + 0x40)

#define GPIOB_BASE   0x40020400
#define GPIO_MODER   *(volatile uint32_t *) (GPIOB_BASE + 0x00)
#define GPIO_OSPEEDR *(volatile uint32_t *) (GPIOB_BASE + 0x08)
#define GPIO_OTYPER  *(volatile uint32_t *) (GPIOB_BASE + 0x04)
#define GPIO_AFRH    *(volatile uint32_t *) (GPIOB_BASE + 0x24)
#define GPIO_AF4     *(volatile uint32_t *) (GPIOB_BASE + 0x24)

#define I2C1_BASE     0x40005400
#define I2C1_CR1      *(volatile uint32_t *) (I2C1_BASE + 0x00)
#define I2C1_CR2      *(volatile uint32_t *) (I2C1_BASE + 0x04)
#define I2C1_CCR      *(volatile uint32_t *) (I2C1_BASE + 0x1C)
#define I2C1_TRISE    *(volatile uint32_t *) (I2C1_BASE + 0x20)
#define I2C1_PE       *(volatile uint32_t *) (I2C1_BASE + 0x00)

volatile uint16_t raw_data = 2;
volatile uint32_t filtered = 0;
volatile uint32_t prev_val = 0;

void i2c_init() 
{
// Enable Clocks
    RCC_AHB1ENR |= (1 << 1);
    RCC_APB1ENR |= (1 << 21);

// Configure GPIOB
    GPIO_MODER &= ~(0xF << 16);
    GPIO_MODER |= (0xA << 16);
    GPIO_OSPEEDR |= (0xF << 16);
    GPIO_OTYPER |= (0x3 << 8);
    GPIO_AFRH &= ~(0xFF << 0);
    GPIO_AF4 |= (0x44 << 0);

// Configure I2C1
    I2C1_CR2 = 16;       // PCLK1 = 16MHz
    I2C1_CCR = 80;       // 100kHz
    I2C1_TRISE = 17;     // TRISE for 100kHz
    I2C1_PE |= (1 << 0);      // PE (Enable)

}