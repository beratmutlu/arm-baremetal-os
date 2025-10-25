#ifndef KVPRINTF_H
#define KVPRINTF_H

#include <stdarg.h>

typedef void (*kputc_fn)(char c, void *ctx);

int kvprintf(kputc_fn out, void *ctx, const char *fmt, va_list ap);

#endif
