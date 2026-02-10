/*
@ --- Read Sensor + Filter + Recovery ---
.global read_sensor
read_sensor:
    push {r4, r5, lr}
    ldr r0, =I2C1_BASE

    @ 1. Start & Address
    mov r1, #(1 << 8) | (1 << 0) @ START + PE
    str r1, [r0, #0x00]
wait_sb:
    ldr r1, [r0, #0x14]    @ SR1
    tst r1, #(1 << 0)      @ SB bit
    beq wait_sb

    mov r1, #0x32          @ Addr (0x19 << 1) | Write
    str r1, [r0, #0x10]    @ DR
wait_addr:
    ldr r1, [r0, #0x14]
    tst r1, #(1 << 10)     @ AF (Acknowledge Failure)?
    bne i2c_recovery
    tst r1, #(1 << 1)      @ ADDR bit
    beq wait_addr
    ldr r1, [r0, #0x18]    @ SR2 read to clear ADDR

    @ 2. Read 2 Bytes
    mov r1, #(1 << 10) | (1 << 0) @ ACK + PE
    str r1, [r0, #0x00]
    
wait_rxne:
    ldr r1, [r0, #0x14]
    tst r1, #(1 << 6)      @ RXNE
    beq wait_rxne
    ldrb r4, [r0, #0x10]   @ Byte 1

    mov r1, #(1 << 9) | (1 << 0) @ STOP + PE (No ACK for last byte)
    str r1, [r0, #0x00]
    
wait_rxne2:
    ldr r1, [r0, #0x14]
    tst r1, #(1 << 6)
    beq wait_rxne2
    ldrb r5, [r0, #0x10]   @ Byte 2

    @ 3. Filtering (Average)
    lsl r4, r4, #8
    orr r4, r4, r5         @ Combined 16-bit
    
    ldr r1, =prev_val
    ldr r2, [r1]
    add r3, r4, r2
    lsr r3, r3, #1         @ (New + Prev) / 2
    str r4, [r1]           @ Update Prev
    
    ldr r1, =filtered
    str r3, [r1]           @ Result in RAM
    
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

volatile uint16_t raw_data = 2;
volatile uint32_t filtered = 0;
volatile uint32_t prev_val = 0;

void i2c_read_sensor() 
{
    I2C1_CR1 = I2C1_START | I2C1_PE;
    while(!(I2C1_SR1 & (1 << 0)));

    I2C1_DR = 0x32;

    while(1) {
        uint32_t sr1 = I2C1_SR1;
        if(sr1 & (1 << 10)) {
            return;
        }
        if(sr1 & (1 << 1)) break;
    };

    (void)I2C1_SR2;

    // read 2 bytes
    I2C1_CR1 = I2C1_ACK | I2C1_PE;

    while(!(I2C1_SR1 & (1 << 6)));
    uint8_t byte = (uint8_t)I2C1_DR;

    I2C1_CR1 = I2C1_STOP | I2C1_PE;

    while(!(I2C1_SR1 & (1 << 6)));
    uint8_t byte2 = (uint8_t)I2C1_DR;

    uint16_t combined = (byte << 8) | byte2;
    uint32_t new_val = (combined + prev_val) >> 1;
    prev_val = combined;
    filtered = new_val;
}

