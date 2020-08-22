OUTPUT_ARCH(m68k)
SEARCH_DIR(.)

/*
 * Setup the memory map of the SEGA Genesis.
 * stack grows down from high memory.
 *
 * The memory map look like this:
 * +--------------------+ <- low memory
 * | .text              |
 * |        _stext      |
 * +--------------------+
 * | .data (ROM)        | initialized vars, copied to .data
 * +--------------------+
 * | .rodata            | Non-code data that can be put in far chunks
 * |        _srodata    |
 * +--------------------+ <- end of ROM
 * .                    .
 * .                    .
 * .                    .
 * +--------------------+ <- 0xFFFF0000
 * | .data (RAM)        | initialized data goes here
 * |        _sdata      |
 * +--------------------+
 * | .bss               |
 * |        _bss_start  | start of bss
 * |        _bss_end    | start of heap, for malloc()
 * +--------------------+
 * .                    .
 * .                    .
 * .                    .
 * |        __stack     | top of stack
 * +--------------------+ <- 0x(1)00000000
 */
MEMORY
{
	rom : ORIGIN = 0x00000000, LENGTH = 0x02000000
	ram : ORIGIN = 0xFFFF0000, LENGTH = 0x00010000
}

PROVIDE (__stack = 0x00000000);

SECTIONS
{
  .text 0x00000000:
  {
    KEEP(*(.text.keepboot)) *(.text.*) *(.text)
    . = ALIGN(0x2);
  } > rom
  _stext = SIZEOF(.text);

  .data 0xFFFF0000:
  AT ( SIZEOF(.text) )
  {
    *(.data .data.*)
    . = ALIGN(0x2);
  } > ram
  _sdata = SIZEOF(.data);

  .rodata 0x00000000 + SIZEOF(.text) + SIZEOF(.data):
  {
    *(.rodata .rodata.*)
    . = ALIGN(0x2);
  } > rom
  _srodata = SIZEOF(.rodata);

  .bss 0xFFFF0000 + SIZEOF(.data):
  {
    _bss_start = . ;
    *(.bss .bss.*)
    . = ALIGN(0x2);
    _bss_end = . ;
  } > ram
}
