#pragma once

#include <stddef.h>

struct fat32_fs;
struct fat32_file;

int fat32_get_handle(struct fat32_fs *);
int fat32_lookup(struct fat32_fs *, const char *, struct fat32_file *);
int fat32_read(struct fat32_file *, void *, unsigned long, size_t);
int fat32_file_size(struct fat32_file *);
int fat32_is_directory(struct fat32_file *);
