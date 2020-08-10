#pragma once

#define S_FRAME_SIZE 272 // size of all saved registers
#define S_X0 0           // offset of x0 register in saved stack frame

#define SYNC_INVALID_EL2      0
#define IRQ_INVALID_EL2       1
#define FIQ_INVALID_EL2       2
#define ERROR_INVALID_EL2     3

#define SYNC_INVALID_EL01_64  4
#define IRQ_INVALID_EL01_64   5
#define FIQ_INVALID_EL01_64   6
#define ERROR_INVALID_EL01_64 7

#define SYNC_INVALID_EL01_32  8
#define IRQ_INVALID_EL01_32   9
#define FIQ_INVALID_EL01_32   10
#define ERROR_INVALID_EL01_32 11

#ifndef __ASSEMBLER__

void switch_from_kthread(void);
void err_hang(void);

#endif
