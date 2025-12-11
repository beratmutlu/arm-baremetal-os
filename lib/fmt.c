/**
 * @file fmt.c
 * @brief Integer-to-string formatting helpers (implementation).
 */


#include <lib/fmt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/** 
 * @brief Buffer size for digit conversion in any supported base.
 */
#define MAX_UINT32_DIGITS 32


/**
 * @brief In-place reverse of a substring.
 * Reverses the range [offset, offset+len) inside @p str.
 * @param str    Buffer containing the string
 * @param offset Starting index of range to reverse  
 * @param len    Number of characters to reverse
 */
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

/**
 * @brief Convert unsigned magnitude to string in the given base.
 * 
 * Writes digits into @p buf and \\0-terminates; returns length (excl. \\0) or -1.
 * Caller ensures @p cap >= 2. Supports BASE_DECIMAL and BASE_HEX.
 * @param mag  Unsigned integer to convert
 * @param buf  Output buffer (receives \\0-terminated string)
 * @param base Numeric base (10 or 16)
 * @param cap  Buffer capacity in bytes
 * 
 * @return Number of characters written (excluding \\0), or -1 on error. 
 */
static int utoa_base(uint32_t mag, /*out*/ char* buf, enum fmt_base base, size_t cap){
    if (!buf || cap < 2) {
        return -1;
    }

    static const char digits[] = "0123456789abcdef";

    /* Special case: zero */
    if (mag == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    size_t len = 0;

    /* Extract digits in reverse order (least significant first)
     * Example: 123 → '3', '2', '1' → will be reversed to "123"
     */
    while (mag != 0) {
        if (len + 1 >= cap) {
            return -1;  /* Need room for \0 */
        }

        uint32_t digit = mag % base;    /* Extract least significant digit */
        mag /= base;                    /* Remove it from magnitude */
        buf[len++] = digits[digit];
    }

    buf[len] = '\0';

    /* Reverse to get correct order */
    strrev_range(buf, 0, len);
    return (int)len;
}

/**
 * @brief Emit optional "0x" prefix and sign into buffer.
 * Writes prefix (if requested) and sign character at the current position.
 * 
 * @param[in,out] pp         Pointer to current position in buffer (updated)
 * @param         prefix_len Length of prefix (0 or 2 for "0x")
 * @param         sign_ch    Sign character ('-' or 0 for none)
 */
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

/**
 * @brief Assemble the final field: padding, prefix/sign, digits, and \\0.
 * Returns total characters written (excl. \\0) or -1 on insufficient capacity.
 * 
 * @param mag     Unsigned magnitude to format
 * @param buf     Output buffer
 * @param cap     Buffer capacity in bytes
 * @param sign_ch Sign character ('-' or 0 for none)
 * @param spec    Formatting specification (base, width, padding)
 * 
 * @return Length of formatted string (excluding \\0), or -1 if insufficient capacity.
 */
static int format_field(uint32_t mag, /*out*/ char* buf, size_t cap, char sign_ch, struct fmt_spec spec){
    if (!buf || cap == 0) {
        return -1;
    }

    /* Convert magnitude to digit string */
    char tmp[MAX_UINT32_DIGITS];
    int dlen = utoa_base(mag, tmp, spec.base, sizeof(tmp));
    if (dlen < 0){
        return -1;
    }

    /* Calculate component sizes */
    const size_t prefix_len = (spec.prefix_0x && spec.base == BASE_HEX) ? 2 : 0;
    const size_t sign_len = (sign_ch != 0) ? 1 : 0;
    const size_t total_len = (size_t)dlen + sign_len;

    /* Calculate padding needed to reach min_width */
    size_t pad_count = 0;
    if (spec.min_width > total_len) {
        pad_count = spec.min_width - total_len;
    }

    /* Validate buffer capacity
     * Layout: [space_pad] [prefix] [sign] [zero_pad] [digits] [\\0]
     * Note: Either space_pad OR zero_pad is used, never both. 
     * Also either prefix OR sign used, as prefix is only for pointer (unsigned) values.
     */
    size_t need = pad_count + prefix_len + total_len + 1; /* +1 for \\0 */
    if (cap < need) {
        return -1;
    }

    char *p = buf;

    /* Space padding goes before everything */
    if (spec.pad_char == ' ') {
        memset(p, ' ', pad_count);
        p += pad_count;
    }

    /* Sign and prefix */
    emit_prefix_and_sign(&p, (int)prefix_len, sign_ch);

    /* Zero padding goes after prefix/sign but before digits */
    if (spec.pad_char != ' ') {
        memset(p, '0', pad_count);
        p += pad_count;
    }
    
    /* Digits */
    memcpy(p, tmp, (size_t)dlen); 
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
