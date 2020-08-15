#include <inttypes.h>
#include "debug.h"
#include "mm.h"
#include "sd.h"
#include "utils.h"
#include "fat32.h"

#define FAT32_MAX_FILENAME_LEN 255
#define BLOCKSIZE 512

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME                                                         \
  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

struct mbr {
  uint8_t bootloader[446];
  struct {
    uint8_t bootflag;
    uint8_t first_chs[3];
    uint8_t type;
    uint8_t last_chs[3];
    uint32_t first_lba;
    uint32_t total_sector;
  } __attribute__((__packed__)) partitiontable[4];
  uint8_t bootsig[2];
} __attribute__((__packed__));

struct fat32_dent {
  uint8_t DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t DIR_NTRes;
  uint8_t DIR_CrtTimeTenth;
  uint16_t DIR_CrtTime;
  uint16_t DIR_CrtDate;
  uint16_t DIR_LstAccDate;
  uint16_t DIR_FstClusHI;
  uint16_t DIR_WrtTime;
  uint16_t DIR_WrtDate;
  uint16_t DIR_FstClusLO;
  uint32_t DIR_FileSize;
} __attribute__((__packed__));

struct fat32_lfnent {
  uint8_t LDIR_Ord;
  uint8_t LDIR_Name1[10];
  uint8_t LDIR_Attr;
  uint8_t LDIR_Type;
  uint8_t LDIR_Chksum;
  uint8_t LDIR_Name2[12];
  uint16_t LDIR_FstClusLO;
  uint8_t LDIR_Name3[4];
} __attribute__((__packed__));


#define is_active_cluster(c) ((c) >= 0x2 && (c) <= 0x0ffffff6)
#define is_terminal_cluster(c) ((c) >= 0x0ffffff8 && (c) <= 0x0fffffff)
#define dir_cluster(dir) (((dir)->DIR_FstClusHI << 16) | (dir)->DIR_FstClusLO)
#define UNUSED_CLUSTER 0
#define RESERVED_CLUSTER 1
#define BAD_CLUSTER 0x0FFFFFF7

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i;
  for(i=0; i<n && *s1 && (*s1==*s2); i++, s1++, s2++);
  return (i!=n)?(*s1 - *s2):0;
}

static uint8_t *alloc_and_readblock(unsigned int lba) {
  uint8_t *buf = (uint8_t *)allocate_page();
  if (sd_readblock(lba, buf, 1) < 0)
    PANIC("sd_readblock() failed.");
  return buf;
}

static int fat32_is_valid_boot(struct fat32_boot *boot) {
  if (boot->BS_BootSign != 0xaa55)
    return 0;
  if (boot->BPB_BytsPerSec != BLOCKSIZE)
    return 0;
  if (!(boot->BS_FilSysType[0] == 'F' &&
        boot->BS_FilSysType[1] == 'A' &&
        boot->BS_FilSysType[2] == 'T'))
    return 0;

  return 1;
}

static void fat32_file_init(struct fat32_fs *fat32, struct fat32_file *fatfile,
    uint8_t attr, uint32_t size, uint32_t cluster) {
  fatfile->fat32 = fat32;
  fatfile->attr = attr;
  fatfile->size = size;
  fatfile->cluster = cluster;
}

int fat32_get_handle(struct fat32_fs *fat32) {
  uint8_t *bbuf = alloc_and_readblock(0);
  struct mbr *mbr = (struct mbr *)bbuf;

  if (mbr->bootsig[0] != 0x55 || mbr->bootsig[1] != 0xaa) {
    WARN("invalid boot signature in MBR.");
    return -1;
  }

  // assume that partition table 0 is FAT32 LBA
  if (mbr->partitiontable[0].type != 0xc) {
    WARN("not a FAT32 partition");
    return -1;
  }
  free_page(bbuf);

  uint32_t first_lba = mbr->partitiontable[0].first_lba;
  bbuf = alloc_and_readblock(mbr->partitiontable[0].first_lba);
  fat32->boot = *(struct fat32_boot *)bbuf;
  struct fat32_boot *boot = &(fat32->boot);
  free_page(bbuf);

  fat32->fatstart = boot->BPB_RsvdSecCnt;
  fat32->fatsectors = boot->BPB_FATSz32 * boot->BPB_NumFATs;
  fat32->rootstart = fat32->fatstart + fat32->fatsectors;
  fat32->rootsectors = (sizeof(struct fat32_dent) * boot->BPB_RootEntCnt +
                        boot->BPB_BytsPerSec - 1) / boot->BPB_BytsPerSec;
  fat32->datastart = fat32->rootstart + fat32->rootsectors;
  fat32->datasectors = boot->BPB_TotSec32 - fat32->datastart;
  fat32->first_lba = first_lba;
  if (fat32->datasectors / boot->BPB_SecPerClus < 65526 ||
      !fat32_is_valid_boot(boot)) {
    WARN("Bad fat32 filesystem.");
    return -1;
  }

  fat32_file_init(fat32, &fat32->root, ATTR_DIRECTORY, 0, fat32->boot.BPB_RootClus);

  return 0;
}

static uint32_t fatent_read(struct fat32_fs *fat32, uint32_t index) {
  struct fat32_boot *boot = &(fat32->boot);
  uint32_t sector = fat32->fatstart + (index * 4 / boot->BPB_BytsPerSec);
  uint32_t offset = index * 4 % boot->BPB_BytsPerSec;
  uint8_t *bbuf = alloc_and_readblock(sector + fat32->first_lba);
  uint32_t entry = *((uint32_t *)(bbuf + offset)) & 0x0fffffff;
  free_page(bbuf);
  return entry;
}

static uint32_t walk_cluster_chain(struct fat32_fs *fat32, uint32_t offset, uint32_t cluster) {
  int nlook =
      offset / (fat32->boot.BPB_SecPerClus * fat32->boot.BPB_BytsPerSec);

  struct fat32_boot *boot = &(fat32->boot);
  uint8_t *bbuf = NULL;
  uint32_t prevsector = 0;

  for (int i = 0; i < nlook; i++) {
    uint32_t sector = fat32->fatstart + (cluster * 4 / boot->BPB_BytsPerSec);
    uint32_t offset = cluster * 4 % boot->BPB_BytsPerSec;
    if (prevsector != sector) {
      if (bbuf != NULL)
        free_page(bbuf);
      bbuf = alloc_and_readblock(sector + fat32->first_lba);
    }
    cluster = *((uint32_t *)(bbuf + offset)) & 0x0fffffff;
    if (!is_active_cluster(cluster)) {
      cluster = BAD_CLUSTER;
      goto exit;
    }
  }
exit:
  if (bbuf != NULL)
    free_page(bbuf);
  return cluster;
}

static uint32_t cluster_to_sector(struct fat32_fs *fat32, uint32_t cluster) {
  return fat32->datastart + (cluster - 2) * fat32->boot.BPB_SecPerClus;
}

static uint8_t create_sum(struct fat32_dent *entry) {
  int i;
  uint8_t sum;

  for (i = sum = 0; i < 11; i++)
    sum = (sum >> 1) + (sum << 7) + entry->DIR_Name[i];
  return sum;
}

static char *get_sfn(struct fat32_dent *sfnent) {
  static char name[13];
  char *ptr = name;
  for (int i = 0; i <= 7; i++, ptr++) {
    *ptr = sfnent->DIR_Name[i];
    if (*ptr == 0x05)
      *ptr = 0xe5;
    if (*ptr == ' ')
      break;
  }
  if (sfnent->DIR_Name[8] != ' ') {
    *ptr++ = '.';
    for (int i = 8; i <= 10; i++, ptr++) {
      *ptr = sfnent->DIR_Name[i];
      if (*ptr == 0x05)
        *ptr = 0xe5;
      if (*ptr == ' ')
        break;
    }
  }
  *ptr = '\0';
  return name;
}

static char *get_lfn(struct fat32_dent *sfnent, size_t sfnoff,
                     struct fat32_dent *prevblk_dent) {
  struct fat32_lfnent *lfnent = (struct fat32_lfnent *)sfnent;
  uint8_t sum = create_sum(sfnent);
  static char name[256];
  char *ptr = name;
  int seq = 1;
  int is_prev_blk = 0;

  while (1) {
    if (!is_prev_blk && (sfnoff & (BLOCKSIZE - 1)) == 0) {
      // block boundary
      lfnent = (struct fat32_lfnent *)prevblk_dent;
      is_prev_blk = 1;
    } else {
      lfnent--;
    }

    if (lfnent == NULL || lfnent->LDIR_Chksum != sum ||
        (lfnent->LDIR_Attr & ATTR_LONG_NAME) != ATTR_LONG_NAME ||
        (lfnent->LDIR_Ord & 0x1f) != seq++) {
      return NULL;
    }
    for (int i = 0; i < 10; i += 2)
      *ptr++ = lfnent->LDIR_Name1[i]; // UTF16-LE
    for (int i = 0; i < 12; i += 2)
      *ptr++ = lfnent->LDIR_Name2[i]; // UTF16-LE
    for (int i = 0; i < 4; i += 2)
      *ptr++ = lfnent->LDIR_Name3[i]; // UTF16-LE
    if (lfnent->LDIR_Ord & 0x40)
      break;

    sfnoff -= sizeof(struct fat32_dent);
  }
  *ptr = '\0';
  return name;
}

static uint32_t fat32_firstblk(struct fat32_fs *fat32, uint32_t cluster, size_t file_off) {
  uint32_t secs_per_clus = fat32->boot.BPB_SecPerClus;
  uint32_t remblk = file_off % (secs_per_clus * BLOCKSIZE) / BLOCKSIZE;
  return cluster_to_sector(fat32, cluster) + remblk;
}

static int fat32_nextblk(struct fat32_fs *fat32, int prevblk, uint32_t *cluster) {
  uint32_t secs_per_clus = fat32->boot.BPB_SecPerClus;
  if (prevblk % secs_per_clus != secs_per_clus - 1) {
    return prevblk + 1;
  } else {
    // go over a cluster boundary
    *cluster = fatent_read(fat32, *cluster);
    return fat32_firstblk(fat32, *cluster, 0);
  }
}

static int fat32_lookup_main(struct fat32_file *fatfile, const char *name, struct fat32_file *found) {
  struct fat32_fs *fat32 = fatfile->fat32;

  if (!(fatfile->attr & ATTR_DIRECTORY))
    return -1;

  uint8_t *prevbuf = NULL;
  uint8_t *bbuf = NULL;
  uint32_t current_cluster = fatfile->cluster;

  for (int blkno = fat32_firstblk(fat32, current_cluster, 0);
       is_active_cluster(current_cluster);) {
    bbuf = alloc_and_readblock(blkno + fat32->first_lba);

    for (uint32_t i = 0; i < BLOCKSIZE; i += sizeof(struct fat32_dent)) {
      struct fat32_dent *dent = (struct fat32_dent *)(bbuf + i);

      if (dent->DIR_Name[0] == 0x00)
        break;
      if (dent->DIR_Name[0] == 0xe5)
        continue;
      if (dent->DIR_Attr & (ATTR_VOLUME_ID | ATTR_LONG_NAME))
        continue;

      char *dent_name = NULL;

      dent_name = get_lfn(dent, i, prevbuf ?
          prevbuf + (BLOCKSIZE - sizeof(struct fat32_dent)) : NULL);
      if (dent_name == NULL)
        dent_name = get_sfn(dent);


      INFO("File: %s size=%d(%x)", dent_name, dent->DIR_FileSize, dent->DIR_FileSize);
      if (strncmp(name, dent_name, FAT32_MAX_FILENAME_LEN) == 0) {
        uint32_t dent_clus = (dent->DIR_FstClusHI << 16) | dent->DIR_FstClusLO;
        if (dent_clus == 0) {
          // root directory
          dent_clus = fat32->boot.BPB_RootClus;
        }

        fat32_file_init(fat32, found, dent->DIR_Attr, dent->DIR_FileSize, dent_clus);
        INFO("found!");
        goto file_found;
      }
    }

    if (prevbuf != NULL)
      free_page(prevbuf);
    prevbuf = bbuf;
    bbuf = NULL;
    blkno = fat32_nextblk(fat32, blkno, &current_cluster);
  }

  if (prevbuf != NULL)
    free_page(prevbuf);
  if (bbuf != NULL)
    free_page(bbuf);
  return -1;

file_found:
  if (prevbuf != NULL)
    free_page(prevbuf);
  if (bbuf != NULL)
    free_page(bbuf);

  return 0;
}

int fat32_lookup(struct fat32_fs *fat32, const char *name, struct fat32_file *fatfile) {
  return fat32_lookup_main(&fat32->root, name, fatfile);
}

int fat32_read(struct fat32_file *fatfile, void *buf, unsigned long offset, size_t count) {
  struct fat32_fs *fat32 = fatfile->fat32;

  if (offset < 0)
    return -1;

  uint32_t tail = MIN(count + offset, fatfile->size);
  uint32_t remain = tail - offset;

  if (tail <= offset)
    return 0;

  uint32_t current_cluster = walk_cluster_chain(fat32, offset, fatfile->cluster);
  uint32_t inblk_off = offset % BLOCKSIZE;
  for (int blkno = fat32_firstblk(fat32, current_cluster, offset);
       remain > 0 && is_active_cluster(current_cluster);
       blkno = fat32_nextblk(fat32, blkno, &current_cluster)) {
    uint8_t *bbuf = alloc_and_readblock(blkno + fat32->first_lba);
    uint32_t copylen = MIN(BLOCKSIZE - inblk_off, remain);
    memcpy(buf, bbuf + inblk_off, copylen);
    free_page(bbuf);
    buf += copylen;
    remain -= copylen;

    inblk_off = 0;
  }

  uint32_t read_bytes = (tail - offset) - remain;
  return read_bytes;
}

int fat32_file_size(struct fat32_file *fatfile) {
  return fatfile->size;
}

int fat32_is_directory(struct fat32_file *fatfile) {
  return (fatfile->attr & ATTR_DIRECTORY) != 0;
}
