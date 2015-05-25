#include <asm/page.h>
#include <asm/types.h>
#include <asm/processor.h>
#include <config.h>

lpae_t boot_pgtable[LPAE_ENTRIES] __attribute__((__aligned__(4096)));
unsigned char cpu0_stack[STACK_SIZE] __attribute__((__aligned__(STACK_SIZE)));
uint64_t init_ttbr;


unsigned long smp_up_cpu = MPIDR_INVALID;
