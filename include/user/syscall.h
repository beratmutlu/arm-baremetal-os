#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

static inline void syscall_exit(void);
static inline void syscall_exit(void) {
    asm volatile("svc #0");
}



#endif