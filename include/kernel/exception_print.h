#ifndef EXCEPTION_PRINT_H
#define EXCEPTION_PRINT_H

#include <kernel/exceptions.h>

struct exc_frame;

void print_exception_infos(const enum exc_kind kind,
                          const struct exc_frame* frame);

#endif