#ifndef STACKS_H
#define STACKS_H

#include <stdint.h>

void arm_init_stacks(void);

extern unsigned char vectors_table[];

void arm_set_vbar(void *base);

#endif