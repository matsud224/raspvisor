#pragma once

#include "sched.h"
#include "task.h"

struct raw_binary_loader_args {
  unsigned long load_addr;
  unsigned long entry_point;
  unsigned long sp;
  const char *filename;
};

int raw_binary_loader (void *, struct pt_regs *);

struct linux_loader_args {
  const char *kernel_image;
  const char *device_tree;
  const char *initramfs;
};

int linux_loader (void *, struct pt_regs *);
