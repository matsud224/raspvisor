#pragma once

void timer_init(void);
void handle_timer_irq(void);
unsigned long get_physical_timer_count(void);
