#ifndef STACKS_H
#define STACKS_H

#include <stdint.h>

extern unsigned char __stack_svc_end[];
extern unsigned char __stack_irq_end[];
extern unsigned char __stack_abt_end[];
extern unsigned char __stack_und_end[];
extern unsigned char __stack_sys_end[];

void arm_init_stacks(void);

extern unsigned char vectors_table[];

void arm_set_vbar(void *base);

#endif