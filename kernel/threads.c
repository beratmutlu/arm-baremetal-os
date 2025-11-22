#include <string.h>
#include <stddef.h>
#include <kernel/threads.h>

__attribute__((aligned(8))) unsigned char thread_stacks[THREADS_MAX_COUNT][THREADS_STACK_SIZE];

static thread_t thread_table[THREADS_MAX_COUNT];
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

    thread->state = THREAD_READY;
    
    return thread;
}

void thread_free(thread_t *thread) {
    if (!thread) {
        return;
    }
    thread->state = (thread_state_t) THREAD_ZOMBIE;
    
    memset(&thread->ctx, 0, sizeof(thread->ctx));
    thread->ctx.sp = (uint32_t) (thread->stack + thread->stack_size);

    thread->next_free = free_list;
    free_list = thread;
}