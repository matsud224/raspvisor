#pragma once

#include <stddef.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

struct cpu_sysregs;

void memzero(void *, size_t);
void memcpy(void *, const void *, size_t);

void delay(unsigned long);
void put32(unsigned long, unsigned int);
unsigned int get32(unsigned long);
unsigned long get_el(void);
void set_stage2_pgd(unsigned long, unsigned long);
void restore_sysregs(struct cpu_sysregs *);
void save_sysregs(struct cpu_sysregs *);
void get_all_sysregs(struct cpu_sysregs *);
void assert_vfiq(void);
void assert_virq(void);
void assert_vserror(void);
void clear_vfiq(void);
void clear_virq(void);
void clear_vserror(void);
unsigned long translate_stage1(unsigned long);
unsigned long translate_stage12(unsigned long);

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
