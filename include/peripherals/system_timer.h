#pragma once

#include "peripherals/base.h"

#define SYSTIMER_CS  (PBASE + 0x00003000)
#define SYSTIMER_CLO (PBASE + 0x00003004)
#define SYSTIMER_CHI (PBASE + 0x00003008)
#define SYSTIMER_C0  (PBASE + 0x0000300C)
#define SYSTIMER_C1  (PBASE + 0x00003010)
#define SYSTIMER_C2  (PBASE + 0x00003014)
#define SYSTIMER_C3  (PBASE + 0x00003018)

#define SYSTIMER_CS_M0 (1 << 0)
#define SYSTIMER_CS_M1 (1 << 1)
#define SYSTIMER_CS_M2 (1 << 2)
#define SYSTIMER_CS_M3 (1 << 3)
