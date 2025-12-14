
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
#define SYSCALL_ID_EXIT          2

#ifndef __ASSEMBLER__

/**
 * @brief Terminate the current thread (does not return).
 */
void syscall_exit [[noreturn]] (void);
#endif /* __ASSEMBLER__ */

#endif /* SYSCALL_H */