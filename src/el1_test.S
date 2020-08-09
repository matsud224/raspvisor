.org 0x0

.globl el1_test_1
el1_test_1:
1:
  mov x8, #1
  hvc #0
  b 1b

.globl el1_test_2
el1_test_2:
2:
  mov x8, #2
  hvc #0
  b 2b
