#include <inttypes.h>
#include "board.h"
#include "debug.h"
#include "bcm2837.h"
#include "mm.h"
#include "fifo.h"
#include "peripherals/mini_uart.h"
#include "peripherals/timer.h"
#include "peripherals/irq.h"

struct bcm2837_state {
  struct intctrl_regs {
    uint8_t irq_basic_pending;
    uint8_t irq_pending_1;
    uint8_t irq_pending_2;
    uint8_t fiq_control;
    uint8_t enable_irqs_1;
    uint8_t enable_irqs_2;
    uint8_t enable_basic_irqs;
    uint8_t disable_irqs_1;
    uint8_t disable_irqs_2;
    uint8_t disable_basic_irqs;
  } intctrl;

  struct aux_peripherals_regs {
    struct fifo *mu_tx_fifo;
    struct fifo *mu_rx_fifo;
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
  .intctrl = {
    .irq_basic_pending  = 0x0,
    .irq_pending_1      = 0x0,
    .irq_pending_2      = 0x0,
    .fiq_control        = 0x0,
    .enable_irqs_1      = 0x0,
    .enable_irqs_2      = 0x0,
    .enable_basic_irqs  = 0x0,
    .disable_irqs_1     = 0x0,
    .disable_irqs_2     = 0x0,
    .disable_basic_irqs = 0x0,
  },
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

#define ADDR_IN_INTCTRL(a) ((a) >= IRQ_BASIC_PENDING && (a) <= DISABLE_BASIC_IRQS)
#define ADDR_IN_AUX(a) ((a) >= AUX_IRQ && (a) <= AUX_MU_BAUD_REG)
#define ADDR_IN_AUX_MU(a) ((a) >= AUX_MU_IO_REG && (a) <= AUX_MU_BAUD_REG)
#define ADDR_IN_SYSTIMER(a) ((a) >= TIMER_CS && (a) <= TIMER_C3)

void bcm2837_initialize(struct task_struct *tsk) {
  struct bcm2837_state *state = (struct bcm2837_state *)allocate_page();
  *state = initial_state;

  state->aux.mu_tx_fifo = create_fifo();
  state->aux.mu_rx_fifo = create_fifo();

  tsk->board_data = state;

  unsigned long begin = DEVICE_BASE;
  unsigned long end = PHYS_MEMORY_SIZE - SECTION_SIZE;
  for (; begin < end; begin += PAGE_SIZE) {
    set_task_page_notaccessable(tsk, begin);
  }
}

unsigned long handle_intctrl_read(unsigned long addr, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case IRQ_BASIC_PENDING:
    return state->intctrl.irq_basic_pending;
  case IRQ_PENDING_1:
    return state->intctrl.irq_pending_1;
  case IRQ_PENDING_2:
    return state->intctrl.irq_pending_2;
  case FIQ_CONTROL:
    return state->intctrl.fiq_control;
  case ENABLE_IRQS_1:
    return state->intctrl.enable_irqs_1;
  case ENABLE_IRQS_2:
    return state->intctrl.enable_irqs_2;
  case ENABLE_BASIC_IRQS:
    return state->intctrl.enable_basic_irqs;
  case DISABLE_IRQS_1:
    return state->intctrl.disable_irqs_1;
  case DISABLE_IRQS_2:
    return state->intctrl.disable_irqs_2;
  case DISABLE_BASIC_IRQS:
    return state->intctrl.disable_basic_irqs;
  }
  return 0;
}

void handle_intctrl_write(unsigned long addr, unsigned long val, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;
  switch (addr) {
  case FIQ_CONTROL:
    state->intctrl.fiq_control = val;
  case ENABLE_IRQS_1:
    state->intctrl.enable_irqs_1 = val;
  case ENABLE_IRQS_2:
    state->intctrl.enable_irqs_2 = val;
  case ENABLE_BASIC_IRQS:
    state->intctrl.enable_basic_irqs = val;
  case DISABLE_IRQS_1:
    state->intctrl.disable_irqs_1 = val;
  case DISABLE_IRQS_2:
    state->intctrl.disable_irqs_2 = val;
  case DISABLE_BASIC_IRQS:
    state->intctrl.disable_basic_irqs = val;
  }
}

unsigned long handle_aux_read(unsigned long addr, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;

  if ((state->aux.aux_enables & 1) == 0 && ADDR_IN_AUX_MU(addr)) {
    return 0;
  }

  switch (addr) {
  case AUX_IRQ:
    return state->aux.aux_irq;
  case AUX_ENABLES:
    return state->aux.aux_enables;
  case AUX_MU_IO_REG:
    if (AUX_MU_LCR_REG & 0x8) {
      return state->aux.aux_mu_io;
    } else {
      unsigned long data;
      dequeue_fifo(state->aux.mu_rx_fifo, &data);
      return data;
    }
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

void handle_aux_write(unsigned long addr, unsigned long val, int accsz) {
  struct bcm2837_state *state = (struct bcm2837_state *)current->board_data;

  if ((state->aux.aux_enables & 1) == 0 && ADDR_IN_AUX_MU(addr)) {
    return;
  }

  switch (addr) {
  case AUX_ENABLES:
    state->aux.aux_enables = val;
    break;
  case AUX_MU_IO_REG:
    if (AUX_MU_LCR_REG & 0x8)
      state->aux.aux_mu_io = val;
    else
      enqueue_fifo(state->aux.mu_tx_fifo, val & 0xff);
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
  if (ADDR_IN_INTCTRL(addr)) {
    return handle_intctrl_read(addr, accsz);
  } else if (ADDR_IN_AUX(addr)) {
    return handle_aux_read(addr, accsz);
  } else if (ADDR_IN_SYSTIMER(addr)) {
    return handle_systimer_read(addr, accsz);
  }
  return 0;
}

void bcm2837_mmio_write(unsigned long addr, unsigned long val, int accsz) {
  if (ADDR_IN_INTCTRL(addr)) {
    handle_intctrl_write(addr, val, accsz);
  } else if (ADDR_IN_AUX(addr)) {
    handle_aux_write(addr, val, accsz);
  } else if (ADDR_IN_SYSTIMER(addr)) {
    handle_systimer_write(addr, val, accsz);
  }
}

const struct board_ops bcm2837_board_ops = {
  .initialize = bcm2837_initialize,
  .mmio_read  = bcm2837_mmio_read,
  .mmio_write = bcm2837_mmio_write,
};
