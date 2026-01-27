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
		*(.init .init.*)
	}
	ld_section_init_end = .;

	. = KERNEL_TEXT_BASE;
	ld_section_kernel_text = .;
	.text : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (*build/user/*.o *build/user/*/*.o) *(.text .text.*)
	}
	ld_section_kernel_text_end = .;

	. = KERNEL_RODATA_BASE;
	ld_section_kernel_rodata = .;
	.rodata : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (*build/user/*.o *build/user/*/*.o) *(.rodata .rodata.*)
	}
	ld_section_kernel_rodata_end = .;

	. = KERNEL_DATA_BSS_BASE;
	ld_section_kernel_data_bss = .;
	.data : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (*build/user/*.o *build/user/*/*.o) *(.data .data.*)
	}
	.bss : ALIGN(PAGE_SIZE) {
		EXCLUDE_FILE (*build/user/*.o *build/user/*/*.o) *(.bss .bss.* COMMON)
	}
	ld_section_kernel_data_bss_end = .;

	. = USER_TEXT_BASE;
	ld_section_user_text = .;
	.utext : ALIGN(PAGE_SIZE) {
		*build/user/*.o(.text .text.*)
		*build/user/*/*.o(.text .text.*)
	}
	ld_section_user_text_end = .;

	. = USER_RODATA_BASE;
	ld_section_user_rodata = .;
	.urodata : ALIGN(PAGE_SIZE) {
		*build/user/*.o(.rodata .rodata.*)
		*build/user/*/*.o(.rodata .rodata.*)
	}
	ld_section_user_rodata_end = .;

	. = USER_DATA_BSS_BASE;
	ld_section_user_data_bss = .;
	.udata : ALIGN(PAGE_SIZE) {
		*build/user/*.o(.data .data.*)
		*build/user/*/*.o(.data .data.*)
	}
	.ubss : ALIGN(PAGE_SIZE) {
		*build/user/*.o(.bss .bss.* COMMON)
		*build/user/*/*.o(.bss .bss.* COMMON)
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
