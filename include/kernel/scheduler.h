/**
 * @file scheduler.h
 * @brief Cooperative interface for the kernel scheduler.
 *
 * The scheduler module provides:
 *  - A ready queue of runnable threads
 *  - Thread creation for user-mode entry points
 *  - Context switching driven by timer IRQ and syscall exit
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <kernel/threads.h>
#include <kernel/exc_frame_layout.h>

/**
 * @brief Initialize scheduler state and create the idle thread.
 *
 * Must be called after @ref threads_init and before @ref scheduler_start.
 */
void scheduler_init(void);

/**
 * @brief Start running threads (never returns).
 *
 * Picks the first runnable thread (or idle) and transfers control to the
 * assembly entry routine that performs the initial "return to user" sequence.
 */
void scheduler_start(void);

/**
 * @brief Timer tick hook: preempt current thread and run next.
 *
 *
 * @param frame Pointer to the live exception frame of the interrupted context.
 */
void scheduler_on_timer(struct exc_frame *frame);

/**
 * @brief Create a new user thread and enqueue it.
 *
 * @param func     User-mode entry function.
 * @param arg      Optional argument blob (may be NULL).
 * @param arg_size Size of argument blob in bytes.
 */
void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned arg_size);

/**
 * @brief Create a new user process (new address space) and enqueue it.
 *
 * @param func     User-mode entry function.
 * @param arg      Optional argument blob (may be NULL).
 * @param arg_size Size of argument blob in bytes.
 */
void scheduler_process_create(void (*func)(void *), const void *arg, unsigned arg_size);

/**
 * @brief Thread-exit hook: reap current thread and run next.
 *
 * @param frame Pointer to the live exception frame being used for return.
 */
void scheduler_on_thread_exit(struct exc_frame *frame);

/**
 * @brief Return the currently running thread (or NULL before start).
 */
thread_t *scheduler_curr(void);

/**
 * @brief Block the current thread on I/O and switch to the next runnable thread.
 */
void scheduler_blocked_on_io(struct exc_frame *frame);

/**
 * @brief Wake one thread blocked on I/O.
 *
 * @param c Character to deliver to the woken thread.
 */
void scheduler_wake_blocked_on_io(char c);

/**
 * @brief Check whether the I/O wait queue is empty.
 */
bool is_io_queue_empty(void);

/**
 * @brief Update the sleep queue and wake expired sleepers.
 */
void scheduler_update_sleep_q(void);

/**
 * @brief Block current thread on sleep for a number of cycles.
 *
 * @param frame  Pointer to the live exception frame being used for return.
 * @param cycles Number of cycles to sleep.
 */
void scheduler_blocked_on_sleep(struct exc_frame *frame, unsigned cycles);
#endif /* SCHEDULER_H */
