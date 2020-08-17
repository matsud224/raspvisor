#pragma once

#include "sched.h"

/*
 * PSR bits
 */
#define PSR_MODE_EL0t 0x00000000
#define PSR_MODE_EL1t 0x00000004
#define PSR_MODE_EL1h 0x00000005
#define PSR_MODE_EL2t 0x00000008
#define PSR_MODE_EL2h 0x00000009
#define PSR_MODE_EL3t 0x0000000c
#define PSR_MODE_EL3h 0x0000000d

typedef int (*loader_func_t)(void *, unsigned long *, unsigned long *);

struct pt_regs *task_pt_regs(struct task_struct *);
int create_task(loader_func_t, void *);
void init_task_console(struct task_struct *);
int is_uart_forwarded_task(struct task_struct *);
void flush_task_console(struct task_struct *);
void increment_current_pc(int);

struct pt_regs {
  unsigned long regs[31];
  unsigned long sp;
  unsigned long pc;
  unsigned long pstate;
};
