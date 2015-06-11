#ifndef __ASM_VCPU_H
#define __ASM_VCPU_H

#include <asm/types.h>
#include <asm/spinlock.h>

#define CPUINFO_sizeof 168 /* sizeof(struct cpu_info) */

#ifndef __ASSEMBLY__
typedef struct
{
	//general registers
	u32 regs[13];
	u32 hyp_lr;
	u32 hyp_spsr;
	spinlock_t lock;
} cpu_t;
#endif /*__ASSEMBLY__ */

#endif
