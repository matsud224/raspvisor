#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "task.h"
#include "utils.h"

void sys_write(char *buf) { printf(buf); }

void sys_exit() { exit_process(); }

void *const hvc_table[] = {sys_write, sys_exit};
