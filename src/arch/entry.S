#include <config.h>
#include <asm/vcpu.h>

GLOBAL(hyp_traps_vector)
  .word 0                         /* 0x00 - Reset */
  b do_undef    /* 0x04 - Undefined Instruction */
  b .          /* 0x08 - Supervisor Call */
  b do_pabt           /* 0x0c - Prefetch Abort */
  b do_dabt               /* 0x10 - Data Abort */
  b do_hvc               /* 0x14 - Hypervisor */
  b .                      /* 0x18 - IRQ */
  b .                      /* 0x1c - FIQ */

do_undef:
  bl do_undef_handler
  b .
do_pabt:
  bl do_pabt_handler
  b .
do_dabt:
  bl do_dabt_handler
  b .
do_hvc:
//  ldr   sp, =cpu0_stack
//  add   sp, #STACK_SIZE        /* (which grows down from the top). */
//  sub   sp, #CPUINFO_sizeof    /* Make room for CPU save record */
  bl do_hvc_handler
  b .

.align 14
.globl stage2_page_table
stage2_page_table:
	.quad 0x0000000000000461
	.quad 0x0000000040000461
	.quad 0x0000000080000461
	.quad 0x00000000C0000461
