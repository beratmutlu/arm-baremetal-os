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
	
	. = KERNEL_INIT_BASE;
	.init : ALIGN(PAGE_SIZE) {
		*(.init*)
	}
	ld_section_init_end = .;

	. = KERNEL_TEXT_BASE;
	ld_section_kernel_text = .;
	.text : ALIGN(PAGE_SIZE) {
		*(.text*)
		*(.vectors*)
		*(.glue*)
		*(.vfp*)
		*(.v4_bx*)
		*(.iplt*)
		*(.rel.iplt*)
	}
	ld_section_kernel_text_end = .;

	. = KERNEL_RODATA_BASE;
	ld_section_kernel_rodata = .;
	.rodata : ALIGN(PAGE_SIZE) {
		*(.rodata*)
		*(.ARM.extab*)
		*(.ARM.exidx*)
		*(.init_array*)
		*(.fini_array*)
		*(.ctors*)
		*(.dtors*)
		*(.eh_frame*)
		*(.gcc_except_table*)
	}
	ld_section_kernel_rodata_end = .;

	. = KERNEL_DATA_BSS_BASE;
	ld_section_kernel_data_bss = .;
	.data : ALIGN(PAGE_SIZE) {
		*(.data*)
		*(.got*)
		*(.igot*)
		*(.rel.dyn*)
		*(.rela.dyn*)
	}
	.bss : ALIGN(PAGE_SIZE) {
		*(.bss*)
		*(COMMON)
	}
	ld_section_kernel_data_bss_end = .;

	/DISCARD/ : {
		*(.comment*)
		*(.note*)
		*(.debug*)
		*(.ARM.attributes)
		*(.gnu.attributes*)
	}
}
