SECTIONS
{
  . = 0x0000000000000000;
  .text.boot : { *(.text.boot) }
  . = ALIGN(0x00001000);
  el1_test_begin = .;
  .text.user : { build/el1_* (.text) }
  .rodata.user : { build/el1_* (.rodata) }
  .data.user : { build/el1_* (.data) }
  .bss.user : { build/el1_* (.bss) }
  el1_test_end = .;
  .text :  { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  . = ALIGN(0x8);
  bss_begin = .;
  .bss : { *(.bss*) }
  bss_end = .;
  . = ALIGN(0x00001000);
  pg_dir = .;
  .data.pgd : { . += (4 * (1 << 12)); }
}

