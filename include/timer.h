#pragma once

void timer_init(void);
void handle_timer1_irq(void);
void handle_timer3_irq(void);
unsigned long get_physical_timer_count(void);
