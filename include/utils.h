#pragma once

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
