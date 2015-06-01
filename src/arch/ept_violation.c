#include <asm/traps.h>
#include <asm/types.h>
#include <asm/cpregs.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/ept_violation.h>
#include <asm/vcpu.h>
#include <config.h>
#include <stdio.h>

extern cpu_t vcpu;
extern lpae_t ept_L1[];

static inline void flush_tlb(void)
{
  unsigned long
  hcr = READ_SYSREG(HCR_EL2);
  WRITE_SYSREG(hcr & ~HCR_VM, HCR_EL2);
  isb();
  WRITE_SYSREG(hcr , HCR_EL2);
  isb();
}

void ept_violation_handler(struct ept_violation_info_t info)
{
  lpae_t *ept;
  unsigned long tmp;
  printf("EPT Violation : %s\n",info.reason == PREFETCH ? "prefetch" : "data");
  printf("PC : %x\n",vcpu.hyp_lr);
  printf("GVA : 0x%x\n",info.gva);
  printf("GPA : 0x%x\n",(unsigned long)info.gpa);
  ept = get_ept_entry(info.gpa);
  tmp = ept->bits & 0xFFFFFFFF;
  printf("EPT Entry : 0x%x(0x%x)\n",ept,tmp);
  printf("Enable EPT Access\n");
  //ept->bits |= 0xC0;
  ept->p2m.read = 1;
  ept->p2m.write = 1;
  isb();
  dsb();
  flush_tlb();
}

static inline uint64_t gva_to_ipa_par(vaddr_t va)
{
    uint64_t par, tmp;
    tmp = READ_CP64(PAR);
    WRITE_CP32(va, ATS1CPR);
    isb(); /* Ensure result is available. */
    par = READ_CP64(PAR);
    WRITE_CP64(tmp, PAR);
    return par;
}

int gva_to_ipa(vaddr_t va, paddr_t *paddr)
{
    uint64_t par = gva_to_ipa_par(va);
    if ( par & PAR_F )
        return -1;
    *paddr = (par & PADDR_MASK & PAGE_MASK) | ((unsigned long) va & ~PAGE_MASK);
    return 0;
}

extern lpae_t *ept_L2_root;
lpae_t *get_ept_entry(paddr_t gpa)
{
  unsigned long page_num;
  page_num = (gpa >> 21);
  return &ept_L2_root[page_num];
}
