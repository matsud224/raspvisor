#pragma once

#define THREAD_CPU_CONTEXT 0 // offset of cpu_context in task_struct

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096

#define NR_TASKS 64

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS - 1]

#define TASK_RUNNING   0
#define TASK_ZOMBIE    1

struct board_ops;

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
  // not trapped (save & restore performed)
  unsigned long sctlr_el1;
  unsigned long ttbr0_el1;
  unsigned long ttbr1_el1;
  unsigned long tcr_el1;
  unsigned long esr_el1;
  unsigned long far_el1;
  unsigned long afsr0_el1;
  unsigned long afsr1_el1;
  unsigned long mair_el1;
  unsigned long amair_el1;
  unsigned long contextidr_el1;

  unsigned long cpacr_el1;
  unsigned long elr_el1;
  unsigned long fpcr;
  unsigned long fpsr;
  unsigned long midr_el1; // ro
  unsigned long mpidr_el1; // ro
  unsigned long par_el1;
  unsigned long sp_el0;
  unsigned long sp_el1;
  unsigned long spsr_el1;
  unsigned long tpidr_el0;
  unsigned long tpidr_el1;
  unsigned long tpidrro_el0;
  unsigned long vbar_el1;

  // trapped by TACR
  unsigned long actlr_el1; // rw

  // trapped by TID3
  unsigned long id_pfr0_el1; // ro
  unsigned long id_pfr1_el1; // ro
  unsigned long id_mmfr0_el1; // ro
  unsigned long id_mmfr1_el1; // ro
  unsigned long id_mmfr2_el1; // ro
  unsigned long id_mmfr3_el1; // ro
  unsigned long id_isar0_el1; // ro
  unsigned long id_isar1_el1; // ro
  unsigned long id_isar2_el1; // ro
  unsigned long id_isar3_el1; // ro
  unsigned long id_isar4_el1; // ro
  unsigned long id_isar5_el1; // ro
  unsigned long mvfr0_el1; // ro
  unsigned long mvfr1_el1; // ro
  unsigned long mvfr2_el1; // ro
  unsigned long id_aa64pfr0_el1; // ro
  unsigned long id_aa64pfr1_el1; // ro
  unsigned long id_aa64dfr0_el1; // ro
  unsigned long id_aa64dfr1_el1; // ro
  unsigned long id_aa64isar0_el1; // ro
  unsigned long id_aa64isar1_el1; // ro
  unsigned long id_aa64mmfr0_el1; // ro
  unsigned long id_aa64mmfr1_el1; // ro
  unsigned long id_aa64afr0_el1; // ro
  unsigned long id_aa64afr1_el1; // ro

  // trapped by TID2
  unsigned long ctr_el0; // ro
  unsigned long ccsidr_el1; // ro
  unsigned long clidr_el1; // ro
  unsigned long csselr_el1; // rw

  // trapped by TID1
  unsigned long aidr_el1; // ro
  unsigned long revidr_el1; // ro

  // system timer
  unsigned long cntkctl_el1;
  unsigned long cntp_ctl_el0;
  unsigned long cntp_cval_el0;
  unsigned long cntp_tval_el0;
  unsigned long cntv_ctl_el0;
  unsigned long cntv_cval_el0;
  unsigned long cntv_tval_el0;
};


struct mm_struct {
  unsigned long first_table;
  int user_pages_count;
  int kernel_pages_count;
};

struct task_stat {
  long wfx_trap_count;
  long hvc_trap_count;
  long sysreg_trap_count;
  long pf_count;
  long mmio_count;
};

struct task_console {
  struct fifo *in_fifo;
  struct fifo *out_fifo;
};

struct task_struct {
  struct cpu_context cpu_context;
  long state;
  long counter;
  long priority;
  long pid; // used as VMID
  unsigned long flags;
  const char *name;
  const struct board_ops *board_ops;
  void *board_data;
  struct mm_struct mm;
  struct cpu_sysregs cpu_sysregs;
  struct task_stat stat;
  struct task_console console;
};

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void set_cpu_virtual_interrupt(struct task_struct *);
void set_cpu_sysregs(struct task_struct *);
extern void switch_to(struct task_struct *);
extern void cpu_switch_to(struct task_struct *, struct task_struct *);
extern void exit_task(void);
extern void show_task_list(void);

#define INIT_TASK  \
  {  \
    /* cpu_context */ {0}, \
    /* state etc */    0, 0, 1, 0, 0, "", 0, 0,  \
    /* mm */          {0},  \
    /* cpu_sysregs */ {0},  \
    /* stat */        {0},  \
    /* console */     {0},  \
  }
#endif
