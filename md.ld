MEMORY {
    rom (rx) : ORIGIN = 0x000000, LENGTH = 4M
    ram (wx) : ORIGIN = 0xFF0000, LENGTH = 64K
}

SECTIONS {
  /* ROM/RAM boundary addresses for the ROM Header */
  __rom_start = ORIGIN(rom);
  __rom_end = ORIGIN(rom) + LENGTH(rom);
  __ram_start = ORIGIN(ram);
  __ram_end = ORIGIN(ram) + LENGTH(ram);

  /* With experience, one learns the standard, scientific way to compute the */
  /* proper size for a stack: Pick a size at rand and hope. --Jack Ganssle */
  __stack_top = __ram_end;
  __stack_size = 0x400;
  __stack_bottom = __stack_top - __stack_size;

  .text ORIGIN(rom) : {
    __text_start = .;
    KEEP(*(.text.vectors)) /* CPU Vectors */
    KEEP(*(.text.header))  /* ROM Header */
    *(.text.boot)
	*(.text .text.*)
	. = ALIGN(2);
    __text_end = .;
  } > rom

  .data ORIGIN(ram) : {
    __data_start = .;
    *(.data .data.*)
    . = ALIGN(2);
    __data_end = .;
  } > ram AT > rom
  __data_size = SIZEOF(.data);

  .rodata : {
    __rodata_start = .;
    *(.rodata .rodata.*)
    *(.patch)
    . = ALIGN(2);
    __rodata_end = .;
  } > rom
  __rodata_size = SIZEOF(.rodata);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss.stage)   /* Stage (PXM) data decompressed to start of RAM */
    *(.bss.system)  /* End of stage data bleeds into errors */
    *(.bss .bss.*)
    . = ALIGN(2);
    __bss_end = .;
  } > ram
  __bss_size = SIZEOF(.bss);

  /* Compression buffer for graphics and such */
  __compbuf_start = __stack_bottom - 0x1000;
  __compbuf_end = __stack_bottom;
  __compbuf_size = __compbuf_end - __compbuf_start;

  /* Heap is everything remaining between .bss and the stack */
  __heap_start = __bss_end;
  __heap_end = __compbuf_start;
  __heap_size = __heap_end - __heap_start;
}
