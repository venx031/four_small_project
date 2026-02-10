//
// main.c

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

volatile uint16_t raw_data = 2;
volatile uint32_t filtered = 0;
volatile uint32_t prev_val = 0;

void i2c_init();
void i2c_read_sensor();
void i2c_recovery();

int main() 
{
    i2c_init();
    while(1)
    {
        i2c_read_sensor();
        if(raw_data == 0xFFFF) {
            i2c_recovery();
        }
    }
}