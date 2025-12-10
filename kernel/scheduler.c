#include <string.h>
#include <stddef.h>
#include <lib/list.h>
#include <kernel/threads.h>
#include <kernel/scheduler.h>
#include <arch/cpu/banked_regs.h>
#include <arch/cpu/cpu.h>
#include <stdint.h>
#include <kernel/kprintf.h>
#include <user/syscall.h>
#include <arch/bsp/uart.h>
extern void syscall_exit(void);
extern void scheduler_start_asm(struct exc_frame *frame);
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

static thread_t *node_to_thread(list_node *node) {
    return container_of(node, thread_t, runq_node);
}
#define USER_MODE_PSR 0x00000010
#define KERNEL_MODE_PSR_IRQ_ENABLE 0x0000001F
static thread_t *idle_thread = NULL;
static thread_t *current_thread = NULL;

list_create(ready_queue);

static void idle_func(void *arg) {
    arg = NULL;
    while (!arg) {
        asm volatile ("wfi");
    }
}

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


static inline void scheduler_enqueue_ready(thread_t *thread) {
    if (!thread || thread->is_idle || thread->in_runq) {
        return;
    }
    thread->in_runq = true;
    thread->state = THREAD_READY;
    list_add_last(ready_queue, &thread->runq_node);
}

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

    thread->ctx.pc  = (uint32_t)func + 4;       
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
        uart_putc('\n');
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

    uart_putc('\n');
    restore_frame_from_context(current_thread, frame);
}
