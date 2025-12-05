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
                                           thread_t *thread,
                                           bool from_irq)
{
    if (!thread || !frame) {
        return;
    }

    for (int i = 0; i < EXC_GPR_COUNT; i++) {
        thread->ctx.r[i] = frame->r[i];
    }

    thread->ctx.psr = frame->spsr;

    if (from_irq) {
        /*
         * IRQ path:
         *   On entry: LR_irq = interrupted_pc + 4
         *   irq_tramp uses EXC_EXIT 4: PC = LR - 4
         * So resume_pc = frame->lr - 4.
         */
        thread->ctx.pc = frame->lr - 4;
    } else {
        /*
         * Synchronous exceptions (SVC/ABT/UND) where EXC_EXIT 0 is used:
         *   EXC_EXIT 0: PC = LR
         * So resume_pc = frame->lr.
         *
         * Note: for exiting threads we never use this pc again,
         * but keep this for completeness.
         */
        thread->ctx.pc = frame->lr;
    }

    thread->ctx.sp = cpu_get_banked_sp(CPU_USR);
    thread->ctx.lr = cpu_get_banked_lr(CPU_USR);
}

static inline void restore_frame_from_context(const thread_t *thread,
                                              struct exc_frame *frame,
                                              bool to_irq)
{
    if (!thread || !frame) {
        return;
    }

    for (int i = 0; i < EXC_GPR_COUNT; i++) {
        frame->r[i] = thread->ctx.r[i];
    }

    frame->spsr = thread->ctx.psr;

    if (to_irq) {
        frame->lr = thread->ctx.pc + 4;
    } else {
        frame->lr = thread->ctx.pc;
    }

    cpu_set_banked_sp(CPU_USR, thread->ctx.sp);
    cpu_set_banked_lr(CPU_USR, thread->ctx.lr);
}


static inline void scheduler_enqueue_ready(thread_t *thread) {
    if (!thread) {
        return;
    }

    thread->state = THREAD_READY;
    list_add_last(ready_queue, &thread->runq_node);
}

static inline thread_t *scheduler_pick_next(void) {
    list_node *next = list_remove_first(ready_queue);
    if (!next) {
        return NULL;
    }

    thread_t *thread_next = node_to_thread(next);


    return thread_next;
}

void scheduler_start(void) {
    thread_t *next = scheduler_pick_next();
    if (!next) {
        next = idle_thread;
    }
    current_thread = next;
    current_thread->state = THREAD_RUNNING;

    struct exc_frame frame;
    restore_frame_from_context(current_thread, &frame, false);
    scheduler_start_asm(&frame);
}


static thread_t *scheduler_thread_create_helper(void (*func)(void *), const void *arg, unsigned arg_size) {
    thread_t *thread = thread_alloc();
    if (!thread) {
        return NULL;
    }
    thread->state = THREAD_READY;

    uint8_t *sp8 = thread->stack + thread->stack_size;
    
    void *arg_copy_ptr = NULL;
    if (arg && arg_size > 0) {
        sp8 -= arg_size;
        memcpy(sp8, arg, arg_size);
        arg_copy_ptr = (void *)sp8;
    }

    sp8 = (uint8_t *)((uintptr_t)sp8 & ~0x7);
    uint32_t *sp = (uint32_t *)sp8;

    memset(&thread->ctx, 0, sizeof(thread->ctx));
    
    thread->ctx.sp = (uint32_t)sp;

    thread->ctx.r[0] = (uint32_t)arg_copy_ptr;

    thread->ctx.pc = (uint32_t)func;
    thread->ctx.lr = (uint32_t)syscall_exit;

    thread->ctx.psr = USER_MODE_PSR;
    
    
    return thread;
}

void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned arg_size) {
    thread_t *thread = scheduler_thread_create_helper(func, arg, arg_size);
    if (!thread) {
        kprintf("Could not create thread.");
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
        save_context_from_frame(frame, current_thread, true);
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

    restore_frame_from_context(current_thread, frame, true);

    if (!next->is_idle && prev != next) {
        uart_putc('\n');
    }
}
void scheduler_on_thread_exit(struct exc_frame *frame) {
    save_context_from_frame(frame, current_thread, false);
    
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
        if (!current_thread->is_idle) {
            uart_putc('\n');
        }
    }


    restore_frame_from_context(current_thread, frame, false);
}
