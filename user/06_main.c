#include <config.h>
#include <user/uprintf.h>
#include <user/syscalls.h>

volatile char global_char;
volatile int global_counter = 100;

static constexpr int COUNTER_LIMIT = 120;

void worker(void* arg){
	int id = *((int*)arg);
	int local_counter = 0;
	while (global_counter < COUNTER_LIMIT){
		global_counter++;
		local_counter++;

		uprintf("%c:%i (%i:%i)\n", global_char, global_counter, id, local_counter);
		syscall_sleep(1);
	}
}

void worker_thread(void *args){
	test_user(args);

	char c = *((char*)args);
	global_char = c;
	int id1 = 1;
	int id2 = 2;
	int id3 = 3;

	syscall_create_thread(&worker, &id1, sizeof(id1));
	syscall_create_thread(&worker, &id2, sizeof(id2));
	worker(&id3);
}

void main(void) {
	test_user_main();

	while(true) {
		char c = syscall_getc();
		syscall_create_process(worker_thread, &c, sizeof(c));
	}
}
