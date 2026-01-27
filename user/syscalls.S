.syntax unified
.arch armv7-a
.arm

#include <user/syscalls.h>

/* user/syscalls.S */
/* 
 * Syscall Calling Convention:
 * 
 * Register R7: Syscall Number/ID
 * Register R0: Argument 1
 * Register R1: Argument 2
 * Register R2: Argument 3
 * Register R3: Argument 4
 * 
 * Return Value: Stored in R0 upon return.
 */
 
.global syscall_exit
.global syscall_create_thread
.global syscall_getc
.global syscall_putc
.global syscall_undefined
.global syscall_sleep

syscall_exit:
	mov r7, #SYSCALL_ID_EXIT
	svc #0
1:	b 1b

syscall_create_thread:
	mov r7,	#SYSCALL_ID_CREATE
	svc #0
	bx	lr

syscall_getc:
	mov r7, #SYSCALL_ID_GETC
	svc #0
	bx 	lr

syscall_putc:
	mov r7, #SYSCALL_ID_PUTC
	svc #0
	bx 	lr

syscall_sleep:
	mov r7, #SYSCALL_ID_SLEEP
	svc #0
	bx 	lr

syscall_undefined:
	mov r7, #SYSCALL_ID_UND
	svc #0
	bx	lr