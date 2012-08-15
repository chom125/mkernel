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

#ifndef SYS_CALLS_H
#define SYS_CALLS_H

/*
		list of all sys calls
  */


#include "kernel_config.h"

//num is the number of sys call
//0x0    .. 0xfff  - must be called in SVC/IRQ context
//0x1000 .. 0x1fff - must be called in SYS/SVC/IRQ context
//0x2000 ..			 - can be called in any context

#define CALL_GROUP						0x10
#define CALL_GROUP_MASK					0xfffffff0
#define CALL_CONTEXT						0x1000

typedef enum {
	//supervisor context
	SYS_CALL_THREAD	= 0x0 * CALL_GROUP,
	SYS_CALL_MUTEX		= 0x1 * CALL_GROUP,
	SYS_CALL_EVENT		= 0x2 * CALL_GROUP,
	SYS_CALL_SEMAPHORE= 0x3 * CALL_GROUP,
	SYS_CALL_QUEUE		= 0x4 * CALL_GROUP,
	//system context
	SYS_CALL_TIME		= CALL_CONTEXT + 0x0 * CALL_GROUP,
	SYS_CALL_MEM		= CALL_CONTEXT + 0x1 * CALL_GROUP,
	SYS_CALL_DBG		= CALL_CONTEXT + 0x2 * CALL_GROUP
}SYS_CALL_SUPERVISOR;

typedef enum {
	THREAD_CREATE = SYS_CALL_THREAD,
	THREAD_UNFREEZE,
	THREAD_FREEZE,
	THREAD_GET_CURRENT,
	THREAD_SET_PRIORITY,
	THREAD_DESTROY,
	THREAD_SLEEP
#if (KERNEL_PROFILING)
					,
	THREAD_SWITCH_TEST,
	THREAD_STAT,
	STACK_STAT
#endif
}THREAD_SYS_CALLS;

typedef enum {
	MUTEX_CREATE = SYS_CALL_MUTEX,
	MUTEX_LOCK,
	MUTEX_UNLOCK,
	MUTEX_DESTROY
}MUTEX_SYS_CALLS;

typedef enum {
	EVENT_CREATE = SYS_CALL_EVENT,
	EVENT_PULSE,
	EVENT_SET,
	EVENT_IS_SET,
	EVENT_CLEAR,
	EVENT_WAIT,
	EVENT_DESTROY
}EVENT_SYS_CALLS;

typedef enum {
	SEMAPHORE_CREATE = SYS_CALL_SEMAPHORE,
	SEMAPHORE_WAIT,
	SEMAPHORE_SIGNAL,
	SEMAPHORE_DESTROY
}SEMAPHORE_SYS_CALLS;

typedef enum {
	QUEUE_CREATE = SYS_CALL_QUEUE,
	QUEUE_ALLOCATE_BUFFER,
	QUEUE_PUSH,
	QUEUE_PULL,
	QUEUE_RELEASE_BUFFER,
	QUEUE_IS_EMPTY,
	QUEUE_IS_FULL,
	QUEUE_DESTROY
}QUEUE_SYS_CALLS;

typedef enum {
	TIME_GET_SYS_TIME = SYS_CALL_TIME,
	TIME_SET_SYS_TIME,
	TIME_GET_UPTIME
}TIME_SYS_CALLS;

typedef enum {
	MEM_ALLOCATE = SYS_CALL_MEM,
	MEM_FREE,
	POOL_ALLOCATE,
	POOL_FREE,
	POOL_SELECT,
	POOL_SELECT_GLOBAL
#if (KERNEL_PROFILING)
					,
	MEM_STAT
#endif
}MEM_SYS_CALLS;

typedef enum {
	DBG_WRITE = SYS_CALL_DBG,
	DBG_PUSH
}DBG_SYS_CALLS;

#endif // SYS_CALLS_H
