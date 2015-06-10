#ifndef __ASM_TRAPS_H
#define __ASM_TRAPS_H
#include <asm/types.h>

void init_traps(void);
#ifndef __ASSEMBLY__

union hsr {
    uint32_t bits;
    struct {
        unsigned long iss:25;  /* Instruction Specific Syndrome */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    };

    /* Common to all conditional exception classes (0x0N, except 0x00). */
    struct hsr_cond {
        unsigned long iss:20;  /* Instruction Specific Syndrome */
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } cond;

    struct hsr_wfi_wfe {
        unsigned long ti:1;    /* Trapped instruction */
        unsigned long sbzp:19;
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } wfi_wfe;

    /* reg, reg0, reg1 are 4 bits on AArch32, the fifth bit is sbzp. */
    struct hsr_cp32 {
        unsigned long read:1;  /* Direction */
        unsigned long crm:4;   /* CRm */
        unsigned long reg:5;   /* Rt */
        unsigned long crn:4;   /* CRn */
        unsigned long op1:3;   /* Op1 */
        unsigned long op2:3;   /* Op2 */
        unsigned long cc:4;    /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } cp32; /* HSR_EC_CP15_32, CP14_32, CP10 */

    struct hsr_cp64 {
        unsigned long read:1;   /* Direction */
        unsigned long crm:4;    /* CRm */
        unsigned long reg1:5;   /* Rt1 */
        unsigned long reg2:5;   /* Rt2 */
        unsigned long sbzp:1;
        unsigned long op1:4;    /* Op1 */
        unsigned long cc:4;     /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;     /* Exception Class */
    } cp64; /* HSR_EC_CP15_64, HSR_EC_CP14_64 */

     struct hsr_cp {
        unsigned long coproc:4; /* Number of coproc accessed */
        unsigned long sbz0p:1;
        unsigned long tas:1;    /* Trapped Advanced SIMD */
        unsigned long res0:14;
        unsigned long cc:4;     /* Condition Code */
        unsigned long ccvalid:1;/* CC Valid */
        unsigned long len:1;    /* Instruction length */
        unsigned long ec:6;     /* Exception Class */
    } cp; /* HSR_EC_CP */
    struct hsr_dabt {
        unsigned long dfsc:6;  /* Data Fault Status Code */
        unsigned long write:1; /* Write / not Read */
        unsigned long s1ptw:1; /* */
        unsigned long cache:1; /* Cache Maintenance */
        unsigned long eat:1;   /* External Abort Type */
        unsigned long sbzp0:6;
        unsigned long reg:5;   /* Register */
        unsigned long sign:1;  /* Sign extend */
        unsigned long size:2;  /* Access Size */
        unsigned long valid:1; /* Syndrome Valid */
        unsigned long len:1;   /* Instruction length */
        unsigned long ec:6;    /* Exception Class */
    } dabt; /* HSR_EC_DATA_ABORT_* */
};


enum EPT_VIOLATION_REASON {PREFETCH = 0, DABT};
struct ept_violation_info_t
{
    union hsr hsr;
    enum EPT_VIOLATION_REASON reason;
    vaddr_t gva;
    paddr_t gpa;
} ;


#endif /* __ASSEMBLY__ */

#endif
