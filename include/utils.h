#pragma once

#include <stddef.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

struct cpu_sysregs;

void memzero(unsigned long src, unsigned long n);
void memcpy(unsigned long dst, unsigned long src, unsigned long n);

extern void delay(unsigned long);
extern void put32(unsigned long, unsigned int);
extern unsigned int get32(unsigned long);
extern unsigned long get_el(void);
extern void set_stage2_pgd(unsigned long pgd, unsigned long vmid);
extern void _set_sysregs(struct cpu_sysregs *);
extern void _get_sysregs(struct cpu_sysregs *);
extern void assert_vfiq(void);
extern void assert_virq(void);
extern void assert_vserror(void);
extern void clear_vfiq(void);
extern void clear_virq(void);
extern void clear_vserror(void);

int abs(int n);
char *strncpy(char *dest, const char *src, size_t n);
size_t strnlen(const char *s, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strdup(const char *str);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
char *strcpy(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
int isdigit(int c);
int isspace(int c);
int toupper(int c);
int tolower(int c);

