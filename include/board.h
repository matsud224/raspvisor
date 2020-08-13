#pragma once

#include "sched.h"

struct board_ops {
  void (*initialize)(struct task_struct *);
  unsigned long (*mmio_read)(unsigned long, int);
  void (*mmio_write)(unsigned long, unsigned long, int);
};
