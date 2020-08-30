#pragma once

#include <inttypes.h>

void generic_timer_init(void);
void handle_generic_timer_irq(void);

#define CNTHP_CTL_ENABLE  (1 << 0)
#define CNTHP_CTL_IMASK   (1 << 1)
#define CNTHP_CTL_ISTATUS (1 << 2)

#define CORE_TIMER_PRESCALER  0x4000008

unsigned long get_generic_timer_freq(void);
void set_cnthp_ctl(unsigned long);
unsigned long get_cnthp_ctl(void);
void set_cnthp_cval(unsigned long);
unsigned long get_cnthp_cval(void);
void set_cnthp_tval(unsigned long);
unsigned long get_cnthp_tval(void);
