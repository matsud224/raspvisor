#include "peripherals/irq.h"
#include "arm/sysregs.h"
#include "entry.h"
#include "printf.h"
#include "timer.h"
#include "utils.h"
#include "sched.h"

const char *entry_error_messages[] = {
  "SYNC_INVALID_EL2",
  "IRQ_INVALID_EL2",
  "FIQ_INVALID_EL2",
  "ERROR_INVALID_EL2",

  "SYNC_INVALID_EL01_64",
  "IRQ_INVALID_EL01_64",
  "FIQ_INVALID_EL01_64",
  "ERROR_INVALID_EL01_64",

  "SYNC_INVALID_EL01_32",
  "IRQ_INVALID_EL01_32",
  "FIQ_INVALID_EL01_32",
  "ERROR_INVALID_EL01_32",

  "SYNC_ERROR",
  "HVC_ERROR",
  "DATA_ABORT_ERROR",
};

void enable_interrupt_controller() { put32(ENABLE_IRQS_1, SYSTEM_TIMER_IRQ_1); }

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

void show_invalid_entry_message(int type, unsigned long esr,
                                unsigned long address) {
  printf("%s, ESR: %x, address: %x\r\n", entry_error_messages[type], esr,
         address);
}

void show_uncaught_sync_exception_message(int type, unsigned long esr,
                                unsigned long address) {
  int eclass = (esr >> ESR_EL2_EC_SHIFT) & 0x3f;
  struct cpu_sysregs sysregs;
  printf("%s, ESR: %x, address: %x\r\nreason: %s\r\n",
      entry_error_messages[type], esr,
      address, sync_error_reasons[eclass]);
}


void handle_irq(void) {
  unsigned int irq = get32(IRQ_PENDING_1);
  switch (irq) {
  case (SYSTEM_TIMER_IRQ_1):
    handle_timer_irq();
    break;
  default:
    printf("Inknown pending irq: %x\r\n", irq);
  }
}
