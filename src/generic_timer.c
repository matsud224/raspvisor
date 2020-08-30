#include "arm/sysregs.h"
#include "peripherals/irq.h"
#include "generic_timer.h"
#include "utils.h"
#include "debug.h"

void generic_timer_init() {
  set_cnthp_ctl(0);
  put32(CORE_TIMER_PRESCALER, 0x80000000);
  put32(CORE0_TIMER_IRQCNTL, 0xf);
  set_cnthp_ctl(CNTHP_CTL_ENABLE);
  set_cnthp_tval(GENERIC_TIMER_FREQ * 10);
}

void handle_generic_timer_irq() {
  INFO("!!! generic_timer: fired");
  set_cnthp_tval(GENERIC_TIMER_FREQ * 10);
}
