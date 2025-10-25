#ifndef FMT_H
#define FMT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum fmt_base {
    BASE_DECIMAL = 10,
    BASE_HEX     = 16,
};

struct fmt_spec
{
    enum fmt_base base;
    size_t min_width;
    bool prefix_0x;
    char pad_char;
};

static inline struct fmt_spec fmt_spec_default(void) {
    return (struct fmt_spec){ .min_width = 0, .prefix_0x = false, .pad_char = ' ' };
}


int fmt_i32_dec(int32_t num, char *buf, size_t cap, const struct fmt_spec spec);

int fmt_u32(uint32_t num, char *buf, size_t cap, const struct fmt_spec spec);

#endif
