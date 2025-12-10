#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_ID_EXIT          1
#ifndef __ASSEMBLER__
void syscall_exit [[noreturn]] (void);
#endif // __ASSEMBLER__
#endif // SYSCALL_H
