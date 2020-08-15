#include "peripherals/base.h"
#include "peripherals/mini_uart.h"
#include "peripherals/timer.h"

.org 0x0

.globl el1_test_1
el1_test_1:
  ldr x0, =(TIMER_CS)
  hvc #0
1:
  b 1b

.globl el1_test_2
el1_test_2:
  hvc #0
2:
  b 2b
