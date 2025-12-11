
/**
 * @file syscalls.h
 * @defgroup syscall_api Syscalls
 * @brief User <-> kernel syscall identifiers and prototypes.
 *
 * This header defines syscall IDs used by your traps/dispatch logic and
 * exposes C prototypes for callable syscall entry stubs.
 *
 * In your scheduler, @ref syscall_exit is used as the initial LR of a newly
 * created user thread so that returning from the entry function cleanly exits
 * the thread.
 * @{
 */
#ifndef SYSCALL_H
#define SYSCALL_H

/** Syscall ID: exit current thread. */
#define SYSCALL_ID_EXIT          1
#ifndef __ASSEMBLER__

/**
 * @brief Terminate the current thread (does not return).
 */
void syscall_exit [[noreturn]] (void);
#endif /* __ASSEMBLER__ */

#endif /* SYSCALL_H */
/** @} */ /* end of syscall_api */