
/**
 * @file threads.h
 * @brief Thread table and stack management.
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
    THREAD_ZOMBIE,      /**< Finished / free-list eligible */
    THREAD_BLOCKED_IO,
    THREAD_BLOCKED_SLEEP
} thread_state_t;

/**
 * @brief Saved CPU context for a thread (scheduler-visible).
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
    int sleep_cycles_left;
};

/**
 * @brief Initialize the thread subsystem.
 *
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
 * @param thread Thread to free (NULL-safe).
 */
void thread_free(thread_t *thread);

#endif /* THREADS_H */
