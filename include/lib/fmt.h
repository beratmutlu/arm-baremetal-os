/**
 * @file fmt.h
 * @brief Integer-to-string formatting helpers.
 *
 * Minimal, allocation-free formatting for kernel use.
 */


/** @defgroup fmt_api Formatting API
 *  @brief Conversion helpers (e.g. fmt_i32_dec, fmt_u32).
 *  @{
 */

#ifndef FMT_H
#define FMT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @enum fmt_base
 * @brief Numeric base for formatting.
 */
enum fmt_base {
    BASE_DECIMAL = 10,
    BASE_HEX     = 16,
};

/**
 * @struct fmt_spec
 * @brief Formatting specification.
 *
 * Typical usage:
 *   struct fmt_spec sp = fmt_spec_default();
 *   sp.min_width = 8;
 *   sp.pad_char  = '0';
 */
struct fmt_spec
{
    enum fmt_base base;
    size_t min_width;   
    bool prefix_0x;
    char pad_char;
};

/**
 * @brief Return the default formatting spec.
 * @return Default spec (base=decimal, width=0, no prefix, space padding).
 */
static inline struct fmt_spec fmt_spec_default(void) {
    return (struct fmt_spec){
        .base = BASE_DECIMAL, 
        .min_width = 0, 
        .prefix_0x = false, 
        .pad_char = ' ' };
}

/**
 * @brief Return the pointer (0x%08x) formatting spec.
 * @return Pointer spec (base=hex, width=8, 0x-prefix, 0-padding).
 */
static inline struct fmt_spec fmt_spec_pointer(void) {
    return (struct fmt_spec) {
        .base = BASE_HEX,
        .min_width = 8,
        .prefix_0x = true,
        .pad_char = '0' };
}

/**
 * @brief Format a 32-bit signed integer in **decimal** using the given spec.
 *
 * Contract:
 *  - Writes a \\0-terminated string to @p buf on success.
 *  - Returns the number of characters **excluding** the terminating \\0.
 *  - Returns -1 if @p cap is insufficient for the full result (no partial write).
 *
 * @param num  The number to be formatted.
 * @param buf  Output buffer.
 * @param cap  Size of @p buf in bytes.
 * @param spec Formatting options (only min_width and pad_char are used here).
 * @return Length (without \\0) on success, -1 on error.
 */
int fmt_i32_dec(int32_t num, char *buf, size_t cap, const struct fmt_spec spec);


/**
 * @brief Format a 32-bit unsigned integer using @p spec.base (10 or 16).
 *
 * Contract:
 *  - Writes a \\0-terminated string to @p buf on success.
 *  - Returns the number of characters **excluding** the terminating \\0.
 *  - Returns -1 if @p cap is insufficient (no partial write).
 *
 * @param num  The number to be formatted.
 * @param buf  Output buffer.
 * @param cap  Size of @p buf in bytes.
 * @param spec Formatting options (base, width, 0x prefix, padding).
 * @return Length (without \\0) on success, -1 on error.
 */
int fmt_u32(uint32_t num, char *buf, size_t cap, const struct fmt_spec spec);

#endif /* FMT_H */
/** @} */ /* end of fmt_api */