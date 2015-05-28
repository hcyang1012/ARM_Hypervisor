#include <asm/page.h>
#include <asm/types.h>
#include <asm/vcpu.h>
#include <asm/processor.h>
#include <config.h>
#include <stdio.h>
//lpae_t guest_ept_L1[LPAE_ENTRIES]                               __attribute__((__aligned__(4096)));
// lpae_t guest_ept_L2[LPAE_ENTRIES][LPAE_ENTRIES]                 __attribute__((__aligned__(4096)));
// lpae_t guest_ept_L3[LPAE_ENTRIES][LPAE_ENTRIES][LPAE_ENTRIES]   __attribute__((__aligned__(4096)));
extern cpu_t vcpu;

void print_vcpu(void)
{
  printf("R0 : %x\n", vcpu.r0);
  printf("R1 : %x\n", vcpu.r1);
  printf("R2 : %x\n", vcpu.r2);
  printf("R3 : %x\n", vcpu.r3);
  printf("R4 : %x\n", vcpu.r4);
  printf("R5 : %x\n", vcpu.r5);
  printf("R6 : %x\n", vcpu.r6);
  printf("R7 : %x\n", vcpu.r7);
  printf("R8 : %x\n", vcpu.r8);
  printf("R9 : %x\n", vcpu.r9);
  printf("R10 : %x\n", vcpu.r10);
  printf("R11 : %x\n", vcpu.r11);
  printf("R12 : %x\n", vcpu.r12);
  printf("SPSR : %x\n", vcpu.hyp_spsr);
  printf("LR : %x\n", vcpu.hyp_lr);
}
