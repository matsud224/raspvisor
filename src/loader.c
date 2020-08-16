#include <inttypes.h>
#include "loader.h"
#include "fat32.h"
#include "mm.h"
#include "sched.h"
#include "utils.h"
#include "debug.h"

// va should be page-aligned.
int load_file_to_memory(struct task_struct *tsk, const char *name, unsigned long va) {
  struct fat32_fs hfat;
  if (fat32_get_handle(&hfat) < 0) {
    WARN("failed to find fat32 filesystem.");
    return -1;
  }

  struct fat32_file file;
  if (fat32_lookup(&hfat, name, &file) < 0) {
    WARN("requested file \"%s\" is not found.", name);
    return -1;
  }

  int remain = fat32_file_size(&file);
  int offset = 0;
  unsigned long current_va = va & PAGE_MASK;

  while (remain > 0) {
    uint8_t *buf = allocate_task_page(tsk, current_va);
    int readsz = MIN(PAGE_SIZE, remain);
    int actual = fat32_read(&file, buf, offset, readsz);

    if (readsz != actual) {
      WARN("error during file read");
      return -1;
    }

    remain -= readsz;
    offset += readsz;
    current_va += PAGE_SIZE;
  }

  return 0;
}


int raw_binary_loader (void *arg, unsigned long *pc,
    unsigned long *sp) {
  struct raw_binary_loader_args *loader_args = arg;
  if (load_file_to_memory(current,
        loader_args->filename, loader_args->load_addr) < 0)
    return -1;

  *pc = loader_args->entry_point;
  *sp = loader_args->sp;

  return 0;
}

int test_program_loader (void *arg, unsigned long *pc,
    unsigned long *sp) {
  extern unsigned long el1_test_1;
  extern unsigned long el1_test_2;
  extern unsigned long el1_test_begin;
  extern unsigned long el1_test_end;

  unsigned long begin = (unsigned long)&el1_test_begin;
  unsigned long end = (unsigned long)&el1_test_end;
  unsigned long size = end - begin;
  unsigned long func = (unsigned long)&el1_test_1;

  switch ((unsigned long)arg) {
  case 1:
    func = (unsigned long)&el1_test_1;
    break;
  case 2:
    func = (unsigned long)&el1_test_2;
    break;
  }
  unsigned long entry_point = func - begin;

  unsigned long code_page = allocate_task_page(current, 0);
  if (code_page == 0) {
    return -1;
  }
  memcpy(code_page, begin, size);

  *pc = entry_point;
  *sp = 2 * PAGE_SIZE;

  return 0;
}
