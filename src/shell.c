#include "sched.h"
#include "printf.h"
#include "mini_uart.h"
#include "utils.h"

void readline(char *buf, size_t size) {
  int index = 0;
  while(index < size - 1) {
    char c = uart_recv();
    if (c == '\r') c = '\n';
    uart_send(c); // echo back
    if (c == '\n')
      break;
    buf[index++] = c;
  }
  buf[index] = '\0';
}

void run_shell() {
  char buf[64];
  while(1) {
    printf(">>> ");
    readline(buf, sizeof(buf));

    if (strncmp("", buf, sizeof(buf)) == 0) {
      // pass
    } else if (strncmp("list", buf, sizeof(buf)) == 0) {
      show_task_list();
    } else if (strncmp("help", buf, sizeof(buf)) == 0) {
      printf("help...\n");
    } else {
      printf("unknown command\n");
    }
  }
}
