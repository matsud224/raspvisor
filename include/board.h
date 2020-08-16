#pragma once

#include "sched.h"

#define HAVE_FUNC(ops, func, ...) ((ops) && ((ops)->func))

struct board_ops {
  void (*initialize)(struct task_struct *);
  unsigned long (*mmio_read)(struct task_struct *, unsigned long);
  void (*mmio_write)(struct task_struct *, unsigned long, unsigned long);
  void (*entering_vm)(struct task_struct *);
  void (*leaving_vm)(struct task_struct *);
  int (*is_irq_asserted)(struct task_struct *);
  int (*is_fiq_asserted)(struct task_struct *);
};
