#ifndef __ASM_VCPU_H
#define __ASM_VCPU_H

#include <asm/types.h>

#define CPUINFO_sizeof 168 /* sizeof(struct cpu_info) */

#ifndef __ASSEMBLY__
typedef struct
{
	//general registers
	u32 regs[13];
	u32 hyp_lr;
	u32 hyp_spsr;
} cpu_t;
#endif /*__ASSEMBLY__ */

#endif
