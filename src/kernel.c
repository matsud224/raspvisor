#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "mini_uart.h"
#include "printf.h"
#include "sched.h"
#include "sys.h"
#include "task.h"
#include "timer.h"
#include "user.h"
#include "utils.h"

void kernel_main() {
  uart_init();
  init_printf(NULL, putc);
  printf("Starting hypervisor (EL %d)...\r\n", get_el());
  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

  int res = create_vmtask(0);
  if (res < 0) {
    printf("error while starting kernel process");
    return;
  }

  printf("Starting process...\r\n");
  while (1) {
    schedule();
  }
}
