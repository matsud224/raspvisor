#include <inttypes.h>
#include "board.h"
#include "debug.h"
#include "bcm2837.h"
#include "mm.h"
#include "peripherals/mini_uart.h"
#include "peripherals/timer.h"

struct bcm2837_state {
  struct aux_peripherals_regs {
    uint8_t  aux_irq;
    uint8_t  aux_enables;
    uint8_t  aux_mu_io;
    uint8_t  aux_mu_ier;
    uint8_t  aux_mu_iir;
    uint8_t  aux_mu_lcr;
    uint8_t  aux_mu_mcr;
    uint8_t  aux_mu_lsr;
    uint8_t  aux_mu_msr;
    uint8_t  aux_mu_scratch;
    uint8_t  aux_mu_cntl;
    uint32_t aux_mu_stat;
    uint16_t aux_mu_baud;
  } aux;

  struct systimer_regs {
    uint32_t cs;
    uint32_t clo;
    uint32_t chi;
    uint32_t c0;
    uint32_t c1;
    uint32_t c2;
    uint32_t c3;
  } systimer;
};

const struct bcm2837_state initial_state = {
  .aux = {
    .aux_irq        = 0x0,
    .aux_enables    = 0x0,
    .aux_mu_io      = 0x0,
    .aux_mu_ier     = 0x0,
    .aux_mu_iir     = 0xc1,
    .aux_mu_lcr     = 0x0,
    .aux_mu_mcr     = 0x0,
    .aux_mu_lsr     = 0x40,
    .aux_mu_msr     = 0x20,
    .aux_mu_scratch = 0x0,
    .aux_mu_cntl    = 0x3,
    .aux_mu_stat    = 0x30c,
    .aux_mu_baud    = 0x0,
  },
  .systimer = {
    .cs  = 0x0,
    .clo = 0x0,
    .chi = 0x0,
    .c0  = 0x0,
    .c1  = 0x0,
    .c2  = 0x0,
    .c3  = 0x0,
  },
};

void bcm2837_initialize(struct task_struct *tsk) {
  struct bcm2837_state *state = (struct bcm2837_state *)allocate_page();
  *state = initial_state;
  tsk->board_data = state;

  unsigned long begin = DEVICE_BASE;
  unsigned long end = PHYS_MEMORY_SIZE - SECTION_SIZE;
  for (; begin < end; begin += PAGE_SIZE) {
    set_task_page_notaccessable(tsk, begin);
  }
}

unsigned long handle_mini_uart_read(unsigned long addr, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case AUX_IRQ:
    return state->aux.aux_irq;
  case AUX_ENABLES:
    return state->aux.aux_enables;
  case AUX_MU_IO_REG:
    return state->aux.aux_mu_io;
  case AUX_MU_IER_REG:
    return state->aux.aux_mu_ier;
  case AUX_MU_IIR_REG:
    return state->aux.aux_mu_iir;
  case AUX_MU_LCR_REG:
    return state->aux.aux_mu_lcr;
  case AUX_MU_MCR_REG:
    return state->aux.aux_mu_mcr;
  case AUX_MU_LSR_REG:
    return state->aux.aux_mu_lsr;
  case AUX_MU_MSR_REG:
    return state->aux.aux_mu_msr;
  case AUX_MU_SCRATCH:
    return state->aux.aux_mu_scratch;
  case AUX_MU_CNTL_REG:
    return state->aux.aux_mu_cntl;
  case AUX_MU_STAT_REG:
    return state->aux.aux_mu_stat;
  case AUX_MU_BAUD_REG:
    return state->aux.aux_mu_baud;
  }
  return 0;
}

void handle_mini_uart_write(unsigned long addr, unsigned long val, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case AUX_ENABLES:
    state->aux.aux_enables = val;
    break;
  case AUX_MU_IO_REG:
    state->aux.aux_mu_io = val;
    break;
  case AUX_MU_IER_REG:
    state->aux.aux_mu_ier = val;
    break;
  case AUX_MU_IIR_REG:
    state->aux.aux_mu_iir = val;
    break;
  case AUX_MU_LCR_REG:
    state->aux.aux_mu_lcr = val;
    break;
  case AUX_MU_MCR_REG:
    state->aux.aux_mu_mcr = val;
    break;
  case AUX_MU_SCRATCH:
    state->aux.aux_mu_scratch = val;
    break;
  case AUX_MU_CNTL_REG:
    state->aux.aux_mu_cntl = val;
    break;
  case AUX_MU_BAUD_REG:
    state->aux.aux_mu_baud = val;
    break;
  }
}

unsigned long handle_systimer_read(unsigned long addr, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case TIMER_CS:
    return state->systimer.cs;
  case TIMER_CLO:
    return state->systimer.clo;
  case TIMER_CHI:
    return state->systimer.chi;
  case TIMER_C0:
    return state->systimer.c0;
  case TIMER_C1:
    return state->systimer.c1;
  case TIMER_C2:
    return state->systimer.c2;
  case TIMER_C3:
    return state->systimer.c3;
  }
  return 0;
}

void handle_systimer_write(unsigned long addr, unsigned long val, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case TIMER_CS:
    state->systimer.cs = val;
  case TIMER_CLO:
    state->systimer.clo = val;
  case TIMER_CHI:
    state->systimer.chi = val;
  case TIMER_C0:
    state->systimer.c0 = val;
  case TIMER_C1:
    state->systimer.c1 = val;
  case TIMER_C2:
    state->systimer.c2 = val;
  case TIMER_C3:
    state->systimer.c3 = val;
  }
}

unsigned long bcm2837_mmio_read(unsigned long addr, int accsz) {
  if (addr >= AUX_IRQ && addr <= AUX_MU_BAUD_REG) {
    return handle_mini_uart_read(addr, accsz);
  } else if (addr >= TIMER_CS && addr <= TIMER_C3) {
    return handle_systimer_read(addr, accsz);
  }
  return 0;
}

void bcm2837_mmio_write(unsigned long addr, unsigned long val, int accsz) {
  if (addr >= AUX_IRQ && addr <= AUX_MU_BAUD_REG) {
    handle_mini_uart_write(addr, val, accsz);
  } else if (addr >= TIMER_CS && addr <= TIMER_C3) {
    handle_systimer_write(addr, val, accsz);
  }
}

const struct board_ops bcm2837_board_ops = {
  .initialize = bcm2837_initialize,
  .mmio_read  = bcm2837_mmio_read,
  .mmio_write = bcm2837_mmio_write,
};
