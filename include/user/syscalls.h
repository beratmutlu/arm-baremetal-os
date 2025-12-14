
/**
 * @file syscalls.h
 * @brief User <-> kernel syscall identifiers and prototypes.
 *
 * This header defines syscall IDs used by your traps/dispatch logic and
 * exposes C prototypes for callable syscall entry stubs.
 */
#ifndef SYSCALL_H
#define SYSCALL_H

/** Syscall ID: exit current thread. */
#define SYSCALL_ID_EXIT          1
#define SYSCALL_ID_CREATE        2
#define SYSCALL_ID_GETC          3
#define SYSCALL_ID_PUTC          4
#define SYSCALL_ID_SLEEP         5
#define SYSCALL_ID_UND           6

#ifndef __ASSEMBLER__

/**
 * @brief Terminate the current thread (does not return).
 */
void syscall_exit [[noreturn]] (void);

void syscall_create_thread(void (*f) (void *), void * args, unsigned int arg_size);

char syscall_getc(void);

void syscall_putc(char c);

void syscall_sleep(unsigned int cycles);

void syscall_undefined(void);
#endif /* __ASSEMBLER__ */

#endif /* SYSCALL_H */