#include <stddef.h>
#include <stdint.h>

#include "irq.h"
#include "mini_uart.h"
#include "printf.h"
#include "sched.h"
#include "task.h"
#include "timer.h"
#include "utils.h"
#include "mm.h"

int test_program_loader (unsigned long arg, unsigned long *pc,
    unsigned long *sp) {
  const unsigned long entry_point = 0x80000;

  extern unsigned long el1_test_begin;
  extern unsigned long el1_test_end;
  unsigned long begin = (unsigned long)&el1_test_begin;
  unsigned long end = (unsigned long)&el1_test_end;
  unsigned long size = end - begin;

  unsigned long code_page = allocate_user_page(current, entry_point);
  if (code_page == 0) {
    return -1;
  }
  memcpy(code_page, begin, size);

  *pc = entry_point;
  *sp = 2 * PAGE_SIZE;

  return 0;
}

void hypervisor_main() {
  uart_init();
  init_printf(NULL, putc);
  printf("Starting hypervisor (EL %d)...\r\n", get_el());
  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

  int res = create_task(test_program_loader, 0);
  if (res < 0) {
    printf("error while starting kernel process");
    return;
  }

  printf("Starting vm...\r\n");
  while (1) {
    schedule();
  }
}
