#include "task.h"
#include "entry.h"
#include "mm.h"
#include "printf.h"
#include "sched.h"
#include "user.h"
#include "utils.h"

static struct pt_regs *task_pt_regs(struct task_struct *tsk) {
  unsigned long p = (unsigned long)tsk + THREAD_SIZE - sizeof(struct pt_regs);
  return (struct pt_regs *)p;
}

static int prepare_el1_switching(unsigned long start, unsigned long size,
                                 unsigned long pc) {
  struct pt_regs *regs = task_pt_regs(current);
  regs->pstate = PSR_MODE_EL1h;
  regs->pc = pc;
  regs->sp = 2 * PAGE_SIZE;

  unsigned long code_page = allocate_user_page(current, 0);
  if (code_page == 0) {
    return -1;
  }
  memcpy(code_page, start, size);
  set_cpu_sysregs(current);
  unsigned long do_at(unsigned long);
  unsigned long _get_id_aa64mmfr0_el1(void);
  printf("psize=%x\r\n", _get_id_aa64mmfr0_el1()&0xf);
  printf("do_at: %x\r\n", do_at(0));
  while(0);
  return 0;
}

static void prepare_vmtask(unsigned long arg) {
  printf("task: arg=%d, EL=%d\r\n", arg, get_el());
  unsigned int insns[] = {
    0x52800008,  // mov w8, #0
    0xd4000002,  // hvc #0
    0xd65f03c0,  // ret
  };
  int err = prepare_el1_switching((unsigned long)insns, (unsigned long)(sizeof(insns)), 0);
  if (err < 0) {
    printf("task: prepare_el1_switching() failed.\n\r");
  }
  printf("task: entering el1...\n\r");
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

int create_vmtask(unsigned long arg) {
  preempt_disable();
  struct task_struct *p;

  unsigned long page = allocate_kernel_page();
  p = (struct task_struct *)page;
  struct pt_regs *childregs = task_pt_regs(p);

  if (!p)
    return -1;

  p->cpu_context.x19 = (unsigned long)prepare_vmtask;
  p->cpu_context.x20 = arg;
  p->flags = PF_KTHREAD;
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
