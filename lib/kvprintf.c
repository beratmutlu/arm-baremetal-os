#include <lib/kvprintf.h>
#include <lib/fmt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define BASE_DEFAULT BASE_DECIMAL
#define PREFIX_DEFAULT false

static inline void emit(kputc_fn out, void *ctx, char c, int *count) {
    out((unsigned char)c, ctx);
    (*count)++;
}

static inline void str_emit(kputc_fn out, void *ctx, const char *str, int *count) {
    while (*str != '\0') {
        emit(out, ctx, *str, count);
        str++;
    }
}
/*
int kvprintf(kputc_fn out, void *ctx, const char *fmt, va_list ap){
    int count = 0;
    while (*fmt) {
        if (*fmt != '%') {
            out(*fmt++, ctx);
            count++;
            continue;
        } else {
            fmt++;
            bool modified_flag = false;
            bool modified_width = false;
            bool zero_pad = false;
            bool unk = false;
            if (*fmt == '0') {
                zero_pad = true;
                modified_flag = true;
                fmt++;
            }
            size_t width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = (width * 10) + (*fmt - '0');
                modified_flag = true;
                if (width != 8) {
                    unk = true;
                } else {
                    modified_width = true;
                }
                fmt++;
            }
            char conv = *fmt ? *fmt++ : '\0';
            struct fmt_spec spec = {
                .base = BASE_DEFAULT,
                .min_width = width,
                .pad_char = zero_pad ? '0' : ' ',
                .prefix_0x = PREFIX_DEFAULT
            };
            switch (conv)
            {
            case '\0': {
                str_emit(out, ctx, "Unknown conversion specifier", &count);
                break;
            }
            case 'c': {
                if (modified_flag) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                int ch = va_arg(ap, int); 
                emit(out, ctx, (char)ch, &count);
                break;
            }
            case 's': {
                if (modified_flag) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                const char *str = va_arg(ap, const char*);
                if(!str) {
                    str = "(null)";
                }
                str_emit(out, ctx, str, &count);
                break;
            }
            case 'i': {
                if (unk || (modified_flag && !modified_width)) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                int num = va_arg(ap, int);
                char buf[32];
                int len = fmt_i32_dec(num, buf, sizeof(buf), spec);
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'u': {
                if (unk || (modified_flag && !modified_width)) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                unsigned num = va_arg(ap, unsigned);
                char buf[32];
                int len = fmt_u32(num, buf, sizeof(buf), spec);
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'x': {
                if (unk || (modified_flag && !modified_width)) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                unsigned num = va_arg(ap, unsigned);
                spec.base = BASE_HEX;
                char buf[32];
                int len = fmt_u32(num, buf, sizeof(buf), spec);
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'p': {
                if (modified_flag) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                void *p = va_arg(ap, void *);
                uintptr_t addr = (uintptr_t)p;

                char buf[32];
                spec.base = BASE_HEX;
                spec.prefix_0x = true;
                spec.min_width = 8;
                spec.pad_char = '0';
                int len = fmt_u32(addr, buf, sizeof(buf), spec);
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case '%': {
                if (modified_flag) {
                    str_emit(out, ctx, "Unknown conversion specifier", &count);
                    break;
                }
                emit(out, ctx, '%', &count);
                break;
            }
            default: {
                str_emit(out, ctx, "Unknown conversion specifier", &count);
                break;
            }
            }
        }
    }
    return count;
}
*/

int kvprintf(kputc_fn out, void *ctx, const char *fmt, va_list ap){
    int count = 0;
    while (*fmt) {
        if (*fmt != '%') {
            out(*fmt++, ctx);
            count++;
            continue;
        } else {
            fmt++;
            switch (*fmt)
            {
            case '\0':{
                str_emit(out, ctx, "Unknown conversion specifier", &count);
                break;
            }
            case  '0':{
                fmt++;
                switch (*fmt)
                {
                    case '\0':{
                        str_emit(out, ctx, "Unknown conversion specifier", &count);
                        break;
                    }
                    case '8': {
                        fmt++;
                        switch (*fmt){
                            case '\0': {
                                str_emit(out, ctx, "Unknown conversion specifier", &count);
                                break;
                            }
                            case 'i': {
                                fmt++;
                                int num = va_arg(ap, int);
                                char buf[32];
                                int len = fmt_i32_dec(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 8, false, '0'});
                                if (len < 0) return -1;
                                str_emit(out, ctx, buf, &count);
                                break;
                            }
                            case 'u': {
                                fmt++;
                                unsigned num = va_arg(ap, unsigned);
                                char buf[32];
                                int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 8, false, '0'});
                                if (len < 0) return -1;
                                str_emit(out, ctx, buf, &count);
                                break;
                            }
                            case 'x': {
                                fmt++;
                                unsigned num = va_arg(ap, unsigned);
                                char buf[32];
                                int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_HEX, 8, false, '0'});
                                if (len < 0) return -1;
                                str_emit(out, ctx, buf, &count);
                                break;
                            }
                            default: {
                                fmt++;
                                str_emit(out, ctx, "Unknown conversion specifier", &count);
                                break;
                            }
                        }
                        break;
                    }
                    default: {
                        fmt++;
                        str_emit(out, ctx, "Unknown conversion specifier", &count);
                        break;
                    }
                }
            break;
        }
            case '8': {
                fmt++;
                switch (*fmt)
                {
                    case '\0': {
                        str_emit(out, ctx, "Unknown conversion specifier", &count);
                        break;
                    }
                    case 'i': {
                        fmt++;
                        int num = va_arg(ap, int);
                        char buf[32];
                        int len = fmt_i32_dec(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 8, false, ' '});
                        if (len < 0) return -1;
                        str_emit(out, ctx, buf, &count);
                        break;
                    }
                    case 'u': {
                        fmt++;
                        unsigned num = va_arg(ap, unsigned);
                        char buf[32];
                        int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 8, false, ' '});
                        if (len < 0) return -1;
                        str_emit(out, ctx, buf, &count);
                        break;
                    }
                    case 'x': {
                        fmt++;
                        unsigned num = va_arg(ap, unsigned);
                        char buf[32];
                        int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_HEX, 8, false, ' '});
                        if (len < 0) return -1;
                        str_emit(out, ctx, buf, &count);
                        break;
                    }
                    default: {
                        fmt++;
                        str_emit(out, ctx, "Unknown conversion specifier", &count);
                        break;
                    }
                }
                break;
            }
            case 'i': {
                fmt++;
                int num = va_arg(ap, int);
                char buf[32];
                int len = fmt_i32_dec(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 0, false, ' '});
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'u': {
                fmt++;
                unsigned num = va_arg(ap, unsigned);
                char buf[32];
                int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_DECIMAL, 0, false, ' '});
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'x': {
                fmt++;
                unsigned num = va_arg(ap, unsigned);
                char buf[32];
                int len = fmt_u32(num, buf, sizeof(buf), (struct fmt_spec){BASE_HEX, 0, false, ' '});
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case 'c': {
                fmt++;
                int ch = va_arg(ap, int); 
                emit(out, ctx, (char)ch, &count);
                break;
            }
            case 's': {
                fmt++;
                const char *str = va_arg(ap, const char*);
                if(!str) {
                    str = "(null)";
                }
                str_emit(out, ctx, str, &count);
                break;
            }
            case 'p': {
                fmt++;
                void *p = va_arg(ap, void *);
                uintptr_t addr = (uintptr_t)p;
                char buf[32];
                int len = fmt_u32((uint32_t) addr, buf, sizeof(buf), (struct fmt_spec){BASE_HEX, 8, true, '0'});
                if (len < 0) return -1;
                str_emit(out, ctx, buf, &count);
                break;
            }
            case '%': {
                fmt++;
                emit(out, ctx, '%', &count);
                break;
            }
            default: {
                fmt++;
                str_emit(out, ctx, "Unknown conversion specifier", &count);
                break;
            }
            }
        }
    }
    return count;
}
