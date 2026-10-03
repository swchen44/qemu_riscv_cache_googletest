OUTPUT_ARCH(riscv)
ENTRY(_start)
MEMORY { RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 16M }
SECTIONS {
 .text : { KEEP(*(.text.init)) *(.text .text.*) } > RAM
 .rodata : { *(.rodata .rodata.*) } > RAM
 .preinit_array : { __preinit_array_start = .; KEEP(*(.preinit_array)) __preinit_array_end = .; } > RAM
 .init_array : { __init_array_start = .; KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array)) __init_array_end = .; } > RAM
 .fini_array : { __fini_array_start = .; KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array)) __fini_array_end = .; } > RAM
 .data : { *(.data .data.*) . = ALIGN(16); __global_pointer$ = . + 0x800; *(.sdata .sdata.*) } > RAM
 .bss : { . = ALIGN(16); __bss_start = .; *(.bss .bss.* .sbss .sbss.*) *(COMMON) . = ALIGN(16); __bss_end = .; } > RAM
 __heap_start = .;
 __heap_end = ORIGIN(RAM) + LENGTH(RAM) - 128K;
 __stack_top = ORIGIN(RAM) + LENGTH(RAM);
 /DISCARD/ : { *(.comment) }
}
