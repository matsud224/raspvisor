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
#include "sd.h"
#include "debug.h"
#include "loader.h"

void hypervisor_main() {
  uart_init();
  init_printf(NULL, putc);
  printf("=== raspvisor ===\n");

  irq_vector_init();
  timer_init();
  enable_interrupt_controller();
  enable_irq();

  if (sd_init() < 0)
    PANIC("sd_init() failed.");

  struct raw_binary_loader_args bl_args = {
    .load_addr = 0x0,
    .entry_point = 0x0,
    .sp = 0x4000,
    .filename = "test.bin",
  };
  if (create_task(raw_binary_loader, &bl_args) < 0) {
    printf("error while starting task #1");
    return;
  }

  /*
  if (create_task(test_program_loader, (void *)2) < 0) {
    printf("error while starting task #2");
    return;
  }
  */

  while (1) {
    //run_shell();
    schedule();
  }
}
