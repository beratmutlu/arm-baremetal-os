#include <config.h>
#include <user/syscalls.h>
void worker_thread(void * arg) {
	char c = *((char *) arg);
	test_user(arg);

	if( c == 's' ) {
		syscall_undefined();
	}
	
	for(unsigned int i = 0; i < PRINT_COUNT; i++) {
		syscall_putc(c);
		if( c >= 'A' && c <= 'Z') {
			for(volatile unsigned int j=0; j<BUSY_WAIT_COUNTER * ((unsigned int)c-'A');j++);
		} else if ( c >= 'a' && c <= 'z') {
			syscall_sleep((unsigned int) c - 'a');
		} else {
			syscall_sleep(1);
		}
	}
}

void main(void) {
	test_user_main();

	while(true) {
		char c = syscall_getc();
		syscall_create_thread(worker_thread, &c, sizeof(c));
	}
}
