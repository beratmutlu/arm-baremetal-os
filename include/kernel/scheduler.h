#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <kernel/threads.h>
#include <kernel/exc_frame_layout.h>

void scheduler_init(void);

void scheduler_start(void);

void scheduler_on_timer(struct exc_frame *frame);

void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned arg_size);

void scheduler_on_thread_exit(struct exc_frame *frame);

#endif