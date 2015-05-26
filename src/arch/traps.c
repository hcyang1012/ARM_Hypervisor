#include <asm/types.h>
#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/traps.h>

#include <stdio.h>
extern uint32_t hyp_traps_vector[];
void init_traps(void)
{
  /* Setup Hyp vector base */
  printf("Write trap : %x\n",hyp_traps_vector);
  WRITE_SYSREG((vaddr_t)hyp_traps_vector, VBAR_EL2);

  /* Setup hypervisor traps */
  // WRITE_SYSREG(HCR_PTW|HCR_BSU_INNER|HCR_AMO|HCR_IMO|HCR_FMO|HCR_VM|
  //              HCR_TWE|HCR_TWI|HCR_TSC|HCR_TAC|HCR_SWIO|HCR_TIDCP, HCR_EL2);
  isb();
}


void do_dabt_handler(void)
{
  printf("do_dabt\n");
}

void do_pabt_handler(void)
{
  printf("do_pabt\n");
}

void do_undef_handler(void)
{
  printf("do_undef\n");
}

void do_hvc_handler(void)
{
  union hsr hsr = { .bits = READ_SYSREG32(ESR_EL2) };
  printf("do_hvc : 0x%x\n",hsr.ec);
  switch(hsr.ec)
  {
    case HSR_EC_INSTR_ABORT_LOWER_EL:   //Prefetch Abort
      printf("Prefetch Abort : %x\n",hsr.bits);
      break;
    case HSR_EC_DATA_ABORT_LOWER_EL:
      printf("Data abort : %x\n",hsr.bits);
      break;
  }
  while(1);
}
