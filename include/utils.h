#ifndef	_UTILS_H
#define	_UTILS_H

struct cpu_sysregs;

extern void delay ( unsigned long);
extern void put32 ( unsigned long, unsigned int );
extern unsigned int get32 ( unsigned long );
extern unsigned long get_el ( void );
extern void set_stage2_pgd(unsigned long pgd, unsigned long vmid);
extern void _set_sysregs(struct cpu_sysregs *);
extern void _get_sysregs(struct cpu_sysregs *);

#endif  /*_UTILS_H */
