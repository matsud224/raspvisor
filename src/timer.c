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

// for task switch
void handle_timer1_irq(void) {
  curVal += interval;
  put32(TIMER_C1, curVal);
  put32(TIMER_CS, TIMER_CS_M1);
  timer_tick();
}

// for vm's interrupt
void handle_timer3_irq(void) {
  put32(TIMER_CS, TIMER_CS_M3);
}

unsigned long get_physical_timer_count() {
  unsigned long clo = get32(TIMER_CLO);
  unsigned long chi = get32(TIMER_CHI);
  return clo | (chi << 32);
}

void show_systimer_info() {
  printf("HI: %x\nLO: %x\nCS:%x\nC1: %x\n",
      get32(TIMER_CHI), get32(TIMER_CLO),
      get32(TIMER_CS), get32(TIMER_C1));
}
