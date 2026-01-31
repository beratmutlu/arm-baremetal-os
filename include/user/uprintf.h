#ifndef UPRINTF_H
#define UPRINTF_H

#include <stdarg.h>

void uprintf [[gnu::format(printf, 1, 2)]] (const char *fmt, ...);

#endif