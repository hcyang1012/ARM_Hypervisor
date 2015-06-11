#include <asm/types.h>
#include <asm/spinlock.h>


static int _raw_spin_trylock(raw_spinlock_t *lock)
{
    unsigned long contended, res;

    do {
        __asm__ __volatile__(
    "   ldrex   %0, [%2]\n"
    "   teq     %0, #0\n"
    "   strexeq %1, %3, [%2]\n"
    "   movne   %1, #0\n"
        : "=&r" (contended), "=r" (res)
        : "r" (&lock->lock), "r" (1)
        : "cc");
    } while (res);

    if (!contended) {
        smp_mb();
        return 1;
    } else {
        return 0;
    }
}

static void _raw_spin_unlock(raw_spinlock_t *lock)
{
    smp_mb();

    __asm__ __volatile__(
"   str     %1, [%0]\n"
    :
    : "r" (&lock->lock), "r" (0)
    : "cc");

    dsb_sev();
}

void _spin_lock(spinlock_t *lock)
{
    while ( !_raw_spin_trylock(&lock->raw) )
    {
        while ( _raw_spin_is_locked(&lock->raw) )
            cpu_relax();
    }
}

void _spin_unlock(spinlock_t *lock)
{
    _raw_spin_unlock(&lock->raw);
}