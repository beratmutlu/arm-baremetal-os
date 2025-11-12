#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdbool.h>

enum exc_kind {
    EXC_UND,
    EXC_SVC,
    EXC_PABT,
    EXC_DABT,
    EXC_IRQ
};

struct exc_frame;

void und_handler_c[[noreturn]](struct exc_frame* frame);
void svc_handler_c[[noreturn]](struct exc_frame* frame);
void pabt_handler_c[[noreturn]](struct exc_frame* frame);
void dabt_handler_c[[noreturn]](struct exc_frame* frame);
void irq_handler_c(struct exc_frame* frame);

#endif