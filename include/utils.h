#pragma once

#include <stddef.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

struct cpu_sysregs;

void memzero(unsigned long, unsigned long);
void memcpy(unsigned long, unsigned long, unsigned long);

extern void delay(unsigned long);
extern void put32(unsigned long, unsigned int);
extern unsigned int get32(unsigned long);
extern unsigned long get_el(void);
extern void set_stage2_pgd(unsigned long, unsigned long);
extern void restore_sysregs(struct cpu_sysregs *);
extern void save_sysregs(struct cpu_sysregs *);
extern void get_all_sysregs(struct cpu_sysregs *);
extern void assert_vfiq(void);
extern void assert_virq(void);
extern void assert_vserror(void);
extern void clear_vfiq(void);
extern void clear_virq(void);
extern void clear_vserror(void);

int abs(int);
char *strncpy(char *, const char *, size_t);
size_t strnlen(const char *, size_t);
int strcmp(const char *, const char *);
int strncmp(const char *, const char *, size_t);
char *strdup(const char *);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
void *memmove(void *, const void *, size_t);
void *memchr(const void *, int, size_t);
char *strchr(const char *, int);
char *strcpy(char *, const char *);
char *strncat(char *, const char *, size_t);
char *strcat(char *, const char *);
int isdigit(int);
int isspace(int);
int toupper(int);
int tolower(int);

