#pragma once

#include <stddef.h>

struct fat32_boot {
  uint8_t BS_JmpBoot[3];
  uint8_t BS_OEMName[8];
  uint16_t BPB_BytsPerSec;
  uint8_t BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t BPB_NumFATs;
  uint16_t BPB_RootEntCnt;
  uint16_t BPB_TotSec16;
  uint8_t BPB_Media;
  uint16_t BPB_FATSz16;
  uint16_t BPB_SecPerTrk;
  uint16_t BPB_NumHeads;
  uint32_t BPB_HiddSec;
  uint32_t BPB_TotSec32;
  uint32_t BPB_FATSz32;
  uint16_t BPB_ExtFlags;
  uint16_t BPB_FSVer;
  uint32_t BPB_RootClus;
  uint16_t BPB_FSInfo;
  uint16_t BPB_BkBootSec;
  uint8_t BPB_Reserved[12];
  uint8_t BS_DrvNum;
  uint8_t BS_Reserved1;
  uint8_t BS_BootSig;
  uint32_t BS_VolID;
  uint8_t BS_VolLab[11];
  uint8_t BS_FilSysType[8];
  uint8_t BS_BootCode32[420];
  uint16_t BS_BootSign;
} __attribute__((__packed__));

struct fat32_fsi {
  uint32_t FSI_LeadSig;
  uint8_t FSI_Reserved1[480];
  uint32_t FSI_StrucSig;
  uint32_t FSI_Free_Count;
  uint32_t FSI_Nxt_Free;
  uint8_t FSI_Reserved2[12];
  uint32_t FSI_TrailSig;
} __attribute__((__packed__));

struct fat32_file {
  struct fat32_fs *fat32;
  uint8_t attr;
  uint32_t size;
  uint32_t cluster;
};

struct fat32_fs {
  struct fat32_boot boot;
  struct fat32_fsi fsi;
  uint32_t fatstart;
  uint32_t fatsectors;
  uint32_t rootstart;
  uint32_t rootsectors;
  uint32_t datastart;
  uint32_t datasectors;
  uint32_t first_lba;
  struct fat32_file root;
};

int fat32_get_handle(struct fat32_fs *);
int fat32_lookup(struct fat32_fs *, const char *, struct fat32_file *);
int fat32_read(struct fat32_file *, void *, unsigned long, size_t);
int fat32_file_size(struct fat32_file *);
int fat32_is_directory(struct fat32_file *);
