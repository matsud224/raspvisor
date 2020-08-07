.globl get_el
get_el:
  mrs x0, CurrentEL
  lsr x0, x0, #2
  ret

.globl put32
put32:
  str w1,[x0]
  ret

.globl get32
get32:
  ldr w0,[x0]
  ret

.globl delay
delay:
  subs x0, x0, #1
  bne delay
  ret

.globl set_stage2_pgd
set_stage2_pgd:
  // prepare VMID
  and x1, x1, #0xff
  lsl x1, x1, #48
  // set VMID
  orr x0, x0, x1
  // TLB invalidate
  dsb ishst
  tlbi vmalls12e1is
  dsb ish              // ensure completion of TLB invalidation
  isb
  // set address of PGD
  msr vttbr_el2, x0
  ret

.globl _set_sysregs
_set_sysregs:
  ldp x1, x2, [x0], #16
  msr sctlr_el1, x1
  msr spsr_el1, x2
  ldp x1, x2, [x0], #16
  msr ttbr0_el1, x1
  msr ttbr1_el1, x2
  ldp x1, x2, [x0], #16
  msr tcr_el1, x1
  msr mair_el1, x2
  isb
  ret

.globl _get_sysregs
_get_sysregs:
  mrs x1, sctlr_el1
  mrs x2, spsr_el1
  stp x1, x2, [x0], #16
  mrs x1, ttbr0_el1
  mrs x2, ttbr1_el1
  stp x1, x2, [x0], #16
  mrs x1, tcr_el1
  mrs x2, mair_el1
  stp x1, x2, [x0], #16
  ret
