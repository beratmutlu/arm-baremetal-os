#include <user/main.h>
#include <tests/regcheck.h>
#include <kernel/exc_triggers.h>
#include <arch/bsp/uart.h>
#include <config.h>


void main(void * args) {
	test_user(args);
	char c = *((char *) args);
	switch (c) {
		case 'a':
			do_data_abort();
			return;
		case 'p':
			do_prefetch_abort();
			return;
		case 'u':
			do_undef();
			return;
		case 's':
			do_svc();
			return;
		case 'c':
			register_checker();
			return;
	}
	
	for(unsigned int n = 0; n < PRINT_COUNT; n++){
		for(volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++){}
		uart_putc(c);
	}
}