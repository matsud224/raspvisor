#include "utils.h"
#include "debug.h"
#include "mm.h"
#include "board.h"
#include "task.h"
#include "arm/mmu.h"

static unsigned short mem_map[PAGING_PAGES] = { 0 };

paddr_t get_free_page(void);
void free_page(paddr_t);

void *allocate_page() {
  paddr_t page = get_free_page();
  if (page == 0) {
    return 0;
  }
  return (void *)TO_VADDR(page);
}

void deallocate_page(void *page) {
  free_page(TO_PADDR(page));
}

void *allocate_task_page(struct task_struct *task, vaddr_t va) {
  paddr_t page = get_free_page();
  if (page == 0) {
    return 0;
  }
  map_stage2_page(task, va, page, MMU_STAGE2_PAGE_FLAGS);
  return (void *)TO_VADDR(page);
}

void set_task_page_notaccessable(struct task_struct *task, vaddr_t va) {
  map_stage2_page(task, va, 0, MMU_STAGE2_MMIO_PAGE_FLAGS);
}

paddr_t get_free_page() {
  for (int i = 0; i < PAGING_PAGES; i++) {
    if (mem_map[i] == 0) {
      mem_map[i] = 1;
      paddr_t page = LOW_MEMORY + i * PAGE_SIZE;
      memzero((void *)TO_VADDR(page), PAGE_SIZE);
      return page;
    }
  }
  PANIC("no free pages!");
  return 0;
}

void free_page(paddr_t p) {
  mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}

void map_stage2_table_entry(vaddr_t pte, vaddr_t va,
                            paddr_t pa, uint64_t flags) {
  uint64_t index = va >> PAGE_SHIFT;
  index = index & (PTRS_PER_TABLE - 1);
  uint64_t entry = pa | flags;
  ((uint64_t *)pte)[index] = entry;
}

paddr_t map_stage2_table(vaddr_t table, uint64_t shift,
                               vaddr_t va, int *new_table) {
  uint64_t index = va >> shift;
  index = index & (PTRS_PER_TABLE - 1);
  if (!((uint64_t *)table)[index]) {
    *new_table = 1;
    paddr_t next_level_table = get_free_page();
    uint64_t entry = next_level_table | MM_TYPE_PAGE_TABLE;
    ((uint64_t *)table)[index] = entry;
    return next_level_table;
  } else {
    *new_table = 0;
  }
  return ((uint64_t *)table)[index] & PAGE_MASK;
}

void map_stage2_page(struct task_struct *task, vaddr_t va,
                     paddr_t page, uint64_t flags) {
  paddr_t lv1_table;
  if (!task->mm.first_table) {
    task->mm.first_table = get_free_page();
    task->mm.kernel_pages_count++;
  }
  lv1_table = task->mm.first_table;
  int new_table;
  paddr_t lv2_table = map_stage2_table(TO_VADDR(lv1_table),
                                       LV1_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages_count++;
  }
  paddr_t lv3_table = map_stage2_table(TO_VADDR(lv2_table),
                                       LV2_SHIFT, va, &new_table);
  if (new_table) {
    task->mm.kernel_pages_count++;
  }
  map_stage2_table_entry(TO_VADDR(lv3_table), va, page, flags);
  task->mm.user_pages_count++;
}

paddr_t get_ipa(vaddr_t va) {
  paddr_t ipa = translate_el1(va);
  ipa &= 0xFFFFFFFFF000;
  ipa |= va & 0xFFF;
  return ipa;
}

#define ISS_ABORT_DFSC_MASK  0x3f

int handle_mem_abort(vaddr_t addr, uint64_t esr) {
  struct pt_regs *regs = task_pt_regs(current);
  uint64_t dfsc = esr & ISS_ABORT_DFSC_MASK;

  if (dfsc >> 2 == 0x1) {
    // translation fault
    paddr_t page = get_free_page();
    if (page == 0) {
      return -1;
    }
    map_stage2_page(current, get_ipa(addr) & PAGE_MASK, page, MMU_STAGE2_PAGE_FLAGS);
    current->stat.pf_count++;
    return 0;
  } else if (dfsc >> 2 == 0x3) {
    // permission fault (mmio)
    const struct board_ops *ops = current->board_ops;
    //int sas = (esr >> 22) & 0x3;
    unsigned int srt = (esr >> 16) & 0x1f;
    unsigned int wnr = (esr >> 6) & 0x1;
    if (wnr == 0) {
      if (HAVE_FUNC(ops, mmio_read))
        regs->regs[srt] = ops->mmio_read(current, get_ipa(addr));
    } else {
      if (HAVE_FUNC(ops, mmio_write))
        ops->mmio_write(current, get_ipa(addr), regs->regs[srt]);
    }

    increment_current_pc(4);
    current->stat.mmio_count++;
    return 0;
  }
  return -1;
}
