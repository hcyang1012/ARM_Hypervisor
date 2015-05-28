#include <config.h>
#include <asm/vcpu.h>

#define save_guest_context                        \
	push {r12};                                       \
	ldr r12, =vcpu;                                   \
	stmia r12!, {r0-r11};                             \
	pop {r0};                                         \
	mrs r10, elr_hyp;																	\
	mrs	r11, spsr_hyp;																	\
	stmia r12!, {r0, r10, r11};                             \

#define restore_guest_context                     \
	ldr r12, =vcpu;                                   \
	ldmia r12!, {r0-r11};                             \
	push {r0, r10, r11};                                        \
	mov r0, r12;                                      \
	ldmia r0!, {r12};                             \
	ldmia r0!, {r10, r11};                             \
	msr elr_hyp, r10;																	\
	msr	spsr_hyp, r11;																	\
	pop {r0, r10, r11};                                         \

#define DEFINE_TRAP_ENTRY(trap)                                         \
        ALIGN;                                                          \
do_##trap:                                                            \
cpsid i;                                                         \
save_guest_context;                                              \
bl do_handler_##trap;                                            \
restore_guest_context;																					\
cpsie i;																										\
eret;

.align 5
GLOBAL(hyp_traps_vector)
  .word 0                         /* 0x00 - Reset */
  b do_undef    /* 0x04 - Undefined Instruction */
  b do_svc          /* 0x08 - Supervisor Call */
  b do_pabt           /* 0x0c - Prefetch Abort */
  b do_dabt               /* 0x10 - Data Abort */
  b do_hvc               /* 0x14 - Hypervisor */
  b do_irq                      /* 0x18 - IRQ */
  b do_fiq                      /* 0x1c - FIQ */


DEFINE_TRAP_ENTRY(undef)
DEFINE_TRAP_ENTRY(svc)
DEFINE_TRAP_ENTRY(pabt)
DEFINE_TRAP_ENTRY(dabt)
DEFINE_TRAP_ENTRY(hvc)
DEFINE_TRAP_ENTRY(irq)
DEFINE_TRAP_ENTRY(fiq)
