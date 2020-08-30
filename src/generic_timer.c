#include "arm/sysregs.h"
#include "peripherals/irq.h"
#include "generic_timer.h"
#include "utils.h"
#include "debug.h"

void generic_timer_init() {
  set_cnthp_ctl(0);
  put32(CORE_TIMER_PRESCALER, 0x80000000);
  //put32(CORE0_TIMER_IRQCNTL, 0xa); // nCNTPNSIRQ & nCNTVIRQ
}

void handle_generic_timer_irq() {
}

int is_generic_timer_interrupt_asserted(struct task_struct *tsk) {
  if ((tsk->cpu_sysregs.cntp_ctl_el0 & 0x1) &&
      (tsk->cpu_sysregs.cntp_ctl_el0 & 0x4))
    return 1;

  if ((tsk->cpu_sysregs.cntv_ctl_el0 & 0x1) &&
      (tsk->cpu_sysregs.cntv_ctl_el0 & 0x4))
    return 1;

  return 0;
}
