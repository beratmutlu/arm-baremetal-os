/* scheduler.c */
/**
 * @file scheduler.c
 * @brief Round-robin scheduler and context switch glue.
 *
 * This module manages:
 *  - A FIFO ready queue of @ref thread_t
 *  - The idle thread (runs when no user thread is runnable)
 *  - Save/restore between @ref struct exc_frame and @ref cpu_ctx_t
 *
 * The core idea:
 *  - Exception/IRQ entry code creates a C-visible @ref struct exc_frame.
 *  - The scheduler updates that frame to represent the next thread.
 *  - Exception return resumes the selected thread.
 *
 * @ingroup scheduler_api
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

/**
 * @name PSR presets
 * @{
 *
 * USER_MODE_PSR:
 *   CPSR mode bits set to user mode (0x10). No explicit flags set here;
 *   IRQ state depends on your exception-return path.
 *
 * KERNEL_MODE_PSR_IRQ_ENABLE:
 *   CPSR mode bits set to system-ish mode used for idle (0x1F).
 *   (Your project may interpret these bits depending on CPU setup.)
 */
#define USER_MODE_PSR 0x00000010
#define KERNEL_MODE_PSR_IRQ_ENABLE 0x0000001F
/** @} */

static thread_t *idle_thread = NULL;
static thread_t *current_thread = NULL;

/**
 * @brief Scheduler ready queue (FIFO).
 *
 * Threads in this queue are runnable in user mode. The idle thread is not
 * enqueued; it is selected only when the queue is empty.
 */
list_create(ready_queue);

/** @cond INTERNAL */

/**
 * @brief Idle thread body.
 *
 * Spins forever, executing WFI to reduce host CPU usage under emulation.
 * The weird loop condition matches your existing behavior (no external stop).
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
 * The exception frame contains r0–r12 plus exception LR and SPSR.
 * Additionally, user-mode SP/LR are stored using banked register accessors.
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

    /* In this design: exception LR is treated as the "resume PC". */
    thread->ctx.pc = frame->lr;

    /* User-mode banked SP/LR are not in the exception frame. */
    thread->ctx.sp = cpu_get_banked_sp(CPU_USR);
    thread->ctx.lr = cpu_get_banked_lr(CPU_USR);
}

/**
 * @brief Restore thread context into an exception frame (in-place).
 *
 * After this, exception return will resume the selected thread.
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
 * - No-op for NULL
 * - No-op for idle
 * - No-op if already enqueued (@ref thread_t::in_runq)
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

/** @endcond */

/* Public API */

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

    /* Prepare the initial exception frame from the chosen thread context. */
    struct exc_frame frame = {0};
    restore_frame_from_context(current_thread, &frame);

    /* Transfer control to ASM routine that performs the first exception return. */
    scheduler_start_asm(&frame);
}

static thread_t *scheduler_thread_create_helper(void (*func)(void *),
                                                const void *arg,
                                                unsigned arg_size)
{
    thread_t *thread = thread_alloc();
    if (!thread) return NULL;

    memset(&thread->ctx, 0, sizeof(thread->ctx));

    /**
     * Stack discipline:
     *  - Use this thread's private stack top
     *  - Align to 8 bytes (AAPCS / EABI-friendly)
     *  - Optionally copy an argument blob onto the new stack
     */
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
