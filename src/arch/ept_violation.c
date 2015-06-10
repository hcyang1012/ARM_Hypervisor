#include <asm/traps.h>
#include <asm/types.h>
#include <asm/cpregs.h>
#include <asm/processor.h>
#include <asm/page.h>
#include <asm/ept_violation.h>
#include <asm/mmio.h>
#include <asm/vcpu.h>
#include <config.h>
#include <stdio.h>

extern cpu_t vcpu;
extern lpae_t ept_L1[];


void advance_pc(struct ept_violation_info_t *info)
{
  vcpu.hyp_lr += info->hsr.len ? 4 : 2;
}
static inline void flush_tlb(void)
{
  dsb(sy);

  WRITE_CP32((uint32_t) 0, TLBIALLNSNHIS);

  dsb(sy);
  isb();
}
void apply_ept(lpae_t *ept)
{
    isb();
    dsb();
    clean_and_invalidate_dcache_va_range(ept, PAGE_SIZE);
    isb();
    dsb();
    flush_tlb();
    isb();
    dsb();
}
void ept_violation_handler(struct ept_violation_info_t *info)
{
  lpae_t *ept;
  unsigned long tmp;
  
  printf("EPT Violation : %s\n",info->reason == PREFETCH ? "prefetch" : "data");
  printf("PC : %x\n",vcpu.hyp_lr);
  printf("GVA : 0x%x\n",info->gva);
  printf("GPA : 0x%x\n",(unsigned long)info->gpa);
  ept = get_ept_entry(info->gpa);
  tmp = ept->bits & 0xFFFFFFFF;
  printf("EPT Entry : 0x%x(0x%x)\n",ept,tmp);  
  if(handle_mmio(info))
  {

  }

  /* Do not delete following code block */
  /* A sample code for modifying EPT */
  /* After modifying EPT, we must flush both cache and TLB */
  // {
  //   // isb();
  //   // dsb();
  //   // WRITE_SYSREG(hcr & ~HCR_VM, HCR_EL2);
  //   // isb();
  //   // dsb();
  //   // ept = get_ept_entry(info.gpa);
  //   // tmp = ept->bits & 0xFFFFFFFF;
  //   // printf("EPT Entry : 0x%x(0x%x)\n",ept,tmp);
  //   // printf("Enable EPT Access\n");
  //   // ept->p2m.read = 1;
  //   // ept->p2m.write = 1;
  //   // isb();
  //   // dsb(sy);
  //   // WRITE_SYSREG(hcr | HCR_VM, HCR_EL2);
  //   // isb();
  //   // dsb(sy);
  //   // clean_and_invalidate_dcache_va_range(ept, PAGE_SIZE);
  //   // flush_tlb();
  // }
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

extern lpae_t *ept_L3_root;
lpae_t *get_ept_entry(paddr_t gpa)
{
  unsigned long page_num;
  page_num = (gpa >> 12);
  return &ept_L3_root[page_num];
}
