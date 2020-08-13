#include "utils.h"
#include "debug.h"
#include "mm.h"
#include "board.h"
#include "task.h"
#include "arm/mmu.h"

static unsigned short mem_map[PAGING_PAGES] = { 0 };

unsigned long allocate_page() {
  unsigned long page = get_free_page();
  if (page == 0) {
    return 0;
  }
  return page + VA_START;
}

unsigned long allocate_task_page(struct task_struct *task, unsigned long va) {
  unsigned long page = get_free_page();
  if (page == 0) {
    return 0;
  }
  map_stage2_page(task, va, page, MMU_STAGE2_PAGE_FLAGS);
  return page + VA_START;
}

void set_task_page_notaccessable(struct task_struct *task, unsigned long va) {
  map_stage2_page(task, va, 0, MMU_STAGE2_MMIO_PAGE_FLAGS);
}

unsigned long get_free_page() {
  for (int i = 0; i < PAGING_PAGES; i++) {
    if (mem_map[i] == 0) {
      mem_map[i] = 1;
      unsigned long page = LOW_MEMORY + i * PAGE_SIZE;
      memzero(page + VA_START, PAGE_SIZE);
      return page;
    }
  }
  return 0;
}

void free_page(unsigned long p) {
  mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}

void map_stage2_table_entry(unsigned long *pte, unsigned long va,
                            unsigned long pa, unsigned long flags) {
  unsigned long index = va >> PAGE_SHIFT;
  index = index & (PTRS_PER_TABLE - 1);
  unsigned long entry = pa | flags;
  pte[index] = entry;
}

unsigned long map_stage2_table(unsigned long *table, unsigned long shift,
                               unsigned long va, int *new_table) {
  unsigned long index = va >> shift;
  index = index & (PTRS_PER_TABLE - 1);
  if (!table[index]) {
    *new_table = 1;
    unsigned long next_level_table = get_free_page();
    unsigned long entry = next_level_table | MM_TYPE_PAGE_TABLE;
    table[index] = entry;
    return next_level_table;
  } else {
    *new_table = 0;
  }
  return table[index] & PAGE_MASK;
}

void map_stage2_page(struct task_struct *task, unsigned long va,
                     unsigned long page, unsigned long flags) {
  unsigned long lv1_table;
  if (!task->mm.first_table) {
    task->mm.first_table = get_free_page();
    task->mm.kernel_pages_count++;
  }
  lv1_table = task->mm.first_table;
  int new_table;
  unsigned long lv2_table = map_stage2_table((unsigned long *)(lv1_table + VA_START),
                                       LV1_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages_count++;
  }
  unsigned long lv3_table = map_stage2_table((unsigned long *)(lv2_table + VA_START),
                                       LV2_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages_count++;
  }
  map_stage2_table_entry((unsigned long *)(lv3_table + VA_START), va, page, flags);
  task->mm.user_pages_count++;
}


#define ISS_ABORT_DFSC_MASK  0x3f

int handle_mem_abort(unsigned long addr, unsigned long esr) {
  struct pt_regs *regs = task_pt_regs(current);
  unsigned int dfsc = esr & ISS_ABORT_DFSC_MASK;

  if (dfsc >> 2 == 0x1) {
    // translation fault
    unsigned long page = get_free_page();
    if (page == 0) {
      return -1;
    }
    map_stage2_page(current, addr & PAGE_MASK, page, MMU_STAGE2_PAGE_FLAGS);
    return 0;
  } else if (dfsc >> 2 == 0x3) {
    // permission fault (mmio)
    const struct board_ops *ops = current->board_ops;
    if (ops) {
      int sas = (esr >> 22) & 0x3;
      int srt = (esr >> 16) & 0x1f;
      int wnr = (esr >> 6) & 0x1;
      if (wnr == 0) {
        if (ops->mmio_read)
          regs->regs[srt] = ops->mmio_read(addr, sas);
      } else {
        if (ops->mmio_write)
          ops->mmio_write(addr, regs->regs[srt], sas);
      }
    }

    increment_current_pc(4);
    return 0;
  }
  return -1;
}
