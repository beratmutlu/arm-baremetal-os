
#include <stddef.h>
#include <stdint.h>
#include <kernel/exc_frame_layout.h>

#define PAGE_ALIGN __attribute__((aligned(EXC_STACK_SIZE))) volatile

/* Each stack is sandwiched between guards */
PAGE_ALIGN unsigned char svc_stack_guard_lower[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char svc_stack[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char svc_stack_guard_upper[EXC_STACK_SIZE];

PAGE_ALIGN unsigned char irq_stack_guard_lower[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char irq_stack[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char irq_stack_guard_upper[EXC_STACK_SIZE];

PAGE_ALIGN unsigned char abt_stack_guard_lower[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char abt_stack[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char abt_stack_guard_upper[EXC_STACK_SIZE];

PAGE_ALIGN unsigned char und_stack_guard_lower[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char und_stack[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char und_stack_guard_upper[EXC_STACK_SIZE];

PAGE_ALIGN unsigned char sys_stack_guard_lower[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char sys_stack[EXC_STACK_SIZE];
PAGE_ALIGN unsigned char sys_stack_guard_upper[EXC_STACK_SIZE];
