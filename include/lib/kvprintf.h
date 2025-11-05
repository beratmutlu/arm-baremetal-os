
/**
 * @file kvprintf.h
 * @brief Minimal kprintf core with pluggable byte sink.
 *
 * Supported conversions: %c, %s, %i (decimal), %u (decimal), %x (hex), %p, %%.
 * Modifiers: optional leading '0' and width=8 (only) for %i/%u/%x.
 * %s with NULL prints "(null)".
 * Returns total characters emitted, or -1 on internal formatting error.
 *
 * @defgroup kprintf_api kprintf core API (public)
 * @brief kvprintf with kputc_fn sink.
 * @{
 */

#ifndef KVPRINTF_H
#define KVPRINTF_H

#include <stdarg.h>

typedef void (*kputc_fn)(char c, void *ctx);

/**
 * @brief Format and emit into a byte sink.
 * @param out  Byte-emitter callback (must not be NULL).
 * @param ctx  User context passed to @p out (may be NULL).
 * @param fmt  Format string (must not be NULL).
 * @param ap   Varargs list.
 * @return Characters emitted, or -1 on internal formatting error.
 */
int kvprintf(kputc_fn out, void *ctx, const char *fmt, va_list ap);

#endif /* KVPRINTF_H */
/** @} */ /* end of kprintf_api */