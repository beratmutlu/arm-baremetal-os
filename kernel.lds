ENTRY(_start)

MEMORY 
{
    ROM (rx)  : ORIGIN = 0x00008000, LENGTH = 0x00040000
    RAM (rwx) : ORIGIN = 0x00048000, LENGTH = 0x07FB8000
}

SECTIONS
{
    . = 0x00008000;

    .vectors : ALIGN(32) {
        KEEP(*(.vectors))
    } > ROM
    
    .init : { *(.init) } > ROM
    .text : { *(.text) } > ROM
    .rodata : { *(.rodata) } > ROM
    
    . = ALIGN(4096);
    
    .data : { *(.data) } > RAM
    .bss  : { *(.bss) } > RAM
    
    . = ALIGN(8);
    _end = .;
    
    /* Stacks allocated from high memory (growing down) */
    __stack_top = 0x08000000;
    
    __stack_svc_end = __stack_top;
    __stack_svc_start = __stack_svc_end - 0x1000;
    
    __stack_irq_end = __stack_svc_start;
    __stack_irq_start = __stack_irq_end - 0x1000;
    
    __stack_abt_end = __stack_irq_start;
    __stack_abt_start = __stack_abt_end - 0x1000;
    
    __stack_und_end = __stack_abt_start;
    __stack_und_start = __stack_und_end - 0x1000;
    
    __stack_sys_end = __stack_und_start;
    __stack_sys_start = __stack_sys_end - 0x1000;
}