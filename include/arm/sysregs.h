#ifndef _SYSREGS_H
#define _SYSREGS_H

// ***************************************
// SCTLR_EL2, System Control Register (EL2)
// ***************************************

#define SCTLR_EE                        (0 << 25)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED	(SCTLR_EE | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2)
// ***************************************

#define HCR_E2H           (0 << 34)
#define HCR_RW	    			(1 << 31)
#define HCR_TGE           (0 << 27)
#define HCR_TWI           (1 << 13)
#define HCR_AMO           (1 << 5)  // routing to EL2
#define HCR_IMO           (1 << 4)  // routing to EL2
#define HCR_FMO           (1 << 3)  // routing to EL2
#define HCR_SWIO          (1 << 1)
#define HCR_VM            (1 << 0)  // stage 2 translation enable
#define HCR_VALUE			(HCR_E2H | HCR_RW | HCR_TGE | HCR_TWI | HCR_AMO | HCR_IMO | HCR_FMO | HCR_SWIO | HCR_VM)

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3)
// ***************************************

#define SCR_RESERVED			(3 << 4)
#define SCR_RW				(1 << 10)
#define SCR_NS				(1 << 0)
#define SCR_VALUE	    	    	(SCR_RESERVED | SCR_RW | SCR_NS)

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3)
// ***************************************

#define SPSR_MASK_ALL 			(7 << 6)
#define SPSR_EL2h			(9 << 0)
#define SPSR_VALUE			(SPSR_MASK_ALL | SPSR_EL2h)

// ***************************************
// ESR_EL2, Exception Syndrome Register (EL2)
// ***************************************

#define ESR_EL2_EC_SHIFT               26
#define ESR_EL2_EC_TRAP_WFx            1
#define ESR_EL2_EC_HVC64          22
#define ESR_EL2_EC_DABT_LOW            36

// ***************************************
// VTCR_EL2,  Virtualization Translation Control Register (EL2)
// ***************************************

#define VTCR_NSA       (1 << 30)
#define VTCR_NSW       (1 << 29)
#define VTCR_VS        (1 << 19)
#define VTCR_PS        (5 << 16)
#define VTCR_TG0       (0 << 14)  // 4KB
#define VTCR_SL0       (2 << 6)
#define VTCR_T0SZ      (64 - 48)
#define VTCR_VALUE     (VTCR_NSA | VTCR_NSW | VTCR_VS | VTCR_PS | VTCR_TG0 | VTCR_SL0 | VTCR_T0SZ)

#endif
