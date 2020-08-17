#include "sched.h"
#include "irq.h"
#include "mm.h"
#include "utils.h"
#include "debug.h"
#include "board.h"
#include "task.h"

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

void set_cpu_sysregs(struct task_struct *tsk) {
  set_stage2_pgd(tsk->mm.first_table, tsk->pid);
  _set_sysregs(&tsk->cpu_sysregs);
}

void set_cpu_virtual_interrupt(struct task_struct *tsk) {
  if (HAVE_FUNC(tsk->board_ops, is_irq_asserted) &&
    tsk->board_ops->is_irq_asserted(current))
    assert_virq();
  else
    clear_virq();

  if (HAVE_FUNC(tsk->board_ops, is_fiq_asserted) &&
    tsk->board_ops->is_fiq_asserted(current))
    assert_vfiq();
  else
    clear_vfiq();
}

void switch_to(struct task_struct *next) {
  if (current == next)
    return;
  struct task_struct *prev = current;
  current = next;

  set_cpu_sysregs(current);

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

void vm_entering_work() {
  set_cpu_virtual_interrupt(current);
  if (HAVE_FUNC(current->board_ops, entering_vm))
    current->board_ops->entering_vm(current);

  if (is_uart_forwarded_task(current))
    flush_task_console(current);
}

void vm_leaving_work() {
  if (HAVE_FUNC(current->board_ops, leaving_vm))
    current->board_ops->leaving_vm(current);

  if (is_uart_forwarded_task(current))
    flush_task_console(current);
}

const char *task_state_str[] = {
  "RUNNING",
  "ZOMBIE",
};

void show_task_list() {
  preempt_disable();
  printf("%3s %8s %7s %7s\n", "id", "state", "pages", "traps");
  for (int i = 0; i < nr_tasks; i++) {
    struct task_struct *tsk = task[i];
    printf("%3d %8s %7d %7d\n", tsk->pid, task_state_str[tsk->state],
        tsk->mm.user_pages_count, tsk->stat.trap_count);
  }
  preempt_enable();
}
