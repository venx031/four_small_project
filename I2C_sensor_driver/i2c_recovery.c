/*
@ --- Error Recovery ---
i2c_recovery:
    mov r1, #(1 << 15)     @ SWRST
    str r1, [r0, #0x00]
    mov r1, #0
    str r1, [r0, #0x00]
    bl i2c_init            @ Re-init
    pop {r4, r5, pc}
*/

#include <stdint.h>

#define I2C1_BASE     0x40005400

/*
#define I2C1_CR1      *(volatile uint32_t *) (I2C1_BASE + 0x00)
#define I2C1_CR2      *(volatile uint32_t *) (I2C1_BASE + 0x04)
#define I2C1_CCR      *(volatile uint32_t *) (I2C1_BASE + 0x1C)
#define I2C1_TRISE    *(volatile uint32_t *) (I2C1_BASE + 0x20)
*/

#define I2C1_PE       *(volatile uint32_t *) (I2C1_BASE + 0x00)
#define I2C1_CR1      *(volatile uint32_t *) (I2C1_BASE + 0x00)
#define I2C1_SR1      *(volatile uint32_t *) (I2C1_BASE + 0x14)
#define I2C1_SR2      *(volatile uint32_t *) (I2C1_BASE + 0x18)
#define I2C1_DR       *(volatile uint32_t *) (I2C1_BASE + 0x10)
#define I2C1_ADDR     *(volatile uint32_t *) (I2C1_BASE + 0x14)
#define I2C1_AF       *(volatile uint32_t *) (I2C1_BASE + 0x14)
#define I2C1_RXNE     *(volatile uint32_t *) (I2C1_BASE + 0x14)

// Bit-masks
#define I2C1_START     (1 << 8)
#define I2C1_STOP      (1 << 9)
#define I2C1_ACK       (1 << 10)
#define I2C1_SWRST     (1 << 15)

void i2c_recovery()
{
    I2C1_CR1 = I2C1_SWRST;
    I2C1_CR1 = I2C1_PE;

    // Re-init
    extern void i2c_init();
    i2c_init();

}
