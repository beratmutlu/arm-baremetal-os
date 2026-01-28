ENTRY(_start)

PAGE_SIZE = 0x1000;      /* 4 KiB */
MB       = 0x100000;     /* 1 MiB */

SECTIONS
{
  . = 0x00008000;
  .init : ALIGN(PAGE_SIZE)
  {
    *(.init .init.*)
  }
  ld_section_init_end = .;

  ld_section_kernel_text = .;
  .text : ALIGN(PAGE_SIZE)
  {
    *lib*.a:*(.text .text.*)
    build/arch/*.o(.text .text.*)
    build/kernel/*.o(.text .text.*)
  }
  ld_section_kernel_text_end = .;

  ld_section_kernel_rodata = .;
  .rodata : ALIGN(PAGE_SIZE)
  {
    *lib*.a:*(.rodata .rodata.*)
    build/arch/*.o(.rodata .rodata.*)
    build/kernel/*.o(.rodata .rodata.*)
  }
  ld_section_kernel_rodata_end = .;

  . = ALIGN(MB);

  ld_section_kernel_data_bss = .;
  .data : ALIGN(PAGE_SIZE)
  {
    *lib*.a:*(.data .data.*)
    build/arch/*.o(.data .data.*)
    build/kernel/*.o(.data .data.*)
  }

  .bss : ALIGN(PAGE_SIZE)
  {
    *lib*.a:*(.bss .bss.* COMMON)
    build/arch/*.o(.bss .bss.* COMMON)
    build/kernel/*.o(.bss .bss.* COMMON)
  }
  ld_section_kernel_data_bss_end = .;

  . = ALIGN(MB);

  ld_section_user_text = .;
  .utext : ALIGN(PAGE_SIZE)
  {
    build/user/*.o(.text .text.*)
  }
  ld_section_user_text_end = .;

  ld_section_user_rodata = .;
  .urodata : ALIGN(PAGE_SIZE)
  {
    build/user/*.o(.rodata .rodata.*)
  }
  ld_section_user_rodata_end = .;

  ld_section_user_data_bss = .;
  .udata : ALIGN(PAGE_SIZE)
  {
    build/user/*.o(.data .data.*)
  }

  .ubss : ALIGN(PAGE_SIZE)
  {
    build/user/*.o(.bss .bss.* COMMON)
  }
  ld_section_user_data_bss_end = .;
}
