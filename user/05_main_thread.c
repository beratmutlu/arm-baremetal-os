#include <stddef.h>
#include <config.h>
#include <user/syscalls.h>
#include <stdint.h>
#include <user/mmu_triggers.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
static void stackoverflow [[gnu::optimize("-O0")]] (void) {
	stackoverflow();
}
#pragma GCC diagnostic pop

static int user_data = 42;

void user_prog(void * arg) {
	char c = *((char *) arg);
	test_user(arg);

	volatile char var = 'a';

	switch (c) {
	#pragma GCC diagnostic push 
	#pragma GCC diagnostic ignored "-Wpedantic"
     // cppcheck-suppress nullPointer
	case 'n': var = *((volatile char *) NULL); break; // read from NULL
	case 'p': ((void (*)(void)) NULL)(); break; // jump to NULL
	case 'd': read_kernel_data(); break;
	case 'k': read_kernel_text(); break;
	case 'K': read_kernel_stack(); break;
	case 'g': read_mmio(); break;
	case 'c': *((char *) user_prog) = var; break; // write to user text
	case 's': stackoverflow(); break;
	case 'u': read_invalid_addr(); break;
	case 'x': ((void (*)(void)) &user_data)(); break; // jump to use data
	#pragma GCC diagnostic pop
	}

	for(unsigned int i = 0; i < 10; i++) {
		syscall_putc(c);

		if( c >= 'A' && c <= 'Z') {
			for(volatile unsigned int j = 0; j < BUSY_WAIT_COUNTER; j++) {}
		} else {
			syscall_sleep(2);
		}
	}
}

void main (void) {
	while(true){
		char c = syscall_getc();
		syscall_create_thread(user_prog, &c, sizeof(c));
	}
}
