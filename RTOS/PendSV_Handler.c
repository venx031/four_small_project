// rtos_priorities.c
//
/*source_code_ASM
.syntax unified
.cpu cortex-m4
.thumb

.equ NVIC_INT_CTRL, 0xE000ED04
.equ PENDSVSET_BIT, 0x10000000

@ Структура TCB (Task Control Block):
@ 0: stack_ptr (SP)
@ 4: priority (0 - высший)
@ 8: state (0 - Ready, 1 - Blocked)

.section .data
.align 2
current_task_ptr: .word 0

@ Пример списка задач
tasks:
    @ Task 1
    .word 0x20001000  @ SP (заполнится при инициализации)
    .word 2           @ Priority (Low)
    .word 0           @ State (Ready)
    @ Task 2
    .word 0x20001100  @ SP
    .word 1           @ Priority (High)
    .word 0           @ State (Ready)
tasks_end:

.section .text

@ --- Переключение контекста (PendSV) ---
.global PendSV_Handler
.type PendSV_Handler, %function
PendSV_Handler:
    cpsid i                 @ Critical section
    
    @ 1. Сохранение контекста текущей задачи
    ldr r0, =current_task_ptr
    ldr r1, [r0]
    cmp r1, #0
    beq skip_save           @ Если первая задача — не сохраняем
    
    mrs r2, psp             @ Берем указатель стека задачи
    stmdb r2!, {r4-r11}     @ Сохраняем регистры R4-R11 на стек задачи
    str r2, [r1]            @ TCB->stack_ptr = SP
    
skip_save:
    @ 2. Планировщик (Priority Scheduler)
    ldr r1, =tasks
    ldr r2, =tasks_end
    mov r3, #255            @ r3 = текущий лучший приоритет (max)
    mov r4, #0              @ r4 = указатель на лучшую задачу

find_loop:
    ldr r5, [r1, #8]        @ TCB->state
    cmp r5, #0
    bne next_task           @ Если не Ready — пропускаем
    
    ldr r6, [r1, #4]        @ TCB->priority
    cmp r6, r3
    bhs next_task           @ Если приоритет ниже (число больше) — пропускаем
    
    mov r3, r6              @ Новый лучший приоритет
    mov r4, r1              @ Сохраняем указатель на эту задачу
    
next_task:
    add r1, r1, #12         @ К следующему TCB
    cmp r1, r2
    blo find_loop

    @ 3. Восстановление контекста выбранной задачи
    str r4, [r0]            @ current_task_ptr = r4
    ldr r2, [r4]            @ r2 = TCB->stack_ptr
    ldmia r2!, {r4-r11}     @ Восстанавливаем R4-R11
    msr psp, r2             @ Обновляем PSP
    
    mov lr, #0xFFFFFFFD     @ Возврат в Thread Mode через PSP
    cpsie i
    bx lr

@ --- Функция запроса переключения ---
.global yield
yield:
    ldr r0, =NVIC_INT_CTRL
    ldr r1, =PENDSVSET_BIT
    str r1, [r0]
    dsb
    isb
    bx lr
*/

#include <stdint.h>

#define NVIC_INT_CTRL      0xE000ED04
#define PENDSVSET_BIT      0x10000000

typedef struct 
{
uint32_t stack_ptr;
uint32_t priority;
uint32_t state;
} TCB_struct;

volatile TCB_struct* current_task_ptr = 0;
volatile TCB_struct* task = 0;
volatile TCB_struct* task_end = 0;

TCB_struct task_list[2];
uint32_t some_stack_array[40];
uint32_t another_stack_array[40];

void PendSV_Handler() 
{
    __asm volatile ("cpsid i");

    if (current_task_ptr !=0) 
    {
        uint32_t stack_ptr = current_task_ptr->stack_ptr;
        __asm volatile ("mrs %0, psp" : "=r"(stack_ptr));
        __asm volatile ("stmdb %0!, {r4-r11}" : "+r" (stack_ptr));
        current_task_ptr->stack_ptr = stack_ptr;
    }

    // Priority Scheduler
    TCB_struct* best_task = 0;
    uint32_t best_priority = 255;
    for (TCB_struct* tcb = (TCB_struct*)task; tcb < (TCB_struct*)task_end; tcb++) 
    {
        if (tcb->state == 0) // Ready
        {
            if (tcb->priority < best_priority) 
            {
                best_priority = tcb->priority;
                best_task = tcb;
            }
        }
    }

    current_task_ptr = best_task;
    uint32_t stack_ptr = best_task->stack_ptr;
    __asm volatile ("ldmia %0!, {r4-r11}" : "+r"(stack_ptr));
    __asm volatile ("msr psp, %0" : : "r"(stack_ptr));
    __asm volatile ("cpsie i");
    __asm volatile ("mov lr, #0xFFFFFFFD");
    __asm volatile ("bx lr");
}

void Func1() 
{
    volatile uint32_t count1 = 0;
    while(1) 
    {
        count1++;
    }
}

void Func2() 
{
    volatile uint32_t count2 = 0;
    while(1) 
    {
        count2++;
    }
}

void task_init(int task_idx, void (*task_func)(void), uint32_t *stack_top) {
    stack_top[0] = 0x01000000;
    stack_top[-1] = (uint32_t)task_func;
    stack_top[-2] = 0xFFFFFFFD;
    
    task_list[task_idx].stack_ptr = (uint32_t)(stack_top - 16);
}

int main() 
{
    task_init(0, Func1, &some_stack_array[39]);
    task_list[0].priority = 2;
    task_list[0].state = 0;

    task_init(1, Func2, &another_stack_array[39]);
    task_list[1].priority = 1;
    task_list[1].state = 0;

    task = &task_list[0];
    task_end = &task_list[2];

    *(volatile uint32_t*)NVIC_INT_CTRL = PENDSVSET_BIT;

    __asm volatile ("cpsie i");
    
    while(1);

}