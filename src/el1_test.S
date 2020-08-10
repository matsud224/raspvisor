.org 0x0

.globl el1_test_1
el1_test_1:
  mov x0, #0x40
  mov x1, #123
  str x1, [x0]
  ldr x8, [x0]
  hvc #0
1:
  b 1b

.globl el1_test_2
el1_test_2:
  mov x0, #0x10000
  mov x1, #456
  str x1, [x0]
  ldr x8, [x0]
  hvc #0
2:
  b 2b
