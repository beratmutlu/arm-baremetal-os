.syntax unified
.arch armv7-a
.arm

#include <user/syscalls.h>
.global syscall_exit

syscall_exit:
	mov r7, #SYSCALL_ID_EXIT
	svc #0

syscall_create_thread:
	mov r7,	#SYSCALL_ID_CREATE
	svc #0