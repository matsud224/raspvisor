#ifndef _USER_SYS_H
#define _USER_SYS_H

void call_hvc_notify(void);
void call_hvc_exit(void);

extern void user_delay(unsigned long);
extern unsigned long get_sp(void);
extern unsigned long get_pc(void);

#endif /*_USER_SYS_H */
