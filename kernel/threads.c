/**
 * @file threads.c
 * @brief Thread allocation/free on a static thread table.
 */
#include <string.h>
#include <stddef.h>
#include <kernel/threads.h>

/**
 * @brief Static stack pool.
 */
__attribute__((aligned(8))) unsigned char thread_stacks[THREADS_MAX_COUNT][THREADS_STACK_SIZE];

/** Static thread table (one TCB per possible thread). */
static thread_t thread_table[THREADS_MAX_COUNT];

/** Free-list head for available threads. */
static thread_t *free_list = NULL;

void threads_init(void) {
    free_list = NULL;

    for (int i = 0; i < THREADS_MAX_COUNT; i++) {
        thread_t *thread = &thread_table[i];

        thread->tid = (uint32_t)i;
        thread->state = THREAD_ZOMBIE;

        thread->runq_node.next = NULL;
        thread->runq_node.prev = NULL;

        thread->is_idle = false;
        thread->in_runq = false;

        thread->stack = thread_stacks[i];
        thread->stack_size = THREADS_STACK_SIZE;

        memset(&thread->ctx, 0, sizeof(thread->ctx));
        thread->ctx.sp = (uint32_t)(thread->stack + thread->stack_size);

        thread->next_free = free_list;
        free_list = thread;
    }
}

thread_t *thread_alloc(void) {
    if (!free_list) {
        return NULL;
    }
    thread_t *thread = free_list;
    free_list = thread->next_free;
    thread->next_free = NULL;

    thread->in_runq = false;
    thread->state = THREAD_READY;
    
    return thread;
}

void thread_free(thread_t *thread) {
    if (!thread) {
        return;
    }
    thread->state = (thread_state_t) THREAD_ZOMBIE;
    thread->in_runq = false;
    memset(&thread->ctx, 0, sizeof(thread->ctx));
    thread->ctx.sp = (uint32_t) (thread->stack + thread->stack_size);

    thread->next_free = free_list;
    free_list = thread;
}