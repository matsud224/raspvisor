#include "arm/sysregs.h"
#include "entry.h"

  .macro handle_invalid_entry type
  kernel_entry
  mov x0, #\type
  mrs x1, esr_el2
  mrs x2, elr_el2
  mrs x3, far_el2
  bl  show_invalid_entry_message
  b err_hang
  .endm

  .macro  ventry  label
  .align  7
  b \label
  .endm

  .macro  kernel_entry
  sub sp, sp, #S_FRAME_SIZE
  stp x0, x1, [sp, #16 * 0]
  stp x2, x3, [sp, #16 * 1]
  stp x4, x5, [sp, #16 * 2]
  stp x6, x7, [sp, #16 * 3]
  stp x8, x9, [sp, #16 * 4]
  stp x10, x11, [sp, #16 * 5]
  stp x12, x13, [sp, #16 * 6]
  stp x14, x15, [sp, #16 * 7]
  stp x16, x17, [sp, #16 * 8]
  stp x18, x19, [sp, #16 * 9]
  stp x20, x21, [sp, #16 * 10]
  stp x22, x23, [sp, #16 * 11]
  stp x24, x25, [sp, #16 * 12]
  stp x26, x27, [sp, #16 * 13]
  stp x28, x29, [sp, #16 * 14]

  add x21, sp, #S_FRAME_SIZE

  mrs x22, elr_el2
  mrs x23, spsr_el2

  stp x30, x21, [sp, #16 * 15]
  stp x22, x23, [sp, #16 * 16]

  bl vm_leaving_work
  .endm

  .macro  kernel_exit
  bl vm_entering_work

  ldp x30, x21, [sp, #16 * 15]
  ldp x22, x23, [sp, #16 * 16]

  msr elr_el2, x22
  msr spsr_el2, x23

  ldp x0, x1, [sp, #16 * 0]
  ldp x2, x3, [sp, #16 * 1]
  ldp x4, x5, [sp, #16 * 2]
  ldp x6, x7, [sp, #16 * 3]
  ldp x8, x9, [sp, #16 * 4]
  ldp x10, x11, [sp, #16 * 5]
  ldp x12, x13, [sp, #16 * 6]
  ldp x14, x15, [sp, #16 * 7]
  ldp x16, x17, [sp, #16 * 8]
  ldp x18, x19, [sp, #16 * 9]
  ldp x20, x21, [sp, #16 * 10]
  ldp x22, x23, [sp, #16 * 11]
  ldp x24, x25, [sp, #16 * 12]
  ldp x26, x27, [sp, #16 * 13]
  ldp x28, x29, [sp, #16 * 14]
  add sp, sp, #S_FRAME_SIZE
  eret
  .endm


/*
 * Exception vectors.
 */
.align  11
.globl vectors
vectors:
  ventry  sync_invalid_el2         // Synchronous EL2
  ventry  irq_invalid_el2          // IRQ EL2
  ventry  fiq_invalid_el2          // FIQ EL2
  ventry  error_invalid_el2        // Error EL2

  ventry  sync_invalid_el2         // Synchronous EL2
  ventry  el2_irq                  // IRQ EL2
  ventry  fiq_invalid_el2          // FIQ EL2
  ventry  error_invalid_el2        // Error EL2

  ventry  el01_sync                // Synchronous 64-bit EL0 or 1
  ventry  el01_irq                 // IRQ 64-bit EL0 or 1
  ventry  fiq_invalid_el01_64      // FIQ 64-bit EL0 or 1
  ventry  error_invalid_el01_64    // Error 64-bit EL0 or 1

  ventry  sync_invalid_el01_32     // Synchronous 32-bit EL0 or 1
  ventry  irq_invalid_el01_32      // IRQ 32-bit EL0 or 1
  ventry  fiq_invalid_el01_32      // FIQ 32-bit EL0 or 1
  ventry  error_invalid_el01_32    // Error 32-bit EL0 or 1

sync_invalid_el2:
  handle_invalid_entry SYNC_INVALID_EL2

irq_invalid_el2:
  handle_invalid_entry IRQ_INVALID_EL2

fiq_invalid_el2:
  handle_invalid_entry FIQ_INVALID_EL2

error_invalid_el2:
  handle_invalid_entry ERROR_INVALID_EL2

fiq_invalid_el01_64:
  handle_invalid_entry FIQ_INVALID_EL01_64

error_invalid_el01_64:
  handle_invalid_entry ERROR_INVALID_EL01_64

sync_invalid_el01_32:
  handle_invalid_entry SYNC_INVALID_EL01_32

irq_invalid_el01_32:
  handle_invalid_entry IRQ_INVALID_EL01_32

fiq_invalid_el01_32:
  handle_invalid_entry FIQ_INVALID_EL01_32

error_invalid_el01_32:
  handle_invalid_entry ERROR_INVALID_EL01_32


el2_irq:
  kernel_entry
  bl  handle_irq
  kernel_exit

el01_irq:
  kernel_entry
  bl  handle_irq
  kernel_exit

el01_sync:
  kernel_entry
  mrs x0, esr_el2
  mrs x1, elr_el2
  mrs x2, far_el2
  mov x3, x8 // hvc number in x8
  bl handle_sync_exception
  kernel_exit

.globl switch_from_kthread
switch_from_kthread:
  bl  schedule_tail
  mov x0, x20
  mov x1, x21
  mov x3, x22
  mov x4, x23
  blr x19
  bl disable_irq
  kernel_exit

.globl err_hang
err_hang: b err_hang
