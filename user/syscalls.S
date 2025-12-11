#include <user/syscalls.h>
.global syscall_exit

syscall_exit:
	mov r7, #SYSCALL_ID_EXIT
	svc #0
