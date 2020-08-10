#include "task.h"
#include "entry.h"
#include "mm.h"
#include "sched.h"
#include "utils.h"
#include "debug.h"

struct pt_regs *task_pt_regs(struct task_struct *tsk) {
  unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}

static void prepare_task(loader_func_t loader, unsigned long arg) {
  INFO("loading...");

  struct pt_regs *regs = task_pt_regs(current);
  regs->pstate = PSR_MODE_EL1h;

  if (loader(arg, &regs->pc, &regs->sp) < 0) {
    PANIC("failed to load");
  }

  set_cpu_sysregs(current);

  INFO("entering el1...");
}

static struct cpu_sysregs initial_sysregs;

static void prepare_initial_sysregs(void) {
  static int is_first_call = 1;

  if (!is_first_call)
    return;

  _get_sysregs(&initial_sysregs);
  initial_sysregs.sctlr_el1 &= ~1; // surely disable MMU

  is_first_call = 0;
}

int create_task(loader_func_t loader, unsigned long arg) {
  preempt_disable();
  struct task_struct *p;

  unsigned long page = allocate_kernel_page();
  p = (struct task_struct *)page;
  struct pt_regs *childregs = task_pt_regs(p);

  if (!p)
    return -1;

  p->cpu_context.x19 = (unsigned long)prepare_task;
  p->cpu_context.x20 = (unsigned long)loader;
  p->cpu_context.x21 = arg;
  p->flags = 0;
  p->priority = current->priority;
  p->state = TASK_RUNNING;
  p->counter = p->priority;
  p->preempt_count = 1; // disable preemtion until schedule_tail

  prepare_initial_sysregs();
  memcpy((unsigned long)&p->cpu_sysregs, (unsigned long)&initial_sysregs,
         sizeof(struct cpu_sysregs));

  p->cpu_context.pc = (unsigned long)switch_from_kthread;
  p->cpu_context.sp = (unsigned long)childregs;
  int pid = nr_tasks++;
  task[pid] = p;
  p->pid = pid;

  preempt_enable();
  return pid;
}
