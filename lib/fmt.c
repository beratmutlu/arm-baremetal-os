#include <lib/fmt.h>
#include <string.h>
#include <stdbool.h>

#define MAX_UINT32_DIGITS 32


static void strrev_range(char *str, size_t offset, size_t len) {
    if (len <= 1) {
        return;
    }

    for (size_t i = offset, j = len + offset - 1; i < j; i++, j--) {
        char tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}

static int utoa_base(uint32_t mag, /*out*/ char* buf, enum fmt_base base, size_t cap){
    if (!buf || cap < 2) {
        return -1;
    }

    static const char digits[] = "0123456789abcdef";

    
    if (mag == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    size_t len = 0;

    while (mag != 0) {
        if (len + 1 >= cap) {
            return -1;
        } // Buffer too small

        uint32_t digit = mag % base;
        mag /= base;
        buf[len++] = digits[digit];
    }

    buf[len] = '\0';

    // Reverse to correct order
    strrev_range(buf, 0, len);

    return (int)len;
}

static inline void emit_prefix_and_sign(char **pp, int prefix_len, char sign_ch) {
    char *p = *pp;

    if (prefix_len) { 
        *p++ = '0'; 
        *p++ = 'x'; 
    }

    if (sign_ch != 0) { 
        *p++ = sign_ch; 
    }
    *pp = p;
}

static int format_field(uint32_t mag, /*out*/ char* buf, size_t cap, char sign_ch, struct fmt_spec spec){
    if (!buf || cap == 0) {
        return -1;
    }

    char tmp[MAX_UINT32_DIGITS];
    int dlen = utoa_base(mag, tmp, spec.base, sizeof(tmp));
    if (dlen < 0){
        return -1;
    }

    const size_t prefix_len = (spec.prefix_0x && spec.base == BASE_HEX) ? 2 : 0;
    const size_t sign_len = (sign_ch != 0) ? 1 : 0;
    const size_t total_len = (size_t)dlen + sign_len + prefix_len;

    size_t pad_count = 0;
    if (spec.min_width > total_len) {
        pad_count = spec.min_width - total_len;
    }

    size_t need = pad_count + total_len + 1;
    if (cap < need) {
        return -1;
    }

    char *p = buf;

    // Space padding goes before everything
    if (spec.pad_char == ' ') {
        memset(p, ' ', pad_count);
        p += pad_count;
    }

    // Sign and prefix
    emit_prefix_and_sign(&p, prefix_len, sign_ch);

    // Zero padding goes after prefix but before digits
    if (spec.pad_char != ' ') {
        memset(p, '0', pad_count);
        p += pad_count;
    }
    
    // Copy digits
    memcpy(p, tmp, dlen); 
    p += dlen;

    *p = '\0';

    return (int)(need - 1);

}

int fmt_i32_dec(int32_t num, /*out*/ char *buf, size_t cap, struct fmt_spec spec) {
    if (!buf || cap < 2) {
        return -1;
    }


    bool is_negative = (num < 0);
    uint32_t mag;

    if (is_negative) {
        int64_t wide = num;
        mag = (uint32_t)(-wide);
    } else {
        mag = (uint32_t)num;
    }
    char sign_ch = is_negative ? '-': 0;
    
    if (spec.min_width == 0 && sign_ch == 0) {
        return utoa_base(mag, buf, BASE_DECIMAL, cap);
    }
    spec.base = BASE_DECIMAL;
    return format_field(mag, buf, cap, sign_ch, spec);
}

int fmt_u32(uint32_t num, /*out*/ char* buf, size_t cap, struct fmt_spec spec) {
    if (!buf || cap == 0) {
        return -1;
    }

    if (spec.min_width == 0) {
        return utoa_base(num, buf, spec.base, cap);
    } else {
        return format_field(num, buf, cap, 0, spec);
    }
}
