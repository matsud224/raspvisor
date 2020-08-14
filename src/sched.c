#include "sched.h"
#include "irq.h"
#include "mm.h"
#include "utils.h"
#include "debug.h"
#include "board.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct *task[NR_TASKS] = {
    &(init_task),
};
int nr_tasks = 1;

void preempt_disable(void) { current->preempt_count++; }

void preempt_enable(void) { current->preempt_count--; }

void _schedule(void) {
  preempt_disable();
  int next, c;
  struct task_struct *p;
  while (1) {
    c = -1;
    next = 0;
    for (int i = 0; i < NR_TASKS; i++) {
      p = task[i];
      if (p && p->state == TASK_RUNNING && p->counter > c) {
        c = p->counter;
        next = i;
      }
    }
    if (c) {
      break;
    }
    for (int i = 0; i < NR_TASKS; i++) {
      p = task[i];
      if (p) {
        p->counter = (p->counter >> 1) + p->priority;
      }
    }
  }
  switch_to(task[next]);
  preempt_enable();
}

void schedule(void) {
  current->counter = 0;
  _schedule();
}

void set_cpu_sysregs(struct task_struct *task) {
  set_stage2_pgd(task->mm.first_table, task->pid);
  _set_sysregs(&task->cpu_sysregs);
}

void switch_to(struct task_struct *next) {
  if (current == next)
    return;
  struct task_struct *prev = current;
  current = next;

  set_cpu_sysregs(current);
  if (current->board_ops->is_interrupt_required)
    if (current->board_ops->is_interrupt_required(current))
      generate_virq();

  cpu_switch_to(prev, next);
}

void schedule_tail(void) { preempt_enable(); }

void timer_tick() {
  --current->counter;
  if (current->counter > 0 || current->preempt_count > 0) {
    return;
  }
  current->counter = 0;
  _schedule();
}

void exit_task() {
  preempt_disable();
  for (int i = 0; i < NR_TASKS; i++) {
    if (task[i] == current) {
      task[i]->state = TASK_ZOMBIE;
      break;
    }
  }
  preempt_enable();
  schedule();
}
