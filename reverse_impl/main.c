//
// main.c
#include <stdint.h>

enum 
{
    ADDR_A = 0x400E1000,
    ADDR_B = 0x400E1400
};

typedef struct
{
    volatile uint32_t CTRL;
    uint32_t RESERVED[3];
    volatile uint32_t STATUS;
}  Peripheral_Type;

Peripheral_Type * const Device_A = (Peripheral_Type*)ADDR_A;
Peripheral_Type * const Device_B = (Peripheral_Type*)ADDR_B;

void delay(uint32_t count) {
    while (count--) {
        __asm__("");
    }
}

int main() {
    uint32_t r0 = 2, r1 = 1, r2 = 1, r3 = 0;
    Device_A->CTRL |= 0x04;

    while(1) 
    {
        delay(100000);

        if (Device_A->STATUS & 0x2000000) 
        {
            Device_A->CTRL &= ~1;

            if (!(Device_B->STATUS & 0x800000)) 
            {
                Device_A->CTRL |= 0x01;
                continue;
            }
        }
        
        if(r0 != r2) 
        {
            r0++;
        }
    
        else if (r0 != r1)
    
        {
            r1++;
        }
    
        else if (r2 != r1) 
        {
            r2++;
        }
    
        else 
        {
            r3++;
            r0 = 2;
            r1 = 1;
            r2 = 1;
            }


        if (r0 == 8 && r3 == 9)
        {
            r0 = 2;
            r1 = 1;
            r2 = 1;
            r3 = 0;
        } 
    
}
    return 0;
}