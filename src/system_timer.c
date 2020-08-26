#include "sched.h"
#include "utils.h"
#include "debug.h"
#include "peripherals/system_timer.h"

const unsigned int interval = 400000;

void systimer_init() {
  put32(SYSTIMER_C1, get32(SYSTIMER_CLO) + interval);
}

// for task switching
void handle_systimer1_irq() {
  put32(SYSTIMER_C1, get32(SYSTIMER_CLO) + interval);
  put32(SYSTIMER_CS, SYSTIMER_CS_M1);
  timer_tick();
}

// for timer emulation
void handle_systimer3_irq() {
  put32(SYSTIMER_CS, SYSTIMER_CS_M3);
}

unsigned long get_physical_systimer_count() {
  unsigned long clo = get32(SYSTIMER_CLO);
  unsigned long chi = get32(SYSTIMER_CHI);
  return clo | (chi << 32);
}
