
/**
 * @file syscalls.h
 * @brief User <-> kernel syscall identifiers and prototypes.
 *
 * This header defines syscall IDs used by your traps/dispatch logic and
 * exposes C prototypes for callable syscall entry stubs.
 */
#ifndef SYSCALL_H
#define SYSCALL_H

/** Syscall ID: terminate the calling thread*/
#define SYSCALL_ID_EXIT          1

/** Syscall ID: create a new thread*/
#define SYSCALL_ID_CREATE        2

/** Syscall ID: read a single character from input*/
#define SYSCALL_ID_GETC          3

/** Syscall ID: write a single character to output*/
#define SYSCALL_ID_PUTC          4

/** Syscall ID: suspend the calling thread for a given number of cycles*/
#define SYSCALL_ID_SLEEP         5

/** Syscall ID: undefined syscall*/
#define SYSCALL_ID_UND           6

#define SYSCALL_ID_VPRINTF       8

#ifndef __ASSEMBLER__

/** GPR index containing the syscall ID for the handler */
#define SYSCALL_ID_REG           7   

#include <stdarg.h>

/**
 * Terminate the current thread.
 */
void syscall_exit [[noreturn]] (void);

/**
 * Create a new thread starting at f.
 */
void syscall_create_thread(void (*f) (void *), void * args, unsigned int arg_size);

/**
 * Read one character from input.
 */
char syscall_getc(void);

/**
 * Write one character to output.
 */
void syscall_putc(char c);

/**
 * Sleep for at least the given number of cycles.
 */
void syscall_sleep(unsigned int cycles);


void syscall_vprintf(const char *fmt, va_list *ap);

/**
 * An undefined syscall.
 */
void syscall_undefined(void);
#endif /* __ASSEMBLER__ */

#endif /* SYSCALL_H */