#ifndef THREADS_H
#define THREADS_H

#include <stdint.h>
#include <lib/list.h>
#include <kernel/exc_frame_layout.h>
#include <stdbool.h>

#define THREADS_MAX_COUNT   32
#define THREADS_STACK_SIZE  2048

typedef enum thread_table_state {
    THREAD_TABLE_FULL
} thread_table_state_t;

typedef enum thread_state {
    THREAD_READY = 0,
    THREAD_RUNNING,
    THREAD_ZOMBIE
} thread_state_t;

typedef struct cpu_context {
    uint32_t r[EXC_GPR_COUNT];
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} cpu_ctx_t;

typedef struct thread thread_t;

struct thread {
    uint32_t tid;
    thread_state_t state;
    list_node runq_node;
    cpu_ctx_t ctx;
    uint8_t *stack;
    uint32_t stack_size;
    thread_t *next_free;
    bool is_idle;
};

void threads_init(void);

thread_t *thread_alloc(void);

void thread_free(thread_t *thread);


#endif