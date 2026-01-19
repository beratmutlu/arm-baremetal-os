#include <stddef.h>
#include <config.h>
#include <user/syscalls.h>
#include <stdint.h>

extern char ld_section_kernel_text;
extern char ld_section_kernel_data_bss;

/* MMIO base address (BCM2835 peripherals) */
#define MMIO_BASE 0x3F000000u

/**
 * @brief Read from kernel data section
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_data(void) {
    volatile char var = *((volatile char *)&ld_section_kernel_data_bss);
    (void)var;
}

/**
 * @brief Read from kernel text section
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_text(void) {
    volatile char var = *((volatile char *)&ld_section_kernel_text);
    (void)var;
}

/**
 * @brief Read from kernel stack area
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_kernel_stack(void) {
    volatile char var = *((volatile char *)0x004FF000u);
    (void)var;
}

/**
 * @brief Read from MMIO (peripheral) region
 * Triggers data abort if called from user mode with MMU protection enabled
 */
static inline void read_mmio(void) {
    volatile uint32_t var = *((volatile uint32_t *)MMIO_BASE);
    (void)var;
}

/**
 * @brief Read from an invalid/unmapped address
 * Triggers data abort due to translation fault
 */
static inline void read_invalid_addr(void) {
    volatile char var = *((volatile char *)0xFFFFFFFFu);
    (void)var;
}


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
