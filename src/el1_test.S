.org 0x0

.globl el1_test_1
el1_test_1:
  mrs x8, tpidr_el1
  hvc #0
1:
  b 1b

.globl el1_test_2
el1_test_2:
  mrs x8, id_aa64mmfr0_el1
  hvc #0
2:
  b 2b
