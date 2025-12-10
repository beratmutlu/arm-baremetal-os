.syntax unified
.arch armv7-a
.arm

.global syscall_exit
.type syscall_exit, %function

syscall_exit:
    SVC #0          @ Trigger syscall 0 (thread exit)
    
    @ Should never reach here, but loop just in case
1:  B 1b

.size syscall_exit, .-syscall_exit