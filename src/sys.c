#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "task.h"
#include "utils.h"

void sys_notify() { printf("HVC!\r\n"); }

void sys_exit() { exit_process(); }

void *const hvc_table[] = {sys_notify, sys_exit};
