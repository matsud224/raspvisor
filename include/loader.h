#pragma once

#include "sched.h"

struct raw_binary_loader_args {
  unsigned long load_addr;
  unsigned long entry_point;
  unsigned long sp;
  const char *filename;
};

int raw_binary_loader (void *, unsigned long *, unsigned long *);
int test_program_loader (void *, unsigned long *, unsigned long *);
