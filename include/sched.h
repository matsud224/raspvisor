#pragma once

#define THREAD_CPU_CONTEXT 0 // offset of cpu_context in task_struct

#ifndef __ASSEMBLER__

#define THREAD_SIZE 4096

#define NR_TASKS 64

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS - 1]

#define TASK_RUNNING 0
#define TASK_ZOMBIE 1

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
  unsigned long midr_el1;
  unsigned long mpidr_el1;
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
  unsigned long id_pfr0_el1; // r
  unsigned long id_pfr1_el1; // r
  unsigned long id_mmfr0_el1; // r
  unsigned long id_mmfr1_el1; // r
  unsigned long id_mmfr2_el1; // r
  unsigned long id_mmfr3_el1; // r
  unsigned long id_isar0_el1; // r
  unsigned long id_isar1_el1; // r
  unsigned long id_isar2_el1; // r
  unsigned long id_isar3_el1; // r
  unsigned long id_isar4_el1; // r
  unsigned long id_isar5_el1; // r
  unsigned long mvfr0_el1; // r
  unsigned long mvfr1_el1; // r
  unsigned long mvfr2_el1; // r
  unsigned long id_aa64pfr0_el1; // r
  unsigned long id_aa64pfr1_el1; // r
  unsigned long id_aa64dfr0_el1; // r
  unsigned long id_aa64dfr1_el1; // r
  unsigned long id_aa64isar0_el1; // r
  unsigned long id_aa64isar1_el1; // r
  unsigned long id_aa64mmfr0_el1; // r
  unsigned long id_aa64mmfr1_el1; // r
  unsigned long id_aa64afr0_el1; // r
  unsigned long id_aa64afr1_el1; // r

  // trapped by TID2
  unsigned long ctr_el0; // r
  unsigned long ccsidr_el1; // r
  unsigned long clidr_el1; // r
  unsigned long csselr_el1; // rw

  // trapped by TID1
  unsigned long aidr_el1; // r
  unsigned long revidr_el1; // r

  // system timer
  /*
  unsigned long cntfrq_el0;
  unsigned long cntkctl_el1;
  unsigned long cntp_ctl_el0;
  unsigned long cntp_cval_el0;
  unsigned long cntp_tval_el0;
  unsigned long cntpct_el0;
  unsigned long cntps_ctl_el1;
  unsigned long cntps_cval_el1;
  unsigned long cntps_tval_el1;
  unsigned long cntv_ctl_el0;
  unsigned long cntv_cval_el0;
  unsigned long cntv_tval_el0;
  unsigned long cntvct_el0;
  */
};

struct bcm2835 {
  struct aux_peripherals_regs {
    unsigned int aux_enables;
    unsigned int aux_mu_io;
    unsigned int aux_mu_ier;
    unsigned int aux_mu_iir;
    unsigned int aux_mu_lcr;
    unsigned int aux_mu_mcr;
    unsigned int aux_mu_lsr;
    unsigned int aux_mu_msr;
    unsigned int aux_mu_scratch;
    unsigned int aux_mu_cntl;
    unsigned int aux_mu_stat;
    unsigned int aux_mu_baud;
  } aux;

  struct systimer_regs {
    unsigned long cs;
    unsigned long clo;
    unsigned long chi;
    unsigned long c0;
    unsigned long c1;
    unsigned long c2;
    unsigned long c3;
  } systimer;
};

struct mm_struct {
  unsigned long first_table;
  int user_pages_count;
  int kernel_pages_count;
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
  struct bcm2835 bcm2835;
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

#define INIT_TASK  \
  {  \
    /* cpu_context */ {0}, \
    /* state etc */    0, 0, 15, 0, 0, 0,  \
    /* mm */          {0},  \
    /* cpu_sysregs */ {0},  \
    /* bcm2835 */     {{0}, {0}},  \
  }
#endif
