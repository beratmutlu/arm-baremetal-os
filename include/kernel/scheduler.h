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
 * @brief Thread-exit hook: reap current thread and run next.
 *
 * @param frame Pointer to the live exception frame being used for return.
 */
void scheduler_on_thread_exit(struct exc_frame *frame);

/**
 * @brief Return the currently running thread (or NULL before start).
 */
thread_t *scheduler_curr(void);

#endif /* SCHEDULER_H */