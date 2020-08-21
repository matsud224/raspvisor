SECTIONS
{
  . = 0x0000000000000000;
  .text.boot : { *(.text.boot) }
  . = ALIGN(0x00001000);
  .text :  { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  . = ALIGN(0x8);
  bss_begin = .;
  .bss : { *(.bss*) }
  bss_end = .;
}
