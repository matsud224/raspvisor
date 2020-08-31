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
  int total = remain;
  int offset = 0;
  int progress = 0;
  unsigned long current_va = va & PAGE_MASK;

  printf("  0%%");
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

    if (progress != (total - remain) * 100 / total)
      printf("\b\b\b\b%3d%%", progress = (total - remain) * 100 / total);
  }

  return 0;
}


int raw_binary_loader (void *arg, struct pt_regs *regs) {
  struct raw_binary_loader_args *loader_args = arg;
  if (load_file_to_memory(current,
        loader_args->filename, loader_args->load_addr) < 0)
    return -1;

  regs->pc = loader_args->entry_point;
  regs->sp = loader_args->sp;

  current->name = loader_args->filename;

  return 0;
}

int linux_loader (void *arg, struct pt_regs *regs) {
  struct linux_loader_args *loader_args = arg;

  // FIXME
  unsigned long kernel_load_addr = 0x0;
  unsigned long dtb_load_addr = 0x30000000;
  //unsigned long initramfs_load_addr = 0x0;

  if (load_file_to_memory(current,
        loader_args->kernel_image, kernel_load_addr) < 0)
    return -1;

  if (load_file_to_memory(current,
        loader_args->device_tree, dtb_load_addr) < 0)
    return -1;

  /*
  if (loader_args->initramfs != NULL &&
      load_file_to_memory(current,
        loader_args->initramfs, initramfs_load_addr) < 0)
    return -1;
  */

  regs->pc = kernel_load_addr;
  regs->regs[0] = dtb_load_addr;
  regs->regs[1] = 0;
  regs->regs[2] = 0;
  regs->regs[3] = 0;

  current->name = loader_args->kernel_image;

  return 0;
}
