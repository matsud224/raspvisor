#include "arm/mmu.h"
#include "arm/sysregs.h"
#include "mm.h"
#include "peripherals/base.h"
#include "peripherals/irq.h"

.section ".text.boot"

.globl _start
_start:
  mrs x0, mpidr_el1
  and x0, x0,#0xFF    // Check processor id
  cbz x0, master    // Hang for all non-primary CPU
  b proc_hang

proc_hang:
  b proc_hang

master:
  // Init generic timer
  ldr x0, =GENERIC_TIMER_FREQ
  msr cntfrq_el0, x0
  ldr x0, =CNTHCTL_VALUE
  msr cnthctl_el2, x0

  // Initial EL is 3
  // Change EL from 3 to 2
  ldr x0, =SCTLR_VALUE_MMU_DISABLED
  msr sctlr_el2, x0

  ldr x0, =HCR_VALUE
  msr hcr_el2, x0

  ldr x0, =SCR_VALUE
  msr scr_el3, x0

  ldr x0, =SPSR_VALUE
  msr spsr_el3, x0

  adr x0, el2_entry
  msr elr_el3, x0

  eret

el2_entry:
  adr x0, bss_begin
  adr x1, bss_end
  sub x1, x1, x0
  bl  memzero

  bl  __create_page_tables

  mov x0, #VA_START
  add sp, x0, #LOW_MEMORY

  adrp  x0, pg_dir
  msr ttbr0_el2, x0

  ldr x0, =(TCR_VALUE)
  msr tcr_el2, x0

  ldr x0, =(VTCR_VALUE)
  msr vtcr_el2, x0

  ldr x0, =(MAIR_VALUE)
  msr mair_el2, x0

  // clear TLB
  tlbi alle1

  ldr x2, =hypervisor_main

  mov x0, #SCTLR_MMU_ENABLED
  dsb ish
  isb
  msr sctlr_el2, x0
  isb

  br  x2

  .macro  create_table_entry, tbl, virt, shift, tmp1, tmp2
  lsr \tmp1, \virt, #\shift
  and \tmp1, \tmp1, #PTRS_PER_TABLE - 1     // table index
  add \tmp2, \tbl, #PAGE_SIZE
  orr \tmp2, \tmp2, #MM_TYPE_PAGE_TABLE
  str \tmp2, [\tbl, \tmp1, lsl #3]
  .endm

  .macro  create_block_map, tbl, phys, start, end, flags, tmp1
  lsr \start, \start, #SECTION_SHIFT
  and \start, \start, #PTRS_PER_TABLE - 1     // table index
  lsr \end, \end, #SECTION_SHIFT
  and \end, \end, #PTRS_PER_TABLE - 1       // table end index
  lsr \phys, \phys, #SECTION_SHIFT
  mov \tmp1, #\flags
  orr \phys, \tmp1, \phys, lsl #SECTION_SHIFT     // table entry
9999: str \phys, [\tbl, \start, lsl #3]       // store the entry
  add \start, \start, #1          // next entry
  add \phys, \phys, #SECTION_SIZE       // next block
  cmp \start, \end
  b.ls  9999b
  .endm

__create_page_tables:
  mov x29, x30            // save return address

  // 4KB granule
  adrp  x9, pg_dir       // level-0 table
  add   x10, x9, 0x1000  // level-1 table
  add   x11, x9, 0x2000  // level-2 table (0x00000000 - 0x40000000)
  add   x12, x9, 0x3000  // level-2 table (0x40000000 - 0x80000000)

  mov x0, x9
  mov x1, #PG_DIR_SIZE
  bl  memzero

  mov x0, x10
  mov x1, #PG_DIR_SIZE
  bl  memzero

  mov x0, x11
  mov x1, #PG_DIR_SIZE
  bl  memzero

  mov x0, x12
  mov x1, #PG_DIR_SIZE
  bl  memzero

  mov x1, #VA_START
  create_table_entry x9, x1, PGD_SHIFT, x2, x3

  mov x1, #VA_START
  create_table_entry x10, x1, PUD_SHIFT, x2, x3

  ldr x1, =(VA_START + 0x40000000)
  create_table_entry x10, x1, PUD_SHIFT, x2, x3

  /* Mapping kernel and init stack*/
  mov x1, xzr // start mapping from physical offset 0
  mov x2, #VA_START // first virtual address
  ldr x3, =(VA_START + DEVICE_BASE - SECTION_SIZE) // last virtual address
  create_block_map x11, x1, x2, x3, MMU_FLAGS, x4

  /* Mapping device memory*/
  mov x1, #DEVICE_BASE
  ldr x2, =(VA_START + DEVICE_BASE)
  ldr x3, =(VA_START + PHYS_MEMORY_SIZE - SECTION_SIZE)
  create_block_map x11, x1, x2, x3, MMU_DEVICE_FLAGS, x4

  /* Mapping device memory (over 1GB)*/
  mov x1, #CORE_IRQREG_START
  ldr x2, =(VA_START + CORE_IRQREG_START)
  ldr x3, =(VA_START + CORE_IRQREG_START)
  create_block_map x12, x1, x2, x3, MMU_DEVICE_FLAGS, x4

  mov x30, x29            // restore return address
  ret

