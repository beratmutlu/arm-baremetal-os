
/**
 * @file threads.h
 * @defgroup threads_api Threads
 * @brief Thread table and stack management.
 *
 * Provides allocation/free of fixed-size @ref thread_t objects backed by:
 *  - A static thread table (@ref THREADS_MAX_COUNT)
 *  - A static stack pool (@ref THREADS_STACK_SIZE per thread)
 *
 * Ownership model:
 *  - @ref thread_alloc pops a thread from an internal free-list.
 *  - @ref thread_free resets and pushes it back to the free-list.
 *
 * @note This module does not perform scheduling; see @ref scheduler_api.
 * @note Not thread-safe by itself; the OS-Lab environment typically calls this
 *       with interrupts disabled or from a single control path.
 * @{
 */
#ifndef THREADS_H
#define THREADS_H

#include <stdint.h>
#include <lib/list.h>
#include <kernel/exc_frame_layout.h>
#include <stdbool.h>

/** Maximum number of threads (including idle). */
#define THREADS_MAX_COUNT   32

/** Stack size per thread in bytes. */
#define THREADS_STACK_SIZE  2048

/**
 * @brief High-level lifecycle state of a thread.
 */
typedef enum thread_state {
    THREAD_READY = 0,   /**< Runnable and may be scheduled */
    THREAD_RUNNING,     /**< Currently executing on CPU */
    THREAD_ZOMBIE       /**< Finished / free-list eligible */
} thread_state_t;

/**
 * @brief Saved CPU context for a thread (scheduler-visible).
 *
 * The register set mirrors what the scheduler needs to restore a thread.
 * - r[0..EXC_GPR_COUNT-1] correspond to user-mode r0–r12
 * - sp/lr are user-mode banked SP/LR
 * - pc is the resume address (used to populate exception LR)
 * - psr is the CPSR value restored on exception return
 */
typedef struct cpu_context {
    uint32_t r[EXC_GPR_COUNT];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} cpu_ctx_t;

typedef struct thread thread_t;

/**
 * @brief Thread control block.
 *
 * @note @ref runq_node is embedded so the scheduler can enqueue threads without
 *       additional allocations.
 */
struct thread {
    uint32_t tid;           /**< Thread ID (index into static table) */
    thread_state_t state;   /**< Lifecycle state */
    list_node runq_node;    /**< Ready queue linkage */
    cpu_ctx_t ctx;          /**< Saved CPU context */
    uint8_t *stack;         /**< Base of stack memory */
    uint32_t stack_size;    /**< Stack size in bytes */
    thread_t *next_free;    /**< Free-list linkage */
    bool is_idle;           /**< True iff this is the idle thread */
    bool in_runq;           /**< True iff currently enqueued in ready queue */
};

/**
 * @brief Initialize the thread subsystem.
 *
 * Resets the static thread table and builds the internal free-list.
 * Must be called exactly once at boot before allocating threads.
 */
void threads_init(void);

/**
 * @brief Allocate a thread control block from the free-list.
 *
 * @return A thread pointer on success, NULL if no thread is available.
 */
thread_t *thread_alloc(void);

/**
 * @brief Return a thread to the free-list.
 *
 * Resets the thread state to zombie, clears context, resets SP,
 * and pushes it back to the internal free-list.
 *
 * @param thread Thread to free (NULL-safe).
 */
void thread_free(thread_t *thread);

#endif /* THREADS_H */
/** @} */ /* end of threads_api */