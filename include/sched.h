#pragma once

#define THREAD_CPU_CONTEXT 0 // offset of cpu_context in task_struct

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096

#define NR_TASKS 64

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS - 1]

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

#define PF_KTHREAD 0x00000002

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];
extern int nr_tasks;

struct cpu_context {
  unsigned long x19;
  unsigned long x20;
  unsigned long x21;
  unsigned long x22;
  unsigned long x23;
  unsigned long x24;
  unsigned long x25;
  unsigned long x26;
  unsigned long x27;
  unsigned long x28;
  unsigned long fp;
  unsigned long sp;
  unsigned long pc;
};

struct cpu_sysregs {
  unsigned long sctlr_el1;
  unsigned long spsr_el1;
  unsigned long ttbr0_el1;
  unsigned long ttbr1_el1;
  unsigned long tcr_el1;
  unsigned long mair_el1;
};

#define MAX_PROCESS_PAGES 16

struct user_page {
  unsigned long phys_addr;
  unsigned long virt_addr;
};

struct mm_struct {
  unsigned long first_table;
  int user_pages_count;
  struct user_page user_pages[MAX_PROCESS_PAGES];
  int kernel_pages_count;
  unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct task_struct {
  struct cpu_context cpu_context;
  long state;
  long counter;
  long priority;
  long preempt_count;
  long pid; // used as VMID
  unsigned long flags;
  struct mm_struct mm;
  struct cpu_sysregs cpu_sysregs;
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void set_cpu_sysregs(struct task_struct *task);
extern void switch_to(struct task_struct *next);
extern void cpu_switch_to(struct task_struct *prev, struct task_struct *next);
extern void exit_task(void);

#define INIT_TASK                                                              \
  /*cpu_context*/ {                                                            \
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* state etc */ 0, 0, 15, 0, 0,   \
        PF_KTHREAD, /* mm */ {0, 0, {{0}}, 0, {0}},                            \
        /*cpu_sysregs*/ {0, 0, 0, 0, 0, 0},                                    \
  }
#endif
