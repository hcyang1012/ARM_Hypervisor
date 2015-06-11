#ifndef __ASM_SPINLOCK_H
#define __ASM_SPINLOCK_H

#include <asm/types.h>
#include <asm/processor.h>
#include <compiler.h>

#ifndef __ASSEMBLY__
typedef struct {
    volatile unsigned int lock;
} raw_spinlock_t;

typedef struct spinlock {
    raw_spinlock_t raw;
    u16 recurse_cpu:12;
    u16 recurse_cnt:4;
} spinlock_t;


#define spin_lock(l)                  _spin_lock(l)
#define spin_unlock(l)                _spin_unlock(l)

#define _raw_spin_is_locked(x)          ((x)->lock != 0)
#define cpu_relax() barrier() /* Could yield? */


static inline void dsb_sev(void)
{
    __asm__ __volatile__ (
        "dsb\n"
        "sev\n"
        );
}



void _spin_lock(spinlock_t *lock);
void _spin_unlock(spinlock_t *lock);

#endif
#endif 