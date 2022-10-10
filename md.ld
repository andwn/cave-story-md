MEMORY {
    rom (rx) : ORIGIN = 0x000000, LENGTH = 4M
    ram (wx) : ORIGIN = 0xFF0000, LENGTH = 64K
}

SECTIONS {
  /* With experience, one learns the standard, scientific way to compute the */
  /* proper size for a stack: Pick a size at random and hope. --Jack Ganssle */
  __stack_top = ORIGIN(ram) + LENGTH(ram);
  __stack_size = 0x800;
  __stack_bottom = __stack_top - __stack_size;

  .text ORIGIN(rom) : {
    __text_start = .;
    KEEP(*(.text.keepboot)) /* Vectors & ROM Header must always be first */
	*(.text .text.*)
	. = ALIGN(2);
    __text_end = .;
  } > rom

  .data ORIGIN(ram) : {
    __data_start = .;
    *(.data .data.*)
    . = ALIGN(2);
  } > ram AT > rom
  __data_size = SIZEOF(.data);

  .rodata : {
    __rodata_start = .;
    *(.rodata .rodata.*)
    . = ALIGN(2);
    __rodata_end = .;
  } > rom
  __rodata_size = SIZEOF(.rodata);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss .bss.*)
    *(.COMMON)
    . = ALIGN(2);
    __bss_end = .;
  } > ram
  __bss_size = SIZEOF(.bss);

  /* Heap is everything remaining between .bss and the stack */
  __heap_start = __bss_end;
  __heap_end = __stack_bottom;
  __heap_size = __heap_end - __heap_start;
}
