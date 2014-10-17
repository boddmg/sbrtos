/**
* Get into the pendSV interrupt to switch the tasks.
*/
__asm void sys_switch_hal()
{	
NVIC_INT_CTRL   EQU     0xE000ED04                              
NVIC_SYSPRI14   EQU     0xE000ED22                              
NVIC_PENDSV_PRI EQU           0xFF                              
NVIC_PENDSVSET  EQU     0x10000000                              

LDR     R0, =NVIC_INT_CTRL
LDR     R1, =NVIC_PENDSVSET
STR     R1, [R0]
BX      LR                          // Enable interrupts at processor level
ALIGN
}

/**
* Set the pendSV interrupt priority as the lowest one to make sure the task switching won't
* break other interrupt.
*/
__asm void sys_init()
{
    LDR     R0, =NVIC_SYSPRI14      // Set the PendSV interrupt priority 
    LDR     R1, =NVIC_PENDSV_PRI		
    STRB    R1, [R0]

    MOVS    R0, #0                  // Set the PSP to 0 for initial context switch call
    MSR     PSP, R0

		LDR     R0, =NVIC_INT_CTRL      // Trigger the PendSV interrupt (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    CPSIE   I                       // Enable interrupts at processor level

os_start_hang
    B       os_start_hang           // Should never get here
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
																				 // 首次调用，是没有上文
	
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
