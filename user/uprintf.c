#include <user/uprintf.h>
#include <user/syscalls.h>

void uprintf [[gnu::format(printf, 1, 2)]] (const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    syscall_vprintf(fmt, &ap);
    va_end(ap);
}
