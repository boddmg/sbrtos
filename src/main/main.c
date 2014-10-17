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
	int a[30]={0};
	while(1)
	{
		a[29]++;
		now_state = 1;
		sys_switch();
	}
}

void task2_fun()
{
	int a[30] = {0};
	while(1)
	{
		a[29]++;
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
