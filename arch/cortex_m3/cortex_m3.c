/*
	M-Kernel - embedded RTOS
	Copyright (c) 2011-2012, Alexey Kramarenko
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
		list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "cortex_m3.h"
#include "arch.h"
#include "irq.h"
#include "memmap.h"
#include "kernel_config.h"
#include "error.h"
#include "magic.h"
#include "dbg.h"

#define CFSR_IACCVIOL							(1 << 0)
#define CFSR_DACCVIOL							(1 << 1)
#define CFSR_MUNSTKERR							(1 << 3)
#define CFSR_MSTKERR								(1 << 4)

#define CFSR_IBUSERR								(1 << 8)
#define CFSR_PRECISERR							(1 << 9)
#define CFSR_IMPRECISERR						(1 << 10)
#define CFSR_BUNSTKERR							(1 << 11)
#define CFSR_BSTKERR								(1 << 12)

#define CFSR_UNDEFINSTR							(1 << 16)
#define CFSR_INVSTATE							(1 << 17)
#define CFSR_INVPC								(1 << 18)
#define CFSR_NOCP									(1 << 19)
#define CFSR_UNALIGNED							(1 << 24)
#define CFSR_DIVBYZERO							(1 << 25)

CONTEXT get_context()
{
	CONTEXT res = IRQ_CONTEXT;
	switch (__get_IPSR())
	{
	case 0:
		switch (__get_CONTROL())
		{
		case 2:
			res = SYSTEM_CONTEXT;
			break;
		case 3:
			res = USER_CONTEXT;
			break;
		default:
			//in early init PSP is not set
			res = SUPERVISOR_CONTEXT;
		}
		break;
	case 11:
	case 14:
		res = SUPERVISOR_CONTEXT;
		break;
	}
	return res;
}

#if (KERNEL_PROFILING)
unsigned int stack_used_max(unsigned int top, unsigned int cur)
{
	unsigned int i;
	unsigned int last = cur;
	for (i = cur - sizeof(unsigned int); i >= top; i -= 4)
		if (*(unsigned int*)i != MAGIC_UNINITIALIZED)
			last = i;
	return last;
}

void svc_stack_stat()
{
	unsigned int sp, current_stack, max_stack;
	printf("    type       stack        top\n\r");
	printf("-----------------------------------\n\r");
	sp = (unsigned int)__get_MSP();
	current_stack = (SVC_STACK_END - sp) / sizeof(unsigned int);
	max_stack = (SVC_STACK_END - stack_used_max(SVC_STACK_TOP, sp)) / sizeof(unsigned int);
	printf("Supervisor %3d/%3d/%3d   0x%08x\n\r", current_stack, max_stack, SVC_STACK_SIZE, SVC_STACK_TOP);
}
#endif //KERNEL_PROFILING

void HardFault_Handler(void)
{
	if (SCB->HFSR & SCB_HFSR_VECTTBL_Msk)
		fatal_error(ERROR_GENERAL_VECTOR_TABLE_READ_FAULT, NULL);
	fatal_error(ERROR_GENERAL_HARD_FAULT, NULL);
}

void MemManage_Handler(void)
{
	if (SCB->CFSR & CFSR_DACCVIOL)
		error_address(ERROR_MEM_DATA_ACCESS_VIOLATION, SCB->MMFAR);
	else if (SCB->CFSR & CFSR_IACCVIOL)
		error_address(ERROR_MEM_INSTRUCTION_ACCESS_VIOLATION, SCB->MMFAR);
	else if (SCB->CFSR & CFSR_MSTKERR)
		fatal_error(ERROR_GENERAL_STACKING_FAILED, NULL);
	else if (SCB->CFSR & CFSR_MUNSTKERR)
		fatal_error(ERROR_GENERAL_UNSTACKING_FAILED, NULL);
	else
		fatal_error(ERROR_GENERAL_HARD_FAULT, NULL);
}

void BusFault_Handler(void)
{
	if (SCB->CFSR & CFSR_BSTKERR)
		fatal_error(ERROR_GENERAL_STACKING_FAILED, NULL);
	if (SCB->CFSR & CFSR_BUNSTKERR)
		fatal_error(ERROR_GENERAL_UNSTACKING_FAILED, NULL);
	unsigned int address = SCB->BFAR;
	if (SCB->CFSR & (CFSR_IMPRECISERR | CFSR_PRECISERR))
		fatal_error_address(ERROR_GENERAL_DATA_BUS_ERROR, address);
	if (SCB->CFSR & CFSR_IBUSERR)
		fatal_error_address(ERROR_GENERAL_INSTRUCTION_BUS_ERROR, address);
	fatal_error(ERROR_GENERAL_HARD_FAULT, NULL);
}

void UsageFault_Handler(void)
{
	if (SCB->CFSR & CFSR_DIVBYZERO)
		error_thread(ERROR_GENERAL_DIVISION_BY_ZERO);
	else if (SCB->CFSR & CFSR_UNALIGNED)
		fatal_error(ERROR_GENERAL_UNALIGNED_ACCESS, NULL);
	else if (SCB->CFSR & CFSR_NOCP)
		error_thread(ERROR_GENERAL_NO_COPROCESSOR);
	else if (SCB->CFSR & (CFSR_UNDEFINSTR | CFSR_INVPC))
		error_thread(ERROR_GENERAL_UNDEFINED_INSTRUCTION);
	else if (SCB->CFSR & CFSR_INVSTATE)
		error_thread(ERROR_GENERAL_INVALID_STATE);
	//clear bits and continue
	SCB->CFSR = SCB->CFSR;
}

void default_irq_handler(void)
{
#if (KERNEL_DEBUG)
	printf("Warning: irq vector %d without handler\n\r", __get_IPSR());
#endif
}

