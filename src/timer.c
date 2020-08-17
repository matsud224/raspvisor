#include "peripherals/timer.h"
#include "sched.h"
#include "utils.h"
#include "debug.h"

const unsigned int interval = 30000;
unsigned int curVal = 0;

void timer_init(void) {
  curVal = get32(TIMER_CLO);
  curVal += interval;
  put32(TIMER_C1, curVal);
}

void handle_timer_irq(void) {
  curVal += interval;
  put32(TIMER_C1, curVal);
  put32(TIMER_CS, TIMER_CS_M1);
  timer_tick();
}

unsigned long get_physical_timer_count() {
  unsigned long clo = get32(TIMER_CLO);
  unsigned long chi = get32(TIMER_CHI);
  return clo | (chi << 32);
}


