#pragma once

void uart_init(void);
void handle_uart_irq(void);
char uart_recv(void);
void uart_send(char c);
void putc(void *p, char c);
