#pragma once

#include "peripherals/base.h"

#define VA_START 0x0000000000000000

#define PHYS_MEMORY_SIZE 0x40000000

#define PAGE_MASK 0xfffffffffffff000
#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY (2 * SECTION_SIZE)
#define HIGH_MEMORY DEVICE_BASE

#define PAGING_MEMORY (HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES (PAGING_MEMORY / PAGE_SIZE)

#define PTRS_PER_TABLE (1 << TABLE_SHIFT)

#define PGD_SHIFT			PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			PAGE_SHIFT + TABLE_SHIFT

#define LV1_SHIFT PAGE_SHIFT + 2 * TABLE_SHIFT
#define LV2_SHIFT PAGE_SHIFT + TABLE_SHIFT

#define PG_DIR_SIZE (3 * PAGE_SIZE)

#ifndef __ASSEMBLER__

#include "sched.h"
#include <inttypes.h>

typedef uint64_t paddr_t;
typedef uint64_t vaddr_t;

#define TO_VADDR(pa) ((vaddr_t)pa + VA_START)
#define TO_PADDR(pa) ((paddr_t)pa - VA_START)

void map_stage2_page(struct task_struct *task, vaddr_t va,
                     paddr_t page, uint64_t flags);

void *allocate_page(void);
void deallocate_page(void *);
void *allocate_task_page(struct task_struct *task, vaddr_t va);
void set_task_page_notaccessable(struct task_struct *task, vaddr_t va);

int handle_mem_abort(vaddr_t addr, uint64_t esr);

extern paddr_t pg_dir;

#endif
