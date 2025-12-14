
/**
 * @file scheduler.c
 * @brief Round-robin scheduler and context switch glue.
 */
#include <string.h>
#include <stddef.h>
#include <lib/list.h>
#include <kernel/threads.h>
#include <kernel/scheduler.h>
#include <arch/cpu/banked_regs.h>
#include <arch/cpu/cpu.h>
#include <stdint.h>
#include <kernel/kprintf.h>
#include <user/syscalls.h>
#include <arch/bsp/uart.h>

extern void syscall_exit(void);
extern void scheduler_start_asm(struct exc_frame *frame);

/**
 * @brief Classic container_of helper to map a run queue node to its thread.
 */
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

/**
 * @brief Convert a list node from the ready queue back to its owning thread.
 */
static thread_t *node_to_thread(list_node *node) {
    return container_of(node, thread_t, runq_node);
}

/* PSR presets */
#define USER_MODE_PSR 0x00000010
#define KERNEL_MODE_PSR_IRQ_ENABLE 0x0000001F

static thread_t *idle_thread = NULL;
static thread_t *current_thread = NULL;

/**
 * @brief Scheduler ready queue (FIFO).
 * The idle thread is not enqueued; it is selected only when the queue is empty.
 */
list_create(ready_queue);

list_create(sleep_queue);

list_create(io_queue);

/**
 * @brief Idle thread body.
 */
static void idle_func(void *arg) {
    arg = NULL;
    while (!arg) {
        asm volatile ("wfi");
    }
}

/**
 * @brief Save user execution context from an exception frame into a thread.
 *
 * @param frame  Live exception frame captured at exception entry.
 * @param thread Target thread whose context is updated.
 */
static inline void save_context_from_frame(const struct exc_frame *frame,
                                           thread_t *thread)
{
    if (!thread || !frame) {
        return;
    }

    for (int i = 0; i < EXC_GPR_COUNT; i++) {
        thread->ctx.r[i] = frame->r[i];
    }

    thread->ctx.psr = frame->spsr;


    thread->ctx.pc = frame->lr;

    thread->ctx.sp = cpu_get_banked_sp(CPU_USR);
    thread->ctx.lr = cpu_get_banked_lr(CPU_USR);
}

/**
 * @brief Restore thread context into an exception frame (in-place).
 *
 * @param thread Source thread context.
 * @param frame  Destination exception frame to be modified.
 */
static inline void restore_frame_from_context(const thread_t *thread,
                                              struct exc_frame *frame)
{
    if (!thread || !frame) {
        return;
    }

    for (int i = 0; i < EXC_GPR_COUNT; i++) {
        frame->r[i] = thread->ctx.r[i];
    }

    frame->spsr = thread->ctx.psr;

    frame->lr = thread->ctx.pc;

    cpu_set_banked_sp(CPU_USR, thread->ctx.sp);
    cpu_set_banked_lr(CPU_USR, thread->ctx.lr);
}

/**
 * @brief Enqueue a thread as runnable.
 *
 * @param thread Thread to enqueue.
 */
static inline void scheduler_enqueue_ready(thread_t *thread) {
    if (!thread || thread->is_idle || thread->in_runq) {
        return;
    }
    thread->in_runq = true;
    thread->state = THREAD_READY;
    list_add_last(ready_queue, &thread->runq_node);
}

/**
 * @brief Dequeue the next runnable thread from the ready queue.
 *
 * @return Next runnable thread, or NULL if queue is empty.
 */
static inline thread_t *scheduler_pick_next(void) {
    list_node *next = list_remove_first(ready_queue);
    if (!next) {
        return NULL;
    }

    thread_t *thread_next = node_to_thread(next);
    thread_next->in_runq =false;

    return thread_next;
}


thread_t *scheduler_curr(void) {
    return current_thread;
}

void scheduler_start(void) {
    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }
    current_thread = next;
    current_thread->state = THREAD_RUNNING;

    struct exc_frame frame = {0};
    restore_frame_from_context(current_thread, &frame);

    scheduler_start_asm(&frame);
}

static thread_t *scheduler_thread_create_helper(void (*func)(void *),
                                                const void *arg,
                                                unsigned arg_size)
{
    thread_t *thread = thread_alloc();
    if (!thread) return NULL;

    memset(&thread->ctx, 0, sizeof(thread->ctx));

    uint8_t *sp8 = thread->stack + thread->stack_size;
    sp8 = (uint8_t *)((uintptr_t)sp8 & ~0x7); 

    void *arg_ptr = (void *)arg;
    if (arg && arg_size > 0) {
        unsigned total = (arg_size + 7u) & ~7u;  
        sp8 -= total;                            
        memcpy(sp8, arg, arg_size);            
        arg_ptr = sp8;
    }

    thread->ctx.sp  = (uint32_t)(uintptr_t)sp8;   
    thread->ctx.r[0]= (uint32_t)(uintptr_t)arg_ptr; 

    thread->ctx.pc  = (uint32_t)(uintptr_t)func;   
    thread->ctx.lr  = (uint32_t)syscall_exit;
    thread->ctx.psr = USER_MODE_PSR;

    thread->state = THREAD_READY;
    return thread;
}


void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned arg_size) {
    thread_t *thread = scheduler_thread_create_helper(func, arg, arg_size);
    if (!thread) {
        kprintf("Could not create thread.\n");
        return;
    }
    scheduler_enqueue_ready(thread);
}

void scheduler_init(void) {
    idle_thread = scheduler_thread_create_helper(idle_func, NULL, 0);
    idle_thread->is_idle = true;
    idle_thread->ctx.psr = KERNEL_MODE_PSR_IRQ_ENABLE;
}

void scheduler_on_timer(struct exc_frame *frame) {
    thread_t *prev = current_thread;

    if (current_thread && current_thread->state == THREAD_RUNNING) {
        save_context_from_frame(frame, current_thread);
        if (!current_thread->is_idle) {
            current_thread->state = THREAD_READY;
            scheduler_enqueue_ready(current_thread);
        }
    }

    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }

    current_thread = next;
    current_thread->state = THREAD_RUNNING;

    restore_frame_from_context(current_thread, frame);

    if (prev != next) {
    }
}
void scheduler_on_thread_exit(struct exc_frame *frame) {
    thread_t *zombie = current_thread;
    zombie->state = THREAD_ZOMBIE;

    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }

    current_thread = next;
    current_thread->state = THREAD_RUNNING;
    
    if (!zombie->is_idle) {
        thread_free(zombie);
    }

    restore_frame_from_context(current_thread, frame);
}

void scheduler_blocked_on_io(struct exc_frame *frame) {
    thread_t *blocked_io = current_thread;
    blocked_io->state = THREAD_BLOCKED_IO;
    blocked_io->in_runq = false;
    list_add_last(io_queue, &blocked_io->runq_node);
    
    save_context_from_frame(frame, blocked_io);

    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }

    current_thread = next;
    current_thread->state = THREAD_RUNNING;

    restore_frame_from_context(current_thread, frame);
}

void scheduler_wake_blocked_on_io(char c) {
    list_node *to_wake_node = list_remove_first(io_queue);
    if (!to_wake_node) {
        return;
    }
    thread_t *to_wake = node_to_thread(to_wake_node);
    to_wake->ctx.r[0] = (uint32_t) (unsigned char) c;
    to_wake->in_runq = false;
    scheduler_enqueue_ready(to_wake);
    
}

bool is_io_queue_empty() {
    return list_is_empty(io_queue);
}

void scheduler_blocked_on_sleep(struct exc_frame *frame, unsigned cycles) {
    thread_t *blocked_sleep = current_thread;
    blocked_sleep->state = THREAD_BLOCKED_SLEEP;
    blocked_sleep->in_runq = false;
    list_remove(ready_queue, &blocked_sleep->runq_node);
    list_add_last(sleep_queue, &blocked_sleep->runq_node);
    blocked_sleep->sleep_cycles_left = cycles;
    save_context_from_frame(frame, blocked_sleep);

    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }

    current_thread = next;
    current_thread->state = THREAD_RUNNING;

    restore_frame_from_context(current_thread, frame);

}

void scheduler_update_sleep_q(void) {
    list_node *head = sleep_queue;           
    for (list_node *curr = head->next; curr != head; ) {
        list_node *next = curr->next;
        thread_t *t = node_to_thread(curr);

        if (t->sleep_cycles_left > 0) {
            t->sleep_cycles_left--;
        }
        if (t->sleep_cycles_left == 0) {
            list_remove(head, curr);
            scheduler_enqueue_ready(t);
        }
        curr = next;
    }
}
