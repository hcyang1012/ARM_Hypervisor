#include <asm/types.h>
#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/traps.h>
#include <asm/ept_violation.h>
#include <asm/debug.h>

#include <stdio.h>
extern uint32_t hyp_traps_vector[];
extern void guest_return(void);
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


void do_handler_dabt(void)
{
  printf("do_dabt\n");
  print_vcpu();
  while(1);
}

void do_handler_pabt(void)
{
  printf("do_pabt\n");
  print_vcpu();
  while(1);
}

void do_handler_undef(void)
{
  printf("do_undef\n");
  print_vcpu();
  while(1);
}

void do_handler_irq(void)
{
  printf("do_irq\n");
  print_vcpu();
  while(1);
}

void do_handler_fiq(void)
{
  printf("do_fiq\n");
  print_vcpu();
  while(1);
}

void do_handler_svc(void)
{
  printf("do_svc\n");
  print_vcpu();
  while(1);
}

extern void return_guest(void);
void do_handler_hvc(void)
{
  union hsr hsr = { .bits = READ_SYSREG32(ESR_EL2) };
  struct ept_violation_info_t ept_violation_info;
  // printf("do_hvc : 0x%x\n",hsr.ec);
  //print_vcpu();
  switch(hsr.ec)
  {
    case HSR_EC_INSTR_ABORT_LOWER_EL:   // EPT Violation - Prefetch Abort
      printf("Prefetch abort : %x\n",hsr.bits);
      ept_violation_info.hsr.bits = hsr.bits;
      ept_violation_info.reason = PREFETCH;
      ept_violation_info.gva = READ_CP32(HIFAR);
      gva_to_ipa(ept_violation_info.gva, &ept_violation_info.gpa);
      ept_violation_handler(&ept_violation_info);
      break;
    case HSR_EC_DATA_ABORT_LOWER_EL:    // EPT Violation - Data Abort
      printf("Data abort : %x\n",hsr.bits);
      ept_violation_info.hsr.bits = hsr.bits;
      ept_violation_info.reason = DABT;
      ept_violation_info.gva = READ_CP32(HDFAR);
      gva_to_ipa(ept_violation_info.gva, &ept_violation_info.gpa);
      ept_violation_handler(&ept_violation_info);
      break;
    default:
      printf("Unknown HVC Exception : %x\n",hsr.ec);
      break;
  }
}
