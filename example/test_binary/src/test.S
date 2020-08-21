.section ".text.boot"

.globl _start
_start:
  mov x8, 777
  hvc #0
1:
  b 1b
