#include "peripherals/irq.h"
#include "arm/sysregs.h"
#include "entry.h"
#include "timer.h"
#include "utils.h"
#include "sched.h"
#include "debug.h"
#include "mini_uart.h"

const char *entry_error_messages[] = {
  "SYNC_INVALID_EL2",
  "IRQ_INVALID_EL2",
  "FIQ_INVALID_EL2",
  "ERROR_INVALID_EL2",

  "SYNC_INVALID_EL01_64",
  "IRQ_INVALID_EL01_64",
  "FIQ_INVALID_EL01_64",
  "ERROR_INVALID_EL01_64",

  "SYNC_INVALID_EL01_32",
  "IRQ_INVALID_EL01_32",
  "FIQ_INVALID_EL01_32",
  "ERROR_INVALID_EL01_32",
};

void enable_interrupt_controller() {
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1_BIT);
  put32(ENABLE_IRQS_1, AUX_IRQ_BIT);
}

void show_invalid_entry_message(int type, unsigned long esr,
                                unsigned long address) {
  PANIC("uncaught exception(%s) ESR: %x, address: %x", entry_error_messages[type],
         esr, address);
}

void handle_irq(void) {
  unsigned int irq = get32(IRQ_PENDING_1);
  switch (irq) {
  case (SYSTEM_TIMER_IRQ_1_BIT):
    handle_timer_irq();
    break;
  case (AUX_IRQ_BIT):
    handle_uart_irq();
    break;
  default:
    WARN("unknown pending irq: %x", irq);
  }
}
