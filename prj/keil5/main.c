#include "stm32f10x.h"
#include "system_stm32f10x.h" 
#include <stdio.h>

#define SIZE_OF_TASK 0xff

static uint32_t now_state = 0;
struct TypedefTask
{
	uint32_t *stack_p;
};
extern void sys_switch_hal (void);
struct TypedefTask task1;
struct TypedefTask task2;

struct TypedefTask *task_next = &task2;
struct TypedefTask *task_current = &task1;

static uint32_t task1_stack[SIZE_OF_TASK];
static uint32_t task2_stack[SIZE_OF_TASK];

typedef void (*OS_TASK_BODY)(void);

void task_end(void)
{
	now_state = 3;
	while(1)
  {}
}

void sys_switch()
{
	if (task_current == &task1)
	{
		task_next = &task2;
	}else
	{
		task_next = &task1;
	}
	sys_switch_hal();
}
void task1_fun()
{
	while(1)
	{
		now_state = 1;
		sys_switch();
	}
}

void task2_fun()
{
	while(1)
	{
		now_state = 2;
		sys_switch();
	}
}

void task_init(struct TypedefTask *task,OS_TASK_BODY task_body,uint32_t *stack_p)
{
	*(--stack_p) = (uint32_t)0x01000000uL;                          //xPSR
	*(--stack_p) = (uint32_t)task_body;                             // Entry Point
	*(--stack_p) = (uint32_t)task_end;                              // R14 (LR)
	*(--stack_p) = (uint32_t)0x12121212uL;                          // R12
	*(--stack_p) = (uint32_t)0x03030303uL;                          // R3
	*(--stack_p) = (uint32_t)0x02020202uL;                          // R2
	*(--stack_p) = (uint32_t)0x01010101uL;                          // R1
	*(--stack_p) = (uint32_t)0x00000000u;                           // R0

	*(--stack_p) = (uint32_t)0x11111111uL;                          // R11
	*(--stack_p) = (uint32_t)0x10101010uL;                          // R10
	*(--stack_p) = (uint32_t)0x09090909uL;                          // R9
	*(--stack_p) = (uint32_t)0x08080808uL;                          // R8
	*(--stack_p) = (uint32_t)0x07070707uL;                          // R7
	*(--stack_p) = (uint32_t)0x06060606uL;                          // R6
	*(--stack_p) = (uint32_t)0x05050505uL;                          // R5
	*(--stack_p) = (uint32_t)0x04040404uL;                          // R4
	task->stack_p = stack_p;
}

__asm void sys_switch_hal()
{	
NVIC_INT_CTRL   EQU     0xE000ED04                              
NVIC_SYSPRI14   EQU     0xE000ED22                              
NVIC_PENDSV_PRI EQU           0xFF                              
NVIC_PENDSVSET  EQU     0x10000000                              

	LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR                                                // Enable interrupts at processor level
	ALIGN
}

__asm void sys_init()
{
    LDR     R0, =NVIC_SYSPRI14                                  // Set the PendSV exception priority
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0]

    MOVS    R0, #0                                              // Set the PSP to 0 for initial context switch call
    MSR     PSP, R0

		LDR     R0, =NVIC_INT_CTRL                                  // Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    CPSIE   I                                                   // Enable interrupts at processor level

os_start_hang
    B       os_start_hang                                         // Should never get here
	ALIGN
}


__asm void PendSV_Handler(void)
{
	PRESERVE8

	IMPORT  task_next
	IMPORT  task_current
	

	CPSID   I                              // 关中断
	MRS     R0, PSP                        // 获得PSP
	CBZ     R0, task_switch_nosave				 // PSP为0跳到OS_CPU_PendSVHandler_nosave，即不保存上文，直接进入下文。
																				 // 问什么呢，因为首次调用，是没有上文的。
																				 // 保存上文
	SUBS    R0, R0, #0x20                  // 因为寄存器是32位的，4字节对齐，自动压栈的寄存器有8个，所以偏移为8*0x04=0x20
	STM     R0, {R4-R11}                   // 除去自动压栈的寄存器外，需手动将R4-R11压栈

	LDR     R1, =task_current              // 保存上文的SP指针 task_current->stack_p = SP;
	LDR     R1, [R1]
	STR     R0, [R1]

																															
task_switch_nosave                				 // 切换下文

	LDR     R0, =task_current                  // 置 task_current  = task_next;
	LDR     R1, =task_next
	LDR     R2, [R1]
	STR     R2, [R0]

	LDR     R0, [R2]                       // R0中的值为新任务的SP; SP = task_current->stack_p;
	LDM     R0, {R4-R11}                   // 手动弹出 R4-R11
	ADDS    R0, R0, #0x20
 
			
	MSR     PSP, R0                        // PSP = 新任务SP
	ORR     LR, LR, #0x04                  // 确保异常返回后使用PSP
	CPSIE   I
	BX      LR                             // 退出异常，从PSP弹出xPSR，PC，LR，R0-R3，进入新任务运行   
	ALIGN
}


void SysTick_Handler (void) {
}


int main(void)
{
	uint32_t a=0;
	a++;
	a++;
	
	task_init(&task1,task1_fun, &task1_stack[SIZE_OF_TASK-1]);
	task_init(&task2,task2_fun, &task2_stack[SIZE_OF_TASK-1]);
	
	sys_init();
	task1_fun();
//	SysTick_Config(SystemCoreClock / 1000);
//	printf("123");
	while (1)
	{
	}
}
