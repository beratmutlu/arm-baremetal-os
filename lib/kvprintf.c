
/**
 * @file kvprintf.c
 * @brief Implementation of kvprintf using fmt helpers.
 */

#include <lib/kvprintf.h>
#include <lib/fmt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/** 
 * @brief Only width specifier accepted by the parser.
 */
#define ACCEPTED_WIDTH 8u

/** 
 * @brief Buffer size for temporary formatting operations.
 * 
 */
#define FMT_TMP_BUFSZ  32u

/** @brief Error message emitted for unsupported format specifiers. */
static const char *const K_UNKNOWN = "Unknown conversion specifier";

/**
 * @brief Parse state for format string processing.
 */
struct parse_state
{
    bool zero_pad;          /**< True if '0' flag was present. */
    bool saw_any_modifier;  /**< True if any flag or width was seen. */
    bool saw_width;         /**< True if width digits were parsed. */
    bool unknown_width;     /**< True if width != 8 (unsupported). */
    size_t width;           /**< Parsed width value. */
    char conv;              /**< Conversion specifier character. */
    struct fmt_spec spec;   /**< Resolved formatting specification. */
};


/**
 * @brief Emit a single character through the output callback.
 * 
 * @param out   Output callback function
 * @param ctx   User context for callback
 * @param c     Character to emit
 * @param count Pointer to counter (incremented after emission)
 */
static inline void emit(kputc_fn out, void *ctx, char c, int *count) {
    out((unsigned char)c, ctx);
    (*count)++;
}

/**
 * @brief Emit a \\0-terminated string through the output callback.
 * 
 * @param out   Output callback function
 * @param ctx   User context for callback
 * @param str   String to emit (must be \\0-terminated)
 * @param count Pointer to counter (incremented for each character)
 */
static inline void str_emit(kputc_fn out, void *ctx, const char *str, int *count) {
    while (*str != '\0') {
        emit(out, ctx, *str, count);
        str++;
    }
}

/**
 * @brief Emit the "Unknown conversion specifier" error message.
 * 
 * @param out   Output callback function
 * @param ctx   User context for callback
 * @param count Pointer to counter (incremented for each character)
 */
static inline void emit_unknown(kputc_fn out, void *ctx, int *count) {
    str_emit(out, ctx, K_UNKNOWN, count);
}

/**
 * @brief Parse format specifier flags and width from format string.
 * 
 * @param[in,out] pfmt   Pointer to format string pointer (updated to point after conversion char)
 * @param[out]    parsed Populated with parsing results
 */
static inline void parse_format(const char **pfmt, struct parse_state *parsed) {
    *parsed = (struct parse_state) {0};
    parsed->spec = fmt_spec_default();
    
    const char *s = *pfmt;
    
    /* Check for '0' flag (zero-padding) */
    if (*s == '0') {
        parsed->zero_pad = true;
        parsed->saw_any_modifier = true;
        s++;
    }

    /* Parse width digits (if any) */
    size_t width_acc = 0;
    bool have_width = false;

    /* Record width and check if it's supported
     * Currently only width=8 is implemented (for "%08x" style)
     */
    while (*s >= '0' && *s <= '9') {
        have_width = true;
        parsed->saw_any_modifier = true;
        width_acc = width_acc * 10u + (size_t)(*s - '0');
        s++;
    }
    if (have_width) {
        parsed->saw_width = true;
        parsed->width = width_acc;
        if (width_acc != (size_t)ACCEPTED_WIDTH) {
            parsed->unknown_width = true;   /* Will trigger error later */
        }
    }

    /* Extract conversion specifier character */
    parsed->conv = *s != '\0' ? *s++ :   '\0';

    /* Populate the fmt_spec structure for the formatter */
    parsed->spec.min_width = parsed->width;
    parsed->spec.pad_char = parsed->zero_pad ? '0' : ' ';

    *pfmt = s;  /* Advance caller's pointer past the entire specifier */
}

/**
 * @brief Check if parsed format has invalid width combination.
 * 
 * @param parsed Parse state to check
 * @return true if width combination is invalid, false otherwise
 */
static inline bool is_invalid_width_combo(const struct parse_state parsed) {
    return parsed.unknown_width || (parsed.saw_any_modifier && !parsed.saw_width);
}

/**
 * @brief Format and emit a 32-bit integer value.
 *
 * @param out         Output callback function
 * @param ctx         User context for callback
 * @param count       Pointer to character counter (incremented)
 * @param num         Number to format
 * @param spec        Formatting specification
 * @param is_unsigned True for unsigned formatting, false for signed
 * 
 * @return 0 on success, -1 on formatting error
 */
static inline int format_and_emit_32(kputc_fn out, void *ctx, int *count, uint32_t num, struct fmt_spec spec, bool is_unsigned) {
    
    char buf[FMT_TMP_BUFSZ];
    int len;
    if (is_unsigned) {
        len = fmt_u32(num, buf, sizeof(buf), spec);
    } else {
        len = fmt_i32_dec(num, buf, sizeof(buf), spec);
    }

    /* Formatting failure: buffer too small or invalid spec */
    if (len < 0) {
        return -1;
    }
    str_emit(out, ctx, buf, count);
    return 0;
}

int kvprintf(kputc_fn out, void *ctx, const char *fmt, va_list ap){
    int count = 0;

    /* Main parsing loop: process format string character by character */
    while (*fmt) {
         /* Literal character: emit directly */
        if (*fmt != '%') {
            out(*fmt++, ctx);
            count++;
            continue;
        } else {
            /* Format specifier: parse flags, width, and conversion character */
            fmt++;
            struct parse_state parsed;
            parse_format(&fmt, &parsed);

            switch (parsed.conv)
            {
            case '\0': {
                /* Premature end of format string */
                emit_unknown(out, ctx, &count);
                break;
            }
            case 'c': {
                /* Character conversion: no modifiers allowed */
                if (parsed.saw_any_modifier) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                int ch = va_arg(ap, int); 
                emit(out, ctx, (char)ch, &count);
                break;
            }
            case 's': {
                /* String conversion: no modifiers allowed */
                if (parsed.saw_any_modifier) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                const char *str = va_arg(ap, const char*);
                if(!str) {
                    str = "(null)"; /* NULL pointer safety */
                }
                str_emit(out, ctx, str, &count);
                break;
            }
            case 'i': {
                 /* Signed decimal: allow width=8 with optional zero-padding */
                if (is_invalid_width_combo(parsed)) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                int num = va_arg(ap, int);
                parsed.spec.base = BASE_DECIMAL;
                int result = format_and_emit_32(out, ctx, &count, num, parsed.spec, false);
                if (result < 0) {
                    return -1;   /* Internal formatting error */
                }
                break;
            }
            case 'u': {
                /* Unsigned decimal: allow width=8 with optional zero-padding */
                if (is_invalid_width_combo(parsed)) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                unsigned num = va_arg(ap, unsigned);
                parsed.spec.base = BASE_DECIMAL;
                int result = format_and_emit_32(out, ctx, &count, num, parsed.spec, true);
                if (result < 0) {
                    return -1;
                }
                break;
            }
            case 'x': {
                /* Hexadecimal: allow width=8 with optional zero-padding */
                if (is_invalid_width_combo(parsed)) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                unsigned num = va_arg(ap, unsigned);
                parsed.spec.base = BASE_HEX;
                int result = format_and_emit_32(out, ctx, &count, num, parsed.spec, true);
                if (result < 0) {
                    return -1;
                }
                break;
            }
            case 'p': {
                /* Pointer: always 0x%08x format, no modifiers allowed */
                if (parsed.saw_any_modifier) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                void *ptr = va_arg(ap, void *);
                uintptr_t addr = (uintptr_t)ptr;

                parsed.spec = fmt_spec_pointer();
                int result = format_and_emit_32(out, ctx, &count, addr, parsed.spec, true);
                if (result < 0) {
                    return -1;
                }
                break;
            }
            case '%': {
                /* Literal percent sign: no modifiers allowed */
                if (parsed.saw_any_modifier) {
                    emit_unknown(out, ctx, &count);
                    break;
                }
                emit(out, ctx, '%', &count);
                break;
            }
            default: {
                /* Unknown conversion specifier */
                emit_unknown(out, ctx, &count);
                break;
            }
            }
        }
    }
    return count;
}
