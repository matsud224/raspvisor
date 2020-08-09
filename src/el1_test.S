.org 0x0

.globl entry_el1_entry
el1_test_entry:
  mov x8, #123
  hvc #0
  mov x8, #456
  hvc #0
1:
  b 1b
