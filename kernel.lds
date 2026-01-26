ENTRY(_start)

KERNEL_INIT_BASE      = 0x00008000;
KERNEL_TEXT_BASE      = 0x00100000;
KERNEL_RODATA_BASE    = 0x00200000;
KERNEL_DATA_BSS_BASE  = 0x00300000;

USER_TEXT_BASE        = 0x00500000;
USER_RODATA_BASE      = 0x00600000;
USER_DATA_BSS_BASE    = 0x00700000;

PAGE_SIZE             = 0x00001000;  /* 4 KiB */

SECTIONS
{
	. = KERNEL_INIT_BASE;
	.init : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.init*)
	}
	ld_section_init_end = .;

	. = KERNEL_TEXT_BASE;
	ld_section_kernel_text = .;
	.text : ALIGN(PAGE_SIZE) {
		/* Wildcard patterns catch ALL variants (e.g., .text.startup, .text.hot, etc.) */
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.text*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.vectors*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.glue*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.vfp*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.v4_bx*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.iplt*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.rel.iplt*)
	}
	ld_section_kernel_text_end = .;

	. = KERNEL_RODATA_BASE;
	ld_section_kernel_rodata = .;
	.rodata : ALIGN(PAGE_SIZE) {
		/* Catch ALL read-only section variants */
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.rodata*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.ARM*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.init_array*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.fini_array*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.ctors*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.dtors*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.eh_frame*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.gcc_except_table*)
	}
	ld_section_kernel_rodata_end = .;

	. = KERNEL_DATA_BSS_BASE;
	ld_section_kernel_data_bss = .;
	.data : ALIGN(PAGE_SIZE) {
		/* Catch ALL data section variants */
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.data*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.got*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.igot*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.rel.dyn*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.rela.dyn*)
	}
	.bss : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(.bss*)
		EXCLUDE_FILE (build/user/*.o build/user/*/*.o) *(COMMON)
	}
	ld_section_kernel_data_bss_end = .;

	. = USER_TEXT_BASE;
	ld_section_user_text = .;
	.utext : ALIGN(PAGE_SIZE) {
		/* Catch ALL text section variants from user files */
		build/user/*.o(.text*)
		build/user/*/*.o(.text*)
		build/user/*.o(.glue*)
		build/user/*/*.o(.glue*)
		build/user/*.o(.vfp*)
		build/user/*/*.o(.vfp*)
		build/user/*.o(.v4_bx*)
		build/user/*/*.o(.v4_bx*)
		build/user/*.o(.iplt*)
		build/user/*/*.o(.iplt*)
		build/user/*.o(.rel.iplt*)
		build/user/*/*.o(.rel.iplt*)
	}
	ld_section_user_text_end = .;

	. = USER_RODATA_BASE;
	ld_section_user_rodata = .;
	.urodata : ALIGN(PAGE_SIZE) {
		/* Catch ALL read-only section variants from user files */
		build/user/*.o(.rodata*)
		build/user/*/*.o(.rodata*)
		build/user/*.o(.ARM*)
		build/user/*/*.o(.ARM*)
		build/user/*.o(.init_array*)
		build/user/*/*.o(.init_array*)
		build/user/*.o(.fini_array*)
		build/user/*/*.o(.fini_array*)
		build/user/*.o(.ctors*)
		build/user/*/*.o(.ctors*)
		build/user/*.o(.dtors*)
		build/user/*/*.o(.dtors*)
		build/user/*.o(.eh_frame*)
		build/user/*/*.o(.eh_frame*)
		build/user/*.o(.gcc_except_table*)
		build/user/*/*.o(.gcc_except_table*)
	}
	ld_section_user_rodata_end = .;

	. = USER_DATA_BSS_BASE;
	ld_section_user_data_bss = .;
	.udata : ALIGN(PAGE_SIZE) {
		/* Catch ALL data section variants from user files */
		build/user/*.o(.data*)
		build/user/*/*.o(.data*)
		build/user/*.o(.got*)
		build/user/*/*.o(.got*)
		build/user/*.o(.igot*)
		build/user/*/*.o(.igot*)
		build/user/*.o(.rel.dyn*)
		build/user/*/*.o(.rel.dyn*)
		build/user/*.o(.rela.dyn*)
		build/user/*/*.o(.rela.dyn*)
	}
	.ubss : ALIGN(PAGE_SIZE) {
		build/user/*.o(.bss*)
		build/user/*/*.o(.bss*)
		build/user/*.o(COMMON)
		build/user/*/*.o(COMMON)
	}
	ld_section_user_data_bss_end = .;

	/DISCARD/ : {
		*(.comment*)
		*(.note*)
		*(.debug*)
		*(.ARM.attributes)
		*(.gnu.attributes*)
	}
}
