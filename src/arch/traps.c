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

/* This function emulates TakeSVCException() function which is described in the ARM Architecture Reference Manual */
extern cpu_t vcpu;
static void guest_svc(void)
{
  unsigned long sctlr_val;
  unsigned long vbar_valid;
  unsigned long sctlr_te, sctlr_ee;
  unsigned long guest_exception_vector_addr;
  unsigned long guest_svc_return_addr;
  const unsigned long vect_offset = 8;
  
  /* Read exception vector address of guest VM */
  /*
      SCTLR.V : SCTLR[13]
      the Non-secure SCTLR.V bit determines the Exception base address:
      V == 0 The Non-secure VBAR holds the Exception base address.
      V == 1 Exception base address = 0xFFFF0000, the Hivecs setting.
  */
  sctlr_val = READ_SYSREG32(SCTLR_EL1);
  vbar_valid = sctlr_val & (1 << 13);
  
  printf("SCTLR : 0x%x\n",(unsigned long)sctlr_val);

  // R[14] = new_lr_value;
  guest_svc_return_addr = vcpu.hyp_lr;  
  __asm__ __volatile__(
    "msr lr_svc,%0 \r\n"
    : /* Output */
    : "r"(guest_svc_return_addr) /* Input */
    : /* Clobber */
    );  
    
  // new_spsr_value = CPSR;
  // SPSR[] = new_spsr_value;
  printf("SPSR : 0x%x\n",(unsigned long)vcpu.hyp_spsr);
  __asm__ __volatile__(
    "msr SPSR_svc,%0 \r\n"
    : /* Output */
    : "r"(vcpu.hyp_spsr) /* Input */
    : /* Clobber */
    );    

  if(vbar_valid)
  {
    guest_exception_vector_addr = 0xFFFF0000;
  }
  else
  {
    guest_exception_vector_addr = READ_SYSREG32(VBAR_EL1);
    
  }
  printf("VBAR : 0x%x\n", (unsigned long)guest_exception_vector_addr);
  vcpu.hyp_lr = guest_exception_vector_addr + vect_offset;
  printf("vcpu.hyp_lr : 0x%x\n", (unsigned long)vcpu.hyp_lr);
  
  // CPSR.M = '10011';
  vcpu.hyp_spsr &= (~(0x1F));
  vcpu.hyp_spsr |= Mode_SVC; 
        
  // CPSR.I = '1';
  vcpu.hyp_spsr |= (1 << 7);
 
  // CPSR.IT = '00000000';
  vcpu.hyp_spsr &= 0xF9FF03FF;
   
  // CPSR.J = '0'; CPSR.T = SCTLR.TE; // TE=0: ARM, TE=1: Thumb
  vcpu.hyp_spsr &= (~(1 << 24));
  sctlr_te = sctlr_val & (1 << 30);
  if(sctlr_te)
  {
    printf("TE\n");
    vcpu.hyp_spsr |= (1 << 5);
  }
  else
  {
    vcpu.hyp_spsr &= (~(1 << 5));
  }
  
  // CPSR.E = SCTLR.EE; // EE=0: little-endian, EE=1: big-endian
  sctlr_ee = sctlr_val & (1 << 25);
  if(sctlr_ee)
  {
    printf("EE\n");
    vcpu.hyp_spsr |= (1 << 9);
  }
  else
  {
    vcpu.hyp_spsr &= (~(1 << 9));
  }
  printf("Write CPSR : 0x%x\n",(unsigned long)vcpu.hyp_spsr);
}

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
      ept_violation_info.hsr.bits = hsr.bits;
      ept_violation_info.reason = DABT;
      ept_violation_info.gva = READ_CP32(HDFAR);
      gva_to_ipa(ept_violation_info.gva, &ept_violation_info.gpa);
      ept_violation_handler(&ept_violation_info);
      break;
    case HSR_EC_SVC32:
      printf("SVC Instruction called : %x\n",hsr.ec);
      guest_svc();
      break;
    default:
      printf("Unknown HVC Exception : %x\n",hsr.ec);
      break;
  }
}
