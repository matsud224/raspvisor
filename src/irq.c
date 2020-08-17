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
                                unsigned long elr, unsigned long far) {
  PANIC("uncaught exception(%s) esr: %x, elr: %x, far: %x", entry_error_messages[type],
         esr, elr, far);
}

void handle_irq(void) {
  unsigned int irq = get32(IRQ_PENDING_1);
  if (irq & SYSTEM_TIMER_IRQ_1_BIT) {
    irq &= ~SYSTEM_TIMER_IRQ_1_BIT;
    handle_timer_irq();
  }
  if (irq & AUX_IRQ_BIT) {
    irq &= ~AUX_IRQ_BIT;
    handle_uart_irq();
  }

  if (irq)
    WARN("unknown pending irq: %x", irq);
}
