#ifndef __ASM_PAGE_H
#define __ASM_PAGE_H

#include <asm/types.h>
#include <asm/cpregs.h>
#include <asm/processor.h>
#include <const.h>
#include <compiler.h>

#define LPAE_SHIFT      9
#define LPAE_ENTRIES    (_AC(1,U) << LPAE_SHIFT)
#define LPAE_L1_SIZE    (8)
#define LPAE_L2_SIZE    (LPAE_L1_SIZE * LPAE_ENTRIES)
#define LPAE_L3_SIZE    (LPAE_L2_SIZE * LPAE_ENTRIES)

#define THIRD_SHIFT    (PAGE_SHIFT)
#define THIRD_SIZE     ((paddr_t)1 << THIRD_SHIFT)
#define THIRD_MASK     (~(THIRD_SIZE - 1))
#define SECOND_SHIFT   (THIRD_SHIFT + LPAE_SHIFT)
#define SECOND_SIZE    ((paddr_t)1 << SECOND_SHIFT)
#define SECOND_MASK    (~(SECOND_SIZE - 1))
#define FIRST_SHIFT    (SECOND_SHIFT + LPAE_SHIFT)
#define FIRST_SIZE     ((paddr_t)1 << FIRST_SHIFT)
#define FIRST_MASK     (~(FIRST_SIZE - 1))
#define ZEROETH_SHIFT  (FIRST_SHIFT + LPAE_SHIFT)
#define ZEROETH_SIZE   ((paddr_t)1 << ZEROETH_SHIFT)
#define ZEROETH_MASK   (~(ZEROETH_SIZE - 1))

#define MAIR0VAL 0xeeaa4400
#define MAIR1VAL 0xff000004
#define MAIRVAL (MAIR0VAL|MAIR1VAL<<32)

#define PADDR_BITS              40
#define PADDR_MASK              ((1ULL << PADDR_BITS)-1)
#ifndef __ASSEMBLY__

#define __invalidate_dcache_one(R) STORE_CP32(R, DCIMVAC)
#define __clean_and_invalidate_dcache_one(R) STORE_CP32(R, DCCIMVAC)
#define __clean_dcache_one(R) STORE_CP32(R, DCCMVAC)
extern size_t cacheline_bytes;

static inline int invalidate_dcache_va_range(const void *p, unsigned long size)
{
    size_t off;
    const void *end = p + size;

    dsb(sy);           /* So the CPU issues all writes to the range */

    off = (unsigned long)p % cacheline_bytes;
    if ( off )
    {
        p -= off;
        asm volatile (__clean_and_invalidate_dcache_one(0) : : "r" (p));
        p += cacheline_bytes;
        size -= cacheline_bytes - off;
    }
    off = (unsigned long)end % cacheline_bytes;
    if ( off )
    {
        end -= off;
        size -= off;
        asm volatile (__clean_and_invalidate_dcache_one(0) : : "r" (end));
    }

    for ( ; p < end; p += cacheline_bytes )
        asm volatile (__invalidate_dcache_one(0) : : "r" (p));

    dsb(sy);           /* So we know the flushes happen before continuing */

    return 0;
}

static inline int clean_and_invalidate_dcache_va_range
    (const void *p, unsigned long size)
{
    const void *end;
    dsb(sy);         /* So the CPU issues all writes to the range */
    for ( end = p + size; p < end; p += cacheline_bytes )
        asm volatile (__clean_and_invalidate_dcache_one(0) : : "r" (p));
    dsb(sy);         /* So we know the flushes happen before continuing */
    /* ARM callers assume that dcache_* functions cannot fail. */
    return 0;
}


typedef struct __packed {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;      /* Valid mapping */
    unsigned long table:1;      /* == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long ai:3;         /* Attribute Index */
    unsigned long ns:1;         /* Not-Secure */
    unsigned long user:1;       /* User-visible */
    unsigned long ro:1;         /* Read-Only */
    unsigned long sh:2;         /* Shareability */
    unsigned long af:1;         /* Access Flag */
    unsigned long ng:1;         /* Not-Global */

    /* The base address must be appropriately aligned for Block entries */
    unsigned long long base:36; /* Base address of block or next table */
    unsigned long sbz:4;        /* Must be zero */

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long contig:1;     /* In a block of 16 contiguous entries */
    unsigned long pxn:1;        /* Privileged-XN */
    unsigned long xn:1;         /* eXecute-Never */
    unsigned long avail:4;      /* Ignored by hardware */

    /* These 5 bits are only used in Table entries and are ignored in
     * Block entries */
    unsigned long pxnt:1;       /* Privileged-XN */
    unsigned long xnt:1;        /* eXecute-Never */
    unsigned long apt:2;        /* Access Permissions */
    unsigned long nst:1;        /* Not-Secure */
} lpae_pt_t;

/* The p2m tables have almost the same layout, but some of the permission
 * and cache-control bits are laid out differently (or missing) */
typedef struct __packed {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;      /* Valid mapping */
    unsigned long table:1;      /* == 1 in 4k map entries too */

    /* These ten bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long mattr:4;      /* Memory Attributes */
    unsigned long read:1;       /* Read access */
    unsigned long write:1;      /* Write access */
    unsigned long sh:2;         /* Shareability */
    unsigned long af:1;         /* Access Flag */
    unsigned long sbz4:1;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long long base:36; /* Base address of block or next table */
    unsigned long sbz3:4;

    /* These seven bits are only used in Block entries and are ignored
     * in Table entries. */
    unsigned long contig:1;     /* In a block of 16 contiguous entries */
    unsigned long sbz2:1;
    unsigned long xn:1;         /* eXecute-Never */
    unsigned long type:4;       /* Ignore by hardware. Used to store p2m types */

    unsigned long sbz1:5;
} lpae_p2m_t;

/*
 * Walk is the common bits of p2m and pt entries which are needed to
 * simply walk the table (e.g. for debug).
 */
typedef struct __packed {
    /* These are used in all kinds of entry. */
    unsigned long valid:1;      /* Valid mapping */
    unsigned long table:1;      /* == 1 in 4k map entries too */

    unsigned long pad2:10;

    /* The base address must be appropriately aligned for Block entries */
    unsigned long long base:36; /* Base address of block or next table */

    unsigned long pad1:16;
} lpae_walk_t;

typedef union {
    uint64_t bits;
    lpae_pt_t pt;
    lpae_p2m_t p2m;
    lpae_walk_t walk;
} lpae_t;
#endif /*__ASSEMBLY__*/

#endif
