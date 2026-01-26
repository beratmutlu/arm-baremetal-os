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
	. = USER_TEXT_BASE;
	ld_section_user_text = .;
	.utext : ALIGN(PAGE_SIZE) {
		build/user/*.o(.text)
	}
	ld_section_user_text_end = .;

	. = USER_RODATA_BASE;
	ld_section_user_rodata = .;
	.urodata : ALIGN(PAGE_SIZE) {
		build/user/*.o(.rodata .rodata.*)
	}
	ld_section_user_rodata_end = .;

	. = USER_DATA_BSS_BASE;
	ld_section_user_data_bss = .;
	.udata : ALIGN(PAGE_SIZE) {
		build/user/*.o(.data)
	}
	.ubss : ALIGN(PAGE_SIZE) {
		build/user/*.o(.bss)
	}
	ld_section_user_data_bss_end = .;

	. = KERNEL_INIT_BASE;
	.init : ALIGN(PAGE_SIZE) { *(.init) }
	ld_section_init_end = .;

	. = KERNEL_TEXT_BASE;
	ld_section_kernel_text = .;
	.text : ALIGN(PAGE_SIZE) {
		*lib*.a:*(.text)
		build/arch/*.o(.text)
		build/kernel/*.o(.text)
		*(.text)
	}
	ld_section_kernel_text_end = .;

	. = KERNEL_RODATA_BASE;
	ld_section_kernel_rodata = .;
	.rodata : ALIGN(PAGE_SIZE) {
		*lib*.a:*(.rodata .rodata.*)
		build/arch/*.o(.rodata .rodata.*)
		build/kernel/*.o(.rodata .rodata.*)
		*(.rodata .rodata.*)
	}
	ld_section_kernel_rodata_end = .;

	. = KERNEL_DATA_BSS_BASE;
	ld_section_kernel_data_bss = .;
	.data : ALIGN(PAGE_SIZE) {
		*lib*.a:*(.data)
		build/arch/*.o(.data)
		build/kernel/*.o(.data)
		*(.data)
	}
	.bss : ALIGN(PAGE_SIZE) {
		*lib*.a:*(.bss)
		build/arch/*.o(.bss)
		build/kernel/*.o(.bss)
		*(.bss)
	}
	ld_section_kernel_data_bss_end = .;
}
