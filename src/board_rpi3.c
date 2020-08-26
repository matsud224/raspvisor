#include <inttypes.h>
#include "board.h"
#include "debug.h"
#include "board_rpi3.h"
#include "mm.h"
#include "fifo.h"
#include "timer.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/pl011.h"
#include "peripherals/timer.h"
#include "peripherals/irq.h"
#include "peripherals/mbox.h"

struct rpi3_state {
  struct {
    uint8_t irq_enabled[72]; // IRQ 0-64, ARM Timer, ARM Mailbox, ...
    uint8_t fiq_control;
    uint32_t irqs_1_enabled;
    uint32_t irqs_2_enabled;
    uint8_t  basic_irqs_enabled;
  } intctrl;

  struct {
    uint8_t  aux_enables;
    uint8_t  aux_mu_io;
    uint8_t  aux_mu_ier;
    uint8_t  aux_mu_lcr;
    uint8_t  aux_mu_mcr;
    uint8_t  aux_mu_msr;
    uint8_t  aux_mu_scratch;
    uint8_t  aux_mu_cntl;
    uint16_t aux_mu_baud;
  } aux;

  struct {
    uint64_t last_physical_count;
    uint64_t offset;
    uint32_t cs;
    uint32_t c0;
    uint32_t c1;
    uint32_t c2;
    uint32_t c3;
    uint32_t c0_expire;
    uint32_t c1_expire;
    uint32_t c2_expire;
    uint32_t c3_expire;
  } systimer;

  struct {
    struct fifo *fifo;
  } mbox;

  struct {
    uint32_t ibrd;
    uint32_t fbrd;
    uint32_t lcrh;
    uint32_t cr;
    uint32_t ifls;
    uint32_t imsc;
  } pl011;
};

const struct rpi3_state initial_state = {
  .intctrl = {
    .fiq_control        = 0x0,
    .irqs_1_enabled     = 0x0,
    .irqs_2_enabled     = 0x0,
    .basic_irqs_enabled = 0x0,
  },
  .aux = {
    .aux_enables    = 0x0,
    .aux_mu_io      = 0x0,
    .aux_mu_ier     = 0x0,
    .aux_mu_lcr     = 0x0,
    .aux_mu_mcr     = 0x0,
    .aux_mu_msr     = 0x10,
    .aux_mu_scratch = 0x0,
    .aux_mu_cntl    = 0x3,
    .aux_mu_baud    = 0x0,
  },
  .systimer = {
    .cs  = 0x0,
    .c0  = 0x0,
    .c1  = 0x0,
    .c2  = 0x0,
    .c3  = 0x0,
  },
  .pl011 = {
    .ibrd = 0x0,
    .fbrd = 0x0,
    .lcrh = 0x0,
    .cr   = 0x300,
  },
};

#define ADDR_IN_INTCTRL(a) ((a) >= IRQ_BASIC_PENDING && (a) <= DISABLE_BASIC_IRQS)
#define ADDR_IN_AUX(a) ((a) >= AUX_IRQ && (a) <= AUX_MU_BAUD_REG)
#define ADDR_IN_AUX_MU(a) ((a) >= AUX_MU_IO_REG && (a) <= AUX_MU_BAUD_REG)
#define ADDR_IN_PL011(a) ((a) >= PL011_DR && (a) <= PL011_TDR)
#define ADDR_IN_SYSTIMER(a) ((a) >= TIMER_CS && (a) <= TIMER_C3)
#define ADDR_IN_MBOX(a) ((a) >= MBOX_READ && (a) <= MBOX_WRITE)

void rpi3_initialize(struct task_struct *tsk) {
  struct rpi3_state *s = (struct rpi3_state *)allocate_page();
  *s = initial_state;

  s->systimer.last_physical_count = get_physical_timer_count();
  s->mbox.fifo = create_fifo();

  tsk->board_data = s;

  unsigned long begin = DEVICE_BASE;
  unsigned long end = PHYS_MEMORY_SIZE - SECTION_SIZE;
  for (; begin < end; begin += PAGE_SIZE) {
    set_task_page_notaccessable(tsk, begin);
  }
}

unsigned long handle_aux_read(struct task_struct *, unsigned long);

unsigned long handle_intctrl_read(struct task_struct *tsk, unsigned long addr) {
#define BIT(v, n) ((v) & (1 << (n)))
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;
  switch (addr) {
  case IRQ_BASIC_PENDING:
    {
      int pending1 = handle_intctrl_read(tsk, IRQ_PENDING_1) != 0;
      int pending2 = handle_intctrl_read(tsk, IRQ_PENDING_2) != 0;
      return (pending1 << 8) | (pending2 << 9);
    }
  case IRQ_PENDING_1:
    {
      unsigned long systimer_match1 =
        BIT(s->intctrl.irqs_1_enabled, 1) && (s->systimer.cs & 0x2);
      unsigned long systimer_match3 =
        BIT(s->intctrl.irqs_1_enabled, 3) && (s->systimer.cs & 0x8);
      unsigned long aux_int =
        BIT(s->intctrl.irqs_1_enabled, 29) && handle_aux_read(tsk, AUX_IRQ);
      return (systimer_match1 << 1) | (systimer_match3 << 3) | (aux_int << 29);
    }
  case IRQ_PENDING_2:
    {
      unsigned long uart_int =
        BIT(s->intctrl.irqs_2_enabled, (57-32)) && handle_aux_read(tsk, PL011_MIS);
      return (uart_int << (57-32));
    }
  case FIQ_CONTROL:
    return s->intctrl.fiq_control;
  case ENABLE_IRQS_1:
    return s->intctrl.irqs_1_enabled;
  case ENABLE_IRQS_2:
    return s->intctrl.irqs_2_enabled;
  case ENABLE_BASIC_IRQS:
    return s->intctrl.basic_irqs_enabled;
  case DISABLE_IRQS_1:
    return ~s->intctrl.irqs_1_enabled;
  case DISABLE_IRQS_2:
    return ~s->intctrl.irqs_2_enabled;
  case DISABLE_BASIC_IRQS:
    return ~s->intctrl.basic_irqs_enabled;
  }
  return 0;
}

void handle_intctrl_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;
  switch (addr) {
  case FIQ_CONTROL:
    s->intctrl.fiq_control = val;
    break;
  case ENABLE_IRQS_1:
    s->intctrl.irqs_1_enabled |= val;
    break;
  case ENABLE_IRQS_2:
    s->intctrl.irqs_2_enabled |= val;
    break;
  case ENABLE_BASIC_IRQS:
    s->intctrl.basic_irqs_enabled |= val;
    break;
  case DISABLE_IRQS_1:
    s->intctrl.irqs_1_enabled &= ~val;
    break;
  case DISABLE_IRQS_2:
    s->intctrl.irqs_2_enabled &= ~val;
    break;
  case DISABLE_BASIC_IRQS:
    s->intctrl.basic_irqs_enabled &= ~val;
    break;
  }
}

#define LCR_DLAB 0x80

unsigned long handle_aux_read(struct task_struct *tsk, unsigned long addr) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  if ((s->aux.aux_enables & 1) == 0 && ADDR_IN_AUX_MU(addr)) {
    return 0;
  }

  switch (addr) {
  case AUX_IRQ:
    {
      int mu_pending = (s->aux.aux_enables & 0x1) &&
        ~(handle_aux_read(tsk, AUX_MU_IIR_REG) & 0x1);
      return mu_pending;
    }
  case AUX_ENABLES:
    return s->aux.aux_enables;
  case AUX_MU_IO_REG:
    if (s->aux.aux_mu_lcr & LCR_DLAB) {
      s->aux.aux_mu_lcr &= ~LCR_DLAB;
      return s->aux.aux_mu_baud & 0xff;
    } else {
      unsigned long data;
      dequeue_fifo(tsk->console.in_fifo, &data);
      return data & 0xff;
    }
  case AUX_MU_IER_REG:
    if (s->aux.aux_mu_lcr & LCR_DLAB) {
      return s->aux.aux_mu_baud >> 8;
    } else {
      return s->aux.aux_mu_ier;
    }
  case AUX_MU_IIR_REG:
    {
      int tx_int = (s->aux.aux_mu_ier & 0x2) && is_empty_fifo(tsk->console.out_fifo);
      int rx_int = (s->aux.aux_mu_ier & 0x1) && !is_empty_fifo(tsk->console.in_fifo);
      int int_id = tx_int | (rx_int << 1);
      if (int_id == 0x3)
        int_id = 0x1;
      return (!int_id) | (int_id << 1) | (0x3 << 6);
    }
  case AUX_MU_LCR_REG:
    return s->aux.aux_mu_lcr;
  case AUX_MU_MCR_REG:
    return s->aux.aux_mu_mcr;
  case AUX_MU_LSR_REG:
    {
      int dready = !is_empty_fifo(tsk->console.in_fifo);
      int tx_empty = !is_full_fifo(tsk->console.out_fifo);
      int tx_idle = is_empty_fifo(tsk->console.out_fifo);
      return dready | (tx_empty << 5) | (tx_idle << 6);
    }
  case AUX_MU_MSR_REG:
    return s->aux.aux_mu_msr;
  case AUX_MU_SCRATCH:
    return s->aux.aux_mu_scratch;
  case AUX_MU_CNTL_REG:
    return s->aux.aux_mu_cntl;
  case AUX_MU_STAT_REG:
    {
#define MIN(a,b) ((a)<(b)?(a):(b))
      int sym_avail = !is_empty_fifo(tsk->console.in_fifo);
      int space_avail = !is_full_fifo(tsk->console.out_fifo);
      int rx_idle = is_empty_fifo(tsk->console.in_fifo);
      int tx_idle = !is_empty_fifo(tsk->console.out_fifo);
      int tx_full = !space_avail;
      int tx_empty = is_empty_fifo(tsk->console.out_fifo);
      int tx_done = rx_idle & tx_empty;
      int rx_fifo_level = MIN(used_of_fifo(tsk->console.in_fifo), 8);
      int tx_fifo_level = MIN(used_of_fifo(tsk->console.out_fifo), 8);
      return sym_avail | (space_avail << 1) | (rx_idle << 2) |
        (tx_idle << 3) | (tx_full << 5) | (tx_empty << 8) |
        (tx_done << 9) | (rx_fifo_level << 16) |
        (tx_fifo_level << 24);
    }
  case AUX_MU_BAUD_REG:
    return s->aux.aux_mu_baud;
  }
  return 0;
}

void handle_aux_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  if ((s->aux.aux_enables & 1) == 0 && ADDR_IN_AUX_MU(addr)) {
    return;
  }

  switch (addr) {
  case AUX_ENABLES:
    s->aux.aux_enables = val;
    break;
  case AUX_MU_IO_REG:
    if (s->aux.aux_mu_lcr & LCR_DLAB) {
      s->aux.aux_mu_lcr &= ~LCR_DLAB;
      s->aux.aux_mu_baud =
        (s->aux.aux_mu_baud & 0xff00) | (val & 0xff);
    } else {
      enqueue_fifo(tsk->console.out_fifo, val & 0xff);
    }
    break;
  case AUX_MU_IER_REG:
    if (s->aux.aux_mu_lcr & LCR_DLAB) {
      s->aux.aux_mu_baud =
        (s->aux.aux_mu_baud & 0xff) | ((val & 0xff) << 8);
    } else {
      s->aux.aux_mu_ier = val;
    }
    break;
  case AUX_MU_IIR_REG:
    if (val & 0x2)
      clear_fifo(tsk->console.in_fifo);
    if (val & 0x4)
      clear_fifo(tsk->console.out_fifo);
    break;
  case AUX_MU_LCR_REG:
    s->aux.aux_mu_lcr = val;
    break;
  case AUX_MU_MCR_REG:
    s->aux.aux_mu_mcr = val;
    break;
  case AUX_MU_SCRATCH:
    s->aux.aux_mu_scratch = val;
    break;
  case AUX_MU_CNTL_REG:
    s->aux.aux_mu_cntl = val;
    break;
  case AUX_MU_BAUD_REG:
    s->aux.aux_mu_baud = val;
    break;
  }
}

unsigned long handle_pl011_read(struct task_struct *tsk, unsigned long addr) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  switch (addr) {
  case PL011_DR:
    {
      if ((handle_pl011_read(tsk, PL011_CR) & 0x1) &&
          (handle_pl011_read(tsk, PL011_CR) & 0x200)) {
        unsigned long data;
        dequeue_fifo(tsk->console.in_fifo, &data);
        return data & 0xff;
      } else {
        return 0;
      }
    }
  case PL011_FR:
    {
      int busy = !is_empty_fifo(tsk->console.out_fifo);
      int rxfe = is_empty_fifo(tsk->console.in_fifo);
      int txff = is_full_fifo(tsk->console.out_fifo);
      int rxff = is_full_fifo(tsk->console.in_fifo);
      int txfe = is_empty_fifo(tsk->console.out_fifo);
      return ((busy << 3) | (rxfe << 4) | (txff << 5) |
              (rxff << 6) | (txfe << 7));
    }
  case PL011_IBRD:
    return s->pl011.ibrd;
  case PL011_FBRD:
    return s->pl011.fbrd;
  case PL011_LCRH:
    return s->pl011.lcrh;
  case PL011_CR:
    return s->pl011.cr;
  case PL011_IFLS:
    return s->pl011.ifls;
  case PL011_IMSC:
    return s->pl011.imsc;
  case PL011_RIS:
    {
      int uart_en = handle_pl011_read(tsk, PL011_CR) & 0x1;
      int tx_en = handle_pl011_read(tsk, PL011_CR) & 0x100;
      int rx_en = handle_pl011_read(tsk, PL011_CR) & 0x200;
      int tx_int = uart_en && tx_en && is_empty_fifo(tsk->console.out_fifo);
      int rx_int = uart_en && rx_en && !is_empty_fifo(tsk->console.in_fifo);
      return (rx_int << 4) | (tx_int << 5);
    }
  case PL011_MIS:
    return handle_pl011_read(tsk, PL011_RIS) & ~s->pl011.imsc;
  }
  return 0;
}

void handle_pl011_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  switch (addr) {
  case PL011_DR:
    if ((handle_pl011_read(tsk, PL011_CR) & 0x1) &&
        (handle_pl011_read(tsk, PL011_CR) & 0x100))
      enqueue_fifo(tsk->console.out_fifo, val & 0xff);
    break;
  case PL011_IBRD:
    s->pl011.ibrd = val;
    break;
  case PL011_FBRD:
    s->pl011.fbrd = val;
    break;
  case PL011_LCRH:
    s->pl011.lcrh = val;
    break;
  case PL011_CR:
    s->pl011.cr = val;
    break;
  case PL011_IFLS:
    s->pl011.ifls = val;
    break;
  case PL011_IMSC:
    s->pl011.imsc = val;
    break;
  case PL011_ICR:
    if (val & 0x10)
      clear_fifo(tsk->console.in_fifo);
    if (val & 0x20)
      clear_fifo(tsk->console.out_fifo);
    break;
  }
}

#define TO_VIRTUAL_COUNT(s, p) (p - (s)->systimer.offset)
#define TO_PHYSICAL_COUNT(s, v) (v + (s)->systimer.offset)

unsigned long handle_systimer_read(struct task_struct *tsk, unsigned long addr) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;
  switch (addr) {
  case TIMER_CS:
    return s->systimer.cs;
  case TIMER_CLO:
    return TO_VIRTUAL_COUNT(s, get_physical_timer_count()) & 0xffffffff;
  case TIMER_CHI:
    return TO_VIRTUAL_COUNT(s, get_physical_timer_count()) >> 32;
  case TIMER_C0:
    return s->systimer.c0;
  case TIMER_C1:
    return s->systimer.c1;
  case TIMER_C2:
    return s->systimer.c2;
  case TIMER_C3:
    return s->systimer.c3;
  }
  return 0;
}

void handle_systimer_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;
  uint32_t current_clo = handle_systimer_read(tsk, TIMER_CLO);
  const uint32_t min_expire = 10000; // if this value is too short, CLO exceeds this value (timing problem)

  switch (addr) {
  case TIMER_CS:
    s->systimer.cs &= ~val;
    break;
  case TIMER_C0:
    s->systimer.c0 = val;
    s->systimer.c0_expire = MAX(val > current_clo ? val - current_clo : 1, min_expire);
    break;
  case TIMER_C1:
    s->systimer.c1 = val;
    s->systimer.c1_expire = MAX(val > current_clo ? val - current_clo : 1, min_expire);
    break;
  case TIMER_C2:
    s->systimer.c2 = val;
    s->systimer.c2_expire = MAX(val > current_clo ? val - current_clo : 1, min_expire);
    break;
  case TIMER_C3:
    s->systimer.c3 = val;
    s->systimer.c3_expire = MAX(val > current_clo ? val - current_clo : 1, min_expire);
    break;
  }
}

unsigned long handle_mbox_read(struct task_struct *tsk, unsigned long addr) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  switch (addr) {
  case MBOX_READ:
    {
      unsigned long val = 0;
      dequeue_fifo(s->mbox.fifo, &val);
      return val;
    }
  case MBOX_STATUS:
    return (is_empty_fifo(s->mbox.fifo) << MBOX_EMPTY_BIT) |
           (is_full_fifo(s->mbox.fifo) << MBOX_FULL_BIT);
  }
  return 0;
}

void process_mbox_message(int channel, unsigned long msg) {
  struct mbox_message_header *msghdr = (struct mbox_message_header *)(translate_stage12(msg));
  // TODO
  msghdr->code = MBOX_RESPONSE_OK;
}

void handle_mbox_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  switch (addr) {
  case MBOX_WRITE:
    process_mbox_message(val & 0xf, val & ~0xf);
    enqueue_fifo(s->mbox.fifo, val);
    break;
  }
}

unsigned long rpi3_mmio_read(struct task_struct *tsk, unsigned long addr) {
  if (ADDR_IN_INTCTRL(addr)) {
    return handle_intctrl_read(tsk, addr);
  } else if (ADDR_IN_AUX(addr)) {
    return handle_aux_read(tsk, addr);
  } else if (ADDR_IN_PL011(addr)) {
    return handle_pl011_read(tsk, addr);
  } else if (ADDR_IN_SYSTIMER(addr)) {
    return handle_systimer_read(tsk, addr);
  } else if (ADDR_IN_MBOX(addr)) {
    return handle_mbox_read(tsk, addr);
  }
  return 0;
}

void rpi3_mmio_write(struct task_struct *tsk, unsigned long addr, unsigned long val) {
  if (ADDR_IN_INTCTRL(addr)) {
    handle_intctrl_write(tsk, addr, val);
  } else if (ADDR_IN_AUX(addr)) {
    handle_aux_write(tsk, addr, val);
  } else if (ADDR_IN_PL011(addr)) {
    handle_pl011_write(tsk, addr, val);
  } else if (ADDR_IN_SYSTIMER(addr)) {
    handle_systimer_write(tsk, addr, val);
  } else if (ADDR_IN_MBOX(addr)) {
    handle_mbox_write(tsk, addr, val);
  }
}

static int check_expiration(uint32_t *expire, uint64_t lapse) {
  if (*expire == 0)
    return 0;

  if (lapse >= *expire) {
    *expire = 0;
    return 1;
  } else {
    *expire -= lapse;
    return 0;
  }
}

void rpi3_entering_vm(struct task_struct *tsk) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  // update systimer's offset
  unsigned long current_physical_count = get_physical_timer_count();
  uint64_t lapse = current_physical_count - s->systimer.last_physical_count;
  s->systimer.offset += lapse;

  // update cs register
  int matched =
    (check_expiration(&s->systimer.c0_expire, lapse)) |
    (check_expiration(&s->systimer.c1_expire, lapse) << 1) |
    (check_expiration(&s->systimer.c2_expire, lapse) << 2) |
    (check_expiration(&s->systimer.c3_expire, lapse) << 3);

  // update (physical) timer compare value for upcoming timer match
  uint32_t upcoming = 0xffffffff;
  if (s->systimer.c0_expire && upcoming > s->systimer.c0_expire)
    upcoming = s->systimer.c0_expire;
  if (s->systimer.c1_expire && upcoming > s->systimer.c1_expire)
    upcoming = s->systimer.c1_expire;
  if (s->systimer.c2_expire && upcoming > s->systimer.c2_expire)
    upcoming = s->systimer.c2_expire;
  if (s->systimer.c3_expire && upcoming > s->systimer.c3_expire)
    upcoming = s->systimer.c3_expire;

  if (upcoming != 0xffffffff)
    put32(TIMER_C3, get32(TIMER_CLO) + upcoming);

  int fired = (~s->systimer.cs) & matched;
  s->systimer.cs |= fired;
}

void rpi3_leaving_vm(struct task_struct *tsk) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;
  s->systimer.last_physical_count = get_physical_timer_count();
}

int rpi3_is_irq_asserted(struct task_struct *tsk) {
  return handle_intctrl_read(tsk, IRQ_BASIC_PENDING) != 0;
}

int rpi3_is_fiq_asserted(struct task_struct *tsk) {
  struct rpi3_state *s = (struct rpi3_state *)tsk->board_data;

  if ((s->intctrl.fiq_control & 0x80) == 0)
    return 0;

  int source = s->intctrl.fiq_control & 0x7f;
  if (source >= 0 && source <= 31) {
    int pending = handle_intctrl_read(tsk, IRQ_PENDING_1);
    return (pending & (1 << source)) != 0;
  } else if (source >= 32 && source <= 63) {
    int pending = handle_intctrl_read(tsk, IRQ_PENDING_2);
    return (pending & (1 << (source - 32))) != 0;
  } else if (source >= 64 && source <= 71) {
    int pending = handle_intctrl_read(tsk, IRQ_BASIC_PENDING);
    return (pending & (1 << (source - 64))) != 0;
  }

  return 0;
}

void rpi3_debug(struct task_struct *tsk) {
}

const struct board_ops rpi3_board_ops = {
  .initialize = rpi3_initialize,
  .mmio_read  = rpi3_mmio_read,
  .mmio_write = rpi3_mmio_write,
  .entering_vm = rpi3_entering_vm,
  .leaving_vm = rpi3_leaving_vm,
  .is_irq_asserted = rpi3_is_irq_asserted,
  .is_fiq_asserted = rpi3_is_fiq_asserted,
  .debug = rpi3_debug,
};
