#ifndef __ASM_VCPU_H
#define __ASM_VCPU_H

#include <asm/types.h>

#define CPUINFO_sizeof 168 /* sizeof(struct cpu_info) */

#ifndef __ASSEMBLY__
typedef struct
{
	//general registers
	u32 regs[13];
	// u32 r0;
	// u32 r1;
	// u32 r2;
	// u32 r3;
	// u32 r4;
	// u32 r5;
	// u32 r6;
	// u32 r7;
	// u32 r8;
	// u32 r9;
	// u32 r10;
	// u32 r11;
	// u32 r12;
	u32 hyp_lr;
	u32 hyp_spsr;
} cpu_t;
#endif /*__ASSEMBLY__ */

#endif
