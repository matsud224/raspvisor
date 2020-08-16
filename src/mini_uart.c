#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "utils.h"
#include "sched.h"
#include "fifo.h"
#include "printf.h"
#include "task.h"

static void _uart_send(char c) {
  while (1) {
    if (get32(AUX_MU_LSR_REG) & 0x20)
      break;
  }
  put32(AUX_MU_IO_REG, c);
}

void uart_send(char c) {
  if (c == '\n') {
    _uart_send('\r');
    _uart_send('\n');
  } else {
    _uart_send(c);
  }
}

char uart_recv(void) {
  while (1) {
    if (get32(AUX_MU_LSR_REG) & 0x01)
      break;
  }

  char c = get32(AUX_MU_IO_REG) & 0xFF;
  if (c == '\r')
    c = '\n';

  return c;
}

#define ESCAPE_CHAR  '?'
static int uart_forwarded_task = 0;

void handle_uart_irq(void) {
  static char prev = '\0';

  char received = get32(AUX_MU_IO_REG) & 0xff;

  if (prev == ESCAPE_CHAR) {
    if (isdigit(received)) {
      uart_forwarded_task = received - '0';
      printf("\nswitched to %d\n", uart_forwarded_task);
      struct task_struct *tsk = task[uart_forwarded_task];
      if (tsk->state == TASK_RUNNING)
        flush_task_console(tsk);
    } else if (received == 'l') {
      show_task_list();
    }
  } else if (received != ESCAPE_CHAR) {
    struct task_struct *tsk = task[uart_forwarded_task];
    if (tsk->state == TASK_RUNNING) {
      enqueue_fifo(tsk->console.in_fifo, received);
    }
  }
  prev = received;
  printf("received: %c\n", prev);

  put32(AUX_MU_IIR_REG, 0x2); // clear interrupt
}

void uart_init(void) {
  unsigned int selector;

  selector = get32(GPFSEL1);
  selector &= ~(7 << 12); // clean gpio14
  selector |= 2 << 12;    // set alt5 for gpio14
  selector &= ~(7 << 15); // clean gpio15
  selector |= 2 << 15;    // set alt5 for gpio15
  put32(GPFSEL1, selector);

  put32(GPPUD, 0);
  delay(150);
  put32(GPPUDCLK0, (1 << 14) | (1 << 15));
  delay(150);
  put32(GPPUDCLK0, 0);

  put32(AUX_ENABLES, 1); // Enable mini uart (this also enables access to it registers)
  put32(AUX_MU_CNTL_REG, 0); // Disable auto flow control and disable receiver
                             // and transmitter (for now)
  put32(AUX_MU_IER_REG, 1);    // Enable receive interrupt
  put32(AUX_MU_LCR_REG, 3);    // Enable 8 bit mode
  put32(AUX_MU_MCR_REG, 0);    // Set RTS line to be always high
  put32(AUX_MU_BAUD_REG, 270); // Set baud rate to 115200

  put32(AUX_MU_CNTL_REG, 3); // Finally, enable transmitter and receiver
}

// This function is required by printf function
void putc(void *p, char c) { uart_send(c); }
