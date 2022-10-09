MEMORY {
    rom (rx) : ORIGIN = 0x000000, LENGTH = 4M
    ram (wx) : ORIGIN = 0xFF0000, LENGTH = 64K
}

SECTIONS {
  __stack_top = ORIGIN(ram) + LENGTH(ram);
  __stack_size = 0x400;

  .text ORIGIN(rom) : {
    KEEP(*(.text.keepboot))
	*(.text .text.*)
	. = ALIGN(2);
    *(.rodata .rodata.*)
    . = ALIGN(2);
    __text_end = .;
  } > rom

  .data ORIGIN(ram) : {
    __data_start = .;
    *(.data .data.*)
    . = ALIGN(2);
  } > ram AT > rom
  __data_size = SIZEOF(.data);

  .bss (NOLOAD) : {
    __bss_start = .;
    *(.bss .bss.*)
    . = ALIGN(2);
    __bss_end = .;
  } > ram
  __bss_size = SIZEOF(.bss);
}
