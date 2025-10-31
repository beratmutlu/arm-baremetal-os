#include <arch/bsp/uart.h>
#include <kernel/kprintf.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <config.h>

void start_kernel(){
	
	init_uart();

	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();
		
	while(true){
	    char c = uart_getc();
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		kprintf("Es wurde folgendes Zeichen eingegeben: %c, In Hexadezimal: %x, "
		    "In Dezimal: %08i, Als Ptr: %p\n",
			c, (unsigned int)c, (int)c, (void*)c);
		#pragma GCC diagnostic pop
	}
}
