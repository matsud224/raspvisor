#pragma once

#include "peripherals/base.h"

#define PL011_DR      (PBASE + 0x201000)
#define PL011_RSRECR  (PBASE + 0x201004)
#define PL011_FR      (PBASE + 0x201018)
#define PL011_ILPR    (PBASE + 0x201020)
#define PL011_IBRD    (PBASE + 0x201024)
#define PL011_FBRD    (PBASE + 0x201028)
#define PL011_LCRH    (PBASE + 0x20102c)
#define PL011_CR      (PBASE + 0x201030)
#define PL011_IFLS    (PBASE + 0x201034)
#define PL011_IMSC    (PBASE + 0x201038)
#define PL011_RIS     (PBASE + 0x20103c)
#define PL011_MIS     (PBASE + 0x201040)
#define PL011_ICR     (PBASE + 0x201044)
#define PL011_DMACR   (PBASE + 0x201048)
#define PL011_ITCR    (PBASE + 0x201080)
#define PL011_ITIP    (PBASE + 0x201084)
#define PL011_ITOP    (PBASE + 0x201088)
#define PL011_IDR     (PBASE + 0x20108c)
