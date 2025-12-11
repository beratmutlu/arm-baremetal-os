/* scheduler.h */
/**
 * @file scheduler.h
 * @defgroup scheduler_api Scheduler
 * @brief Cooperative interface for the kernel scheduler (A3-style).
 *
 * The scheduler module provides:
 *  - A ready queue of runnable threads
 *  - Thread creation for user-mode entry points
 *  - Context switching driven by timer IRQ and syscall exit
 *
 * The scheduler cooperates with the exception/IRQ entry code via
 * @ref struct exc_frame (see @ref exc_frame_abi). The exception handler passes
 * a pointer to the active exception frame; the scheduler updates this frame
 * in-place to resume the next thread.
 *
 * @{
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
 *
 * @warning This function does not return under normal operation.
 */
void scheduler_start(void);

/**
 * @brief Timer tick hook: preempt current thread and run next.
 *
 * Called from the timer IRQ handler.
 *
 * Semantics:
 *  - Saves current user context into @ref scheduler_curr (if running)
 *  - Enqueues the previous thread if it is not the idle thread
 *  - Picks the next runnable thread (or idle)
 *  - Restores that thread into @p frame so that exception return resumes it
 *
 * @param frame Pointer to the live exception frame of the interrupted context.
 */
void scheduler_on_timer(struct exc_frame *frame);

/**
 * @brief Create a new user thread and enqueue it.
 *
 * The created thread begins execution at @p func in user mode with:
 *  - r0 = pointer to an argument copy (or original @p arg if no copy needed)
 *  - lr = @ref syscall_exit (so "return" triggers thread exit)
 *
 * @param func     User-mode entry function.
 * @param arg      Optional argument blob (may be NULL).
 * @param arg_size Size of argument blob in bytes.
 */
void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned arg_size);

/**
 * @brief Thread-exit hook: reap current thread and run next.
 *
 * Called when a thread executes the exit path (e.g., via syscall).
 * Marks the current thread as zombie, optionally frees it, then restores
 * the next runnable thread into @p frame.
 *
 * @param frame Pointer to the live exception frame being used for return.
 */
void scheduler_on_thread_exit(struct exc_frame *frame);

/**
 * @brief Return the currently running thread (or NULL before start).
 */
thread_t *scheduler_curr(void);

#endif /* SCHEDULER_H */
/** @} */ /* end of scheduler_api */