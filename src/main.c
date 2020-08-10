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
  extern unsigned long el1_test_1;
  extern unsigned long el1_test_2;
  extern unsigned long el1_test_begin;
  extern unsigned long el1_test_end;

  unsigned long begin = (unsigned long)&el1_test_begin;
  unsigned long end = (unsigned long)&el1_test_end;
  unsigned long size = end - begin;
  unsigned long func = (unsigned long)&el1_test_1;

  switch (arg) {
  case 1:
    func = (unsigned long)&el1_test_1;
    break;
  case 2:
    func = (unsigned long)&el1_test_2;
    break;
  }
  unsigned long entry_point = func - begin;

  unsigned long code_page = allocate_user_page(current, 0);
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
  printf("=== raspvisor ===\n");
  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

  if (create_task(test_program_loader, 1) < 0) {
    printf("error while starting task #1");
    return;
  }

  if (create_task(test_program_loader, 2) < 0) {
    printf("error while starting task #2");
    return;
  }

  while (1) {
    schedule();
  }
}
