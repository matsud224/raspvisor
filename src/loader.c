#include <inttypes.h>
#include "loader.h"
#include "fat32.h"
#include "mm.h"
#include "sched.h"
#include "utils.h"
#include "debug.h"

uint32_t crc_table[256];

void make_crc_table(void) {
  for (uint32_t i = 0; i < 256; i++) {
    uint32_t c = i;
    for (int j = 0; j < 8; j++) {
      c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
    }
    crc_table[i] = c;
  }
}

uint32_t crc32_init() {
  uint32_t c = 0xFFFFFFFF;
  return c;
}

uint32_t crc32_update(uint32_t c, uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
  }
  return c;
}

uint32_t crc32_finalize(uint32_t c) {
  return c ^ 0xFFFFFFFF;
}

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

  make_crc_table();
  uint32_t crc = crc32_init();
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

    crc = crc32_update(crc, buf, readsz);
  }

  printf("\ncrc32 of %s = %x\n", name, crc32_finalize(crc));

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
  unsigned long kernel_load_addr = 0x80000;
  unsigned long dtb_load_addr = 0x10000000;
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
