#include "sync_exc.h"
#include "irq.h"
#include "mm.h"
#include "sched.h"
#include "debug.h"
#include "task.h"
#include "arm/sysregs.h"

const char *sync_error_reasons[] = {
  "Unknown reason.",
  "Trapped WFI or WFE instruction execution.",
  "(unknown)",
  "Trapped MCR or MRC access with (coproc==0b1111).",
  "Trapped MCRR or MRRC access with (coproc==0b1111).",
  "Trapped MCR or MRC access with (coproc==0b1110).",
  "Trapped LDC or STC access.",
  "Access to SVE, Advanced SIMD, or floating-point functionality trapped by CPACR_EL1.FPEN, CPTR_EL2.FPEN, CPTR_EL2.TFP, or CPTR_EL3.TFP control.",
  "Trapped VMRS access, from ID group trap.",
  "Trapped use of a Pointer authentication instruction because HCR_EL2.API == 0 || SCR_EL3.API == 0.",
  "(unknown)",
  "(unknown)",
  "Trapped MRRC access with (coproc==0b1110).",
  "Branch Target Exception.",
  "Illegal Execution state.",
  "(unknown)",
  "(unknown)",
  "SVC instruction execution in AArch32 state.",
  "HVC instruction execution in AArch32 state.",
  "SMC instruction execution in AArch32 state.",
  "(unknown)",
  "SVC instruction execution in AArch64 state.",
  "HVC instruction execution in AArch64 state.",
  "SMC instruction execution in AArch64 state.",
  "Trapped MSR, MRS or System instruction execution in AArch64 state.",
  "Access to SVE functionality trapped as a result of CPACR_EL1.ZEN, CPTR_EL2.ZEN, CPTR_EL2.TZ, or CPTR_EL3.EZ.",
  "Trapped ERET, ERETAA, or ERETAB instruction execution.",
  "(unknown)",
  "Exception from a Pointer Authentication instruction authentication failure.",
  "(unknown)",
  "(unknown)",
  "(unknown)",
  "Instruction Abort from a lower Exception level.",
  "Instruction Abort taken without a change in Exception level.",
  "PC alignment fault exception.",
  "(unknown)",
  "Data Abort from a lower Exception level.",
  "Data Abort without a change in Exception level, or Data Aborts taken to EL2 as a result of accesses generated associated with VNCR_EL2 as part of nested virtualization support.",
  "SP alignment fault exception.",
  "(unknown)",
  "Trapped floating-point exception taken from AArch32 state.",
  "(unknown)",
  "(unknown)",
  "(unknown)",
  "Trapped floating-point exception taken from AArch64 state.",
  "(unknown)",
  "(unknown)",
  "SError interrupt.",
  "Breakpoint exception from a lower Exception level.",
  "Breakpoint exception taken without a change in Exception level.",
  "Software Step exception from a lower Exception level.",
  "Software Step exception taken without a change in Exception level.",
  "Watchpoint from a lower Exception level.",
  "Watchpoint exceptions without a change in Exception level, or Watchpoint exceptions taken to EL2 as a result of accesses generated associated with VNCR_EL2 as part of nested virtualization support.",
  "(unknown)",
  "(unknown)",
  "BKPT instruction execution in AArch32 state.",
  "(unknown)",
  "Vector Catch exception from AArch32 state.",
  "(unknown)",
  "BRK instruction execution in AArch64 state.",
};

#define ESR_EL2_EC_SHIFT     26

#define ESR_EL2_EC_TRAP_WFX       1
#define ESR_EL2_EC_TRAP_FP_REG    7
#define ESR_EL2_EC_HVC64          22
#define ESR_EL2_EC_TRAP_SYSTEM    24
#define ESR_EL2_EC_TRAP_SVE       25
#define ESR_EL2_EC_DABT_LOW       36

void handle_trap_wfx() {
  schedule();
}

void handle_hvc64(unsigned long hvc_nr) {
  WARN("HVC #%d", hvc_nr);
}



void handle_trap_system(unsigned long esr) {
#define DEFINE_SYSREG_MSR(name, _op1, _crn, _crm, _op2) do { \
  if (op1 == (_op1) && crn == (_crn) && crm == (_crm) && op2 == (_op2)) { \
    current->cpu_sysregs.name = regs->regs[rt]; \
    goto sys_fin; \
  } \
} while(0)

#define DEFINE_SYSREG_MRS(name, _op1, _crn, _crm, _op2) do { \
  if (op1 == (_op1) && crn == (_crn) && crm == (_crm) && op2 == (_op2)) { \
    regs->regs[rt] = current->cpu_sysregs.name; \
    goto sys_fin; \
  } \
} while(0)

  struct pt_regs *regs = task_pt_regs(current);

  unsigned int op0 = (esr >> 20) & 0x3;
  unsigned int op2 = (esr >> 17) & 0x7;
  unsigned int op1 = (esr >> 14) & 0x7;
  unsigned int crn = (esr >> 10) & 0xf;
  unsigned int rt  = (esr >> 5) & 0x1f;
  unsigned int crm = (esr >> 1) & 0xf;
  unsigned int dir = esr & 0x1;

  //INFO("trap_system: op0=%d,op2=%d,op1=%d,crn=%d,rt=%d,crm=%d,dir=%d",
  //    op0,op2,op1,crn,rt,crm,dir);

  if ((op0 & 2) && dir == 0) {
    // msr(reg)
    DEFINE_SYSREG_MSR(actlr_el1, 0, 1, 0, 1);
    DEFINE_SYSREG_MSR(csselr_el1, 1, 0, 0, 0);
  } else if ((op0 & 2) && dir == 1) {
    // mrs
    DEFINE_SYSREG_MRS(actlr_el1, 0, 1, 0, 1);
    DEFINE_SYSREG_MRS(id_pfr0_el1, 0, 0, 1, 0);
    DEFINE_SYSREG_MRS(id_pfr1_el1, 0, 0, 1, 1);
    DEFINE_SYSREG_MRS(id_mmfr0_el1, 0, 0, 1, 4);
    DEFINE_SYSREG_MRS(id_mmfr1_el1, 0, 0, 1, 5);
    DEFINE_SYSREG_MRS(id_mmfr2_el1, 0, 0, 1, 6);
    DEFINE_SYSREG_MRS(id_mmfr3_el1, 0, 0, 1, 7);
    DEFINE_SYSREG_MRS(id_isar0_el1, 0, 0, 2, 0);
    DEFINE_SYSREG_MRS(id_isar1_el1, 0, 0, 2, 1);
    DEFINE_SYSREG_MRS(id_isar2_el1, 0, 0, 2, 2);
    DEFINE_SYSREG_MRS(id_isar3_el1, 0, 0, 2, 3);
    DEFINE_SYSREG_MRS(id_isar4_el1, 0, 0, 2, 4);
    DEFINE_SYSREG_MRS(id_isar5_el1, 0, 0, 2, 5);
    DEFINE_SYSREG_MRS(mvfr0_el1, 0, 0, 3, 0);
    DEFINE_SYSREG_MRS(mvfr1_el1, 0, 0, 3, 1);
    DEFINE_SYSREG_MRS(mvfr2_el1, 0, 0, 3, 2);
    DEFINE_SYSREG_MRS(id_aa64pfr0_el1, 0, 0, 4, 0);
    DEFINE_SYSREG_MRS(id_aa64pfr1_el1, 0, 0, 4, 1);
    DEFINE_SYSREG_MRS(id_aa64dfr0_el1, 0, 0, 5, 0);
    DEFINE_SYSREG_MRS(id_aa64dfr1_el1, 0, 0, 5, 1);
    DEFINE_SYSREG_MRS(id_aa64isar0_el1, 0, 0, 6, 0);
    DEFINE_SYSREG_MRS(id_aa64isar1_el1, 0, 0, 6, 1);
    DEFINE_SYSREG_MRS(id_aa64mmfr0_el1, 0, 0, 7, 0);
    DEFINE_SYSREG_MRS(id_aa64mmfr1_el1, 0, 0, 7, 1);
    DEFINE_SYSREG_MRS(id_aa64afr0_el1, 0, 0, 5, 4);
    DEFINE_SYSREG_MRS(id_aa64afr1_el1, 0, 0, 5, 5);
    DEFINE_SYSREG_MRS(ctr_el0, 3, 0, 0, 1);
    DEFINE_SYSREG_MRS(ccsidr_el1, 1, 0, 0, 0);
    DEFINE_SYSREG_MRS(clidr_el1, 1, 0, 0, 1);
    DEFINE_SYSREG_MRS(csselr_el1, 2, 0, 0, 0);
    DEFINE_SYSREG_MRS(aidr_el1, 1, 0, 0, 7);
    DEFINE_SYSREG_MRS(revidr_el1, 0, 0, 0, 6);
  }
sys_fin:
  return;
}


void handle_sync_exception(unsigned long esr, unsigned long elr,
    unsigned long far, unsigned long hvc_nr) {
  struct pt_regs *regs = task_pt_regs(current);
  int eclass = (esr >> ESR_EL2_EC_SHIFT) & 0x3f;
  int ilen = ((esr >> 25) & 1) ? 4 : 2;

  switch (eclass) {
  case ESR_EL2_EC_TRAP_WFX:
    handle_trap_wfx();
    regs->pc += ilen;
    break;
  case ESR_EL2_EC_TRAP_FP_REG:
    WARN("TRAP_FP_REG is not implemented.");
    break;
  case ESR_EL2_EC_HVC64:
    handle_hvc64(hvc_nr);
    break;
  case ESR_EL2_EC_TRAP_SYSTEM:
    handle_trap_system(esr);
    regs->pc += ilen;
    break;
  case ESR_EL2_EC_TRAP_SVE:
    WARN("TRAP_SVE is not implemented.");
    break;
  case ESR_EL2_EC_DABT_LOW:
    if (handle_mem_abort(far, esr) < 0)
      PANIC("handle_mem_abort() failed.");
    break;
  default:
    PANIC("uncaught synchronous exception:\n%s\nesr: %x, address: %x",
        sync_error_reasons[eclass], esr, elr);
    break;
  }
}
