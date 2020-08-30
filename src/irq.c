#include "peripherals/irq.h"
#include "arm/sysregs.h"
#include "entry.h"
#include "system_timer.h"
#include "generic_timer.h"
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
  put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_3_BIT);
  put32(ENABLE_IRQS_1, AUX_IRQ_BIT);
}

void show_invalid_entry_message(int type, unsigned long esr,
                                unsigned long elr, unsigned long far) {
  PANIC("uncaught exception(%s) esr: %x, elr: %x, far: %x", entry_error_messages[type],
         esr, elr, far);
}

void handle_irq(void) {
  unsigned int core_irq_src = get32(CORE0_IRQ_SOURCE);
  unsigned int gpu_irq_src = get32(IRQ_PENDING_1);

  if (core_irq_src & INT_SRC_TIMER) {
    handle_generic_timer_irq();
  }

  if (core_irq_src & INT_SRC_GPU) {
    if (gpu_irq_src & SYSTEM_TIMER_IRQ_1_BIT) {
      handle_systimer1_irq();
    }
    if (gpu_irq_src & SYSTEM_TIMER_IRQ_3_BIT) {
      handle_systimer3_irq();
    }
    if (gpu_irq_src & AUX_IRQ_BIT) {
      handle_uart_irq();
    }
  }
}
