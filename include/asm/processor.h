#ifndef __ASM_PROCESSOR_H
#define __ASM_PROCESSOR_H

#include <const.h>


/* MIDR Main ID Register */
#define MIDR_MASK    0xff0ffff0

/* MPIDR Multiprocessor Affinity Register */
#define _MPIDR_UP           (30)
#define MPIDR_UP            (_AC(1,U) << _MPIDR_UP)
#define _MPIDR_SMP          (31)
#define MPIDR_SMP           (_AC(1,U) << _MPIDR_SMP)
#define MPIDR_AFF0_SHIFT    (0)
#define MPIDR_AFF0_MASK     (_AC(0xff,U) << MPIDR_AFF0_SHIFT)
#define MPIDR_HWID_MASK     _AC(0xffffff,U)
#define MPIDR_INVALID       (~MPIDR_HWID_MASK)
#define MPIDR_LEVEL_BITS    (8)
#define AFFINITY_MASK(level)    ~((_AC(0x1,U) << ((level) * MPIDR_LEVEL_BITS)) - 1)


/*
 * Macros to extract affinity level. picked from kernel
 */

#define MPIDR_LEVEL_BITS_SHIFT  3
#define MPIDR_LEVEL_MASK        ((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
         (((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
         ((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

/* TTBCR Translation Table Base Control Register */
#define TTBCR_EAE    _AC(0x80000000,U)
#define TTBCR_N_MASK _AC(0x07,U)
#define TTBCR_N_16KB _AC(0x00,U)
#define TTBCR_N_8KB  _AC(0x01,U)
#define TTBCR_N_4KB  _AC(0x02,U)
#define TTBCR_N_2KB  _AC(0x03,U)
#define TTBCR_N_1KB  _AC(0x04,U)

/* SCTLR System Control Register. */
/* HSCTLR is a subset of this. */
#define SCTLR_TE        (_AC(1,U)<<30)
#define SCTLR_AFE       (_AC(1,U)<<29)
#define SCTLR_TRE       (_AC(1,U)<<28)
#define SCTLR_NMFI      (_AC(1,U)<<27)
#define SCTLR_EE        (_AC(1,U)<<25)
#define SCTLR_VE        (_AC(1,U)<<24)
#define SCTLR_U         (_AC(1,U)<<22)
#define SCTLR_FI        (_AC(1,U)<<21)
#define SCTLR_WXN       (_AC(1,U)<<19)
#define SCTLR_HA        (_AC(1,U)<<17)
#define SCTLR_RR        (_AC(1,U)<<14)
#define SCTLR_V         (_AC(1,U)<<13)
#define SCTLR_I         (_AC(1,U)<<12)
#define SCTLR_Z         (_AC(1,U)<<11)
#define SCTLR_SW        (_AC(1,U)<<10)
#define SCTLR_B         (_AC(1,U)<<7)
#define SCTLR_C         (_AC(1,U)<<2)
#define SCTLR_A         (_AC(1,U)<<1)
#define SCTLR_M         (_AC(1,U)<<0)

#define HSCTLR_BASE     _AC(0x30c51878,U)

/* HCR Hyp Configuration Register */
#define HCR_RW          (_AC(1,UL)<<31) /* Register Width, ARM64 only */
#define HCR_TGE         (_AC(1,UL)<<27) /* Trap General Exceptions */
#define HCR_TVM         (_AC(1,UL)<<26) /* Trap Virtual Memory Controls */
#define HCR_TTLB        (_AC(1,UL)<<25) /* Trap TLB Maintenance Operations */
#define HCR_TPU         (_AC(1,UL)<<24) /* Trap Cache Maintenance Operations to PoU */
#define HCR_TPC         (_AC(1,UL)<<23) /* Trap Cache Maintenance Operations to PoC */
#define HCR_TSW         (_AC(1,UL)<<22) /* Trap Set/Way Cache Maintenance Operations */
#define HCR_TAC         (_AC(1,UL)<<21) /* Trap ACTLR Accesses */
#define HCR_TIDCP       (_AC(1,UL)<<20) /* Trap lockdown */
#define HCR_TSC         (_AC(1,UL)<<19) /* Trap SMC instruction */
#define HCR_TID3        (_AC(1,UL)<<18) /* Trap ID Register Group 3 */
#define HCR_TID2        (_AC(1,UL)<<17) /* Trap ID Register Group 2 */
#define HCR_TID1        (_AC(1,UL)<<16) /* Trap ID Register Group 1 */
#define HCR_TID0        (_AC(1,UL)<<15) /* Trap ID Register Group 0 */
#define HCR_TWE         (_AC(1,UL)<<14) /* Trap WFE instruction */
#define HCR_TWI         (_AC(1,UL)<<13) /* Trap WFI instruction */
#define HCR_DC          (_AC(1,UL)<<12) /* Default cacheable */
#define HCR_BSU_MASK    (_AC(3,UL)<<10) /* Barrier Shareability Upgrade */
#define HCR_BSU_NONE     (_AC(0,UL)<<10)
#define HCR_BSU_INNER    (_AC(1,UL)<<10)
#define HCR_BSU_OUTER    (_AC(2,UL)<<10)
#define HCR_BSU_FULL     (_AC(3,UL)<<10)
#define HCR_FB          (_AC(1,UL)<<9) /* Force Broadcast of Cache/BP/TLB operations */
#define HCR_VA          (_AC(1,UL)<<8) /* Virtual Asynchronous Abort */
#define HCR_VI          (_AC(1,UL)<<7) /* Virtual IRQ */
#define HCR_VF          (_AC(1,UL)<<6) /* Virtual FIQ */
#define HCR_AMO         (_AC(1,UL)<<5) /* Override CPSR.A */
#define HCR_IMO         (_AC(1,UL)<<4) /* Override CPSR.I */
#define HCR_FMO         (_AC(1,UL)<<3) /* Override CPSR.F */
#define HCR_PTW         (_AC(1,UL)<<2) /* Protected Walk */
#define HCR_SWIO        (_AC(1,UL)<<1) /* Set/Way Invalidation Override */
#define HCR_VM          (_AC(1,UL)<<0) /* Virtual MMU Enable */

/* TCR: Stage 1 Translation Control */

#define TCR_T0SZ(x)     ((x)<<0)

#define TCR_IRGN0_NC    (_AC(0x0,UL)<<8)
#define TCR_IRGN0_WBWA  (_AC(0x1,UL)<<8)
#define TCR_IRGN0_WT    (_AC(0x2,UL)<<8)
#define TCR_IRGN0_WB    (_AC(0x3,UL)<<8)

#define TCR_ORGN0_NC    (_AC(0x0,UL)<<10)
#define TCR_ORGN0_WBWA  (_AC(0x1,UL)<<10)
#define TCR_ORGN0_WT    (_AC(0x2,UL)<<10)
#define TCR_ORGN0_WB    (_AC(0x3,UL)<<10)

#define TCR_SH0_NS      (_AC(0x0,UL)<<12)
#define TCR_SH0_OS      (_AC(0x2,UL)<<12)
#define TCR_SH0_IS      (_AC(0x3,UL)<<12)

#define TCR_TG0_4K      (_AC(0x0,UL)<<14)
#define TCR_TG0_64K     (_AC(0x1,UL)<<14)
#define TCR_TG0_16K     (_AC(0x2,UL)<<14)

#ifdef CONFIG_ARM_64

#define TCR_PS(x)       ((x)<<16)
#define TCR_TBI         (_AC(0x1,UL)<<20)

#define TCR_RES1        (_AC(1,UL)<<31|_AC(1,UL)<<23)

#else

#define TCR_RES1        (_AC(1,UL)<<31)

#endif

/* VTCR: Stage 2 Translation Control */

#define VTCR_T0SZ(x)    ((x)<<0)

#define VTCR_SL0(x)     ((x)<<6)

#define VTCR_IRGN0_NC   (_AC(0x0,UL)<<8)
#define VTCR_IRGN0_WBWA (_AC(0x1,UL)<<8)
#define VTCR_IRGN0_WT   (_AC(0x2,UL)<<8)
#define VTCR_IRGN0_WB   (_AC(0x3,UL)<<8)

#define VTCR_ORGN0_NC   (_AC(0x0,UL)<<10)
#define VTCR_ORGN0_WBWA (_AC(0x1,UL)<<10)
#define VTCR_ORGN0_WT   (_AC(0x2,UL)<<10)
#define VTCR_ORGN0_WB   (_AC(0x3,UL)<<10)

#define VTCR_SH0_NS     (_AC(0x0,UL)<<12)
#define VTCR_SH0_OS     (_AC(0x2,UL)<<12)
#define VTCR_SH0_IS     (_AC(0x3,UL)<<12)

#ifdef CONFIG_ARM_64

#define VTCR_TG0_4K     (_AC(0x0,UL)<<14)
#define VTCR_TG0_64K    (_AC(0x1,UL)<<14)
#define VTCR_TG0_16K    (_AC(0x2,UL)<<14)

#define VTCR_PS(x)      ((x)<<16)

#endif

#define VTCR_RES1       (_AC(1,UL)<<31)

/* HCPTR Hyp. Coprocessor Trap Register */
#define HCPTR_TTA       ((_AC(1,U)<<20))        /* Trap trace registers */
#define HCPTR_CP(x)     ((_AC(1,U)<<(x)))       /* Trap Coprocessor x */
#define HCPTR_CP_MASK   ((_AC(1,U)<<14)-1)

/* HSTR Hyp. System Trap Register */
#define HSTR_T(x)       ((_AC(1,U)<<(x)))       /* Trap Cp15 c<x> */

/* HDCR Hyp. Debug Configuration Register */
#define HDCR_TDRA       (_AC(1,U)<<11)          /* Trap Debug ROM access */
#define HDCR_TDOSA      (_AC(1,U)<<10)          /* Trap Debug-OS-related register access */
#define HDCR_TDA        (_AC(1,U)<<9)           /* Trap Debug Access */
#define HDCR_TDE        (_AC(1,U)<<8)           /* Route Soft Debug exceptions from EL1/EL1 to EL2 */
#define HDCR_TPM        (_AC(1,U)<<6)           /* Trap Performance Monitors accesses */
#define HDCR_TPMCR      (_AC(1,U)<<5)           /* Trap PMCR accesses */

#define HSR_EC_UNKNOWN              0x00
#define HSR_EC_WFI_WFE              0x01
#define HSR_EC_CP15_32              0x03
#define HSR_EC_CP15_64              0x04
#define HSR_EC_CP14_32              0x05        /* Trapped MCR or MRC access to CP14 */
#define HSR_EC_CP14_DBG             0x06        /* Trapped LDC/STC access to CP14 (only for debug registers) */
#define HSR_EC_CP                   0x07        /* HCPTR-trapped access to CP0-CP13 */
#define HSR_EC_CP10                 0x08
#define HSR_EC_JAZELLE              0x09
#define HSR_EC_BXJ                  0x0a
#define HSR_EC_CP14_64              0x0c
#define HSR_EC_SVC32                0x11
#define HSR_EC_HVC32                0x12
#define HSR_EC_SMC32                0x13
#ifdef CONFIG_ARM_64
#define HSR_EC_HVC64                0x16
#define HSR_EC_SMC64                0x17
#define HSR_EC_SYSREG               0x18
#endif
#define HSR_EC_INSTR_ABORT_LOWER_EL 0x20
#define HSR_EC_INSTR_ABORT_CURR_EL  0x21
#define HSR_EC_DATA_ABORT_LOWER_EL  0x24
#define HSR_EC_DATA_ABORT_CURR_EL   0x25
#ifdef CONFIG_ARM_64
#define HSR_EC_BRK                  0x3c
#endif

/* FSR format, common */
#define FSR_LPAE                (_AC(1,UL)<<9)
/* FSR short format */
#define FSRS_FS_DEBUG           (_AC(0,UL)<<10|_AC(0x2,UL)<<0)
/* FSR long format */
#define FSRL_STATUS_DEBUG       (_AC(0x22,UL)<<0)



#define __CP32(r, coproc, opc1, crn, crm, opc2) coproc, opc1, r, crn, crm, opc2
#define __CP64(r1, r2, coproc, opc, crm) coproc, opc, r1, r2, crm
#define CP32(r, name...) __CP32(r, name)
#define CP64(r, name...) __CP64(r, name)

#ifndef __ASSEMBLY__
#define nop() \
    asm volatile ( "nop" )

#define sev()           asm volatile("sev" : : : "memory")
#define wfe()           asm volatile("wfe" : : : "memory")
#define wfi()           asm volatile("wfi" : : : "memory")

#define isb()           asm volatile("isb" : : : "memory")
#define dsb(scope)      asm volatile("dsb " #scope : : : "memory")
#define dmb(scope)      asm volatile("dmb " #scope : : : "memory")

#define mb()            dsb(sy)
#ifdef CONFIG_ARM_64
#define rmb()           dsb(ld)
#else
#define rmb()           dsb(sy) /* 32-bit has no ld variant. */
#endif
#define wmb()           dsb(st)

#define smp_mb()        dmb(ish)
#ifdef CONFIG_ARM_64
#define smp_rmb()       dmb(ishld)
#else
#define smp_rmb()       dmb(ish) /* 32-bit has no ishld variant. */
#endif

#define smp_wmb()       dmb(ishst)
#endif //__ASSEMBLY__

#define Mode_USR 0x10
#define Mode_FIQ 0x11
#define Mode_IRQ 0x12
#define Mode_SVC 0x13
#define Mode_ABT 0x17
#define Mode_SYS 0x1F
#define Mode_SVP 0x13
#define Mode_UNDEF 0x1B
#define Mode_HYP 0x1A
#define Mode_MON 0x16

/* PSR bits (CPSR, SPSR) */

#define PSR_THUMB       (1<<5)        /* Thumb Mode enable */
#define PSR_FIQ_MASK    (1<<6)        /* Fast Interrupt mask */
#define PSR_IRQ_MASK    (1<<7)        /* Interrupt mask */
#define PSR_ABT_MASK    (1<<8)        /* Asynchronous Abort mask */
#define PSR_BIG_ENDIAN  (1<<9)        /* arm32: Big Endian Mode */
#define PSR_DBG_MASK    (1<<9)        /* arm64: Debug Exception mask */
#define PSR_IT_MASK     (0x0600fc00)  /* Thumb If-Then Mask */
#define PSR_JAZELLE     (1<<24)       /* Jazelle Mode */

/* 32 bit modes */
#define PSR_MODE_USR 0x10
#define PSR_MODE_FIQ 0x11
#define PSR_MODE_IRQ 0x12
#define PSR_MODE_SVC 0x13
#define PSR_MODE_MON 0x16
#define PSR_MODE_ABT 0x17
#define PSR_MODE_HYP 0x1a
#define PSR_MODE_UND 0x1b
#define PSR_MODE_SYS 0x1f

/* 64 bit modes */
#define PSR_MODE_BIT  0x10 /* Set iff AArch32 */
#define PSR_MODE_EL3h 0x0d
#define PSR_MODE_EL3t 0x0c
#define PSR_MODE_EL2h 0x09
#define PSR_MODE_EL2t 0x08
#define PSR_MODE_EL1h 0x05
#define PSR_MODE_EL1t 0x04
#define PSR_MODE_EL0t 0x00

#define PSR_GUEST32_INIT  (PSR_ABT_MASK|PSR_FIQ_MASK|PSR_IRQ_MASK|PSR_MODE_SVC)

/* Physical Address Register */
#define PAR_F           (_AC(1,U)<<0)

/* .... If F == 1 */
#define PAR_FSC_SHIFT   (1)
#define PAR_FSC_MASK    (_AC(0x3f,U)<<PAR_FSC_SHIFT)
#define PAR_STAGE21     (_AC(1,U)<<8)     /* Stage 2 Fault During Stage 1 Walk */
#define PAR_STAGE2      (_AC(1,U)<<9)     /* Stage 2 Fault */

/* If F == 0 */
#define PAR_MAIR_SHIFT  56                       /* Memory Attributes */
#define PAR_MAIR_MASK   (0xffLL<<PAR_MAIR_SHIFT)
#define PAR_NS          (_AC(1,U)<<9)                   /* Non-Secure */
#define PAR_SH_SHIFT    7                        /* Shareability */
#define PAR_SH_MASK     (_AC(3,U)<<PAR_SH_SHIFT)

#endif
