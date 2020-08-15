#pragma once

#include "sched.h"

int load_file_to_memory(struct task_struct *tsk, const char *name, unsigned long va);
