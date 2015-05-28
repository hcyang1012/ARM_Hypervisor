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
  lpae_t *guest_ept_L1 = ept_L1;
  unsigned long vttbr_val = 0;
  unsigned long hcr;
  hcr = READ_SYSREG(HCR_EL2);
  WRITE_SYSREG(hcr & ~HCR_VM, HCR_EL2);
  isb();

  asm volatile(
    "mov     r1, #0x0\n\t"                    // ; Set high word of physical address
    "MCRR    p15, 6, %0, r1, c2\n\t"          // ; Write VTTBR
    :
    : "r"(vttbr_val)
    : "r1"
    );
  isb();

  vttbr_val = (unsigned long)guest_ept_L1;
  asm volatile(
    "mov     r1, #0x0\n\t"                    // ; Set high word of physical address
    "MCRR    p15, 6, %0, r1, c2\n\t"          // ; Write VTTBR
    :
    : "r"(vttbr_val)
    : "r1"
    );
  isb();
  WRITE_SYSREG(hcr, HCR_EL2);
  isb();
}

void ept_violation_handler(struct ept_violation_info_t info)
{
  lpae_t *ept;
  unsigned long tmp;
  flush_tlb();
  printf("EPT Violation : %s\n",info.reason == PREFETCH ? "prefetch" : "data");
  printf("PC : %x\n",vcpu.hyp_lr);
  printf("GVA : 0x%x\n",info.gva);
  printf("GPA : 0x%x\n",(unsigned long)info.gpa);
  ept = get_ept_entry(info.gpa);
  tmp = ept->bits & 0xFFFFFFFF;
  printf("EPT Entry : 0x%x(0x%x)\n",ept,tmp);
  printf("Enable EPT Access\n");
  ept->bits |= 0xC0;
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
  unsigned long tmp;
  // printf("EPT_L2 address : %x\n",ept_L2_root);
  tmp = (unsigned long)gpa & 0xFFFFFFFF;
  // printf("GPA : %x\n",tmp);
  page_num = (gpa >> 20) & 0xFFF;
  page_num /= 2;
  // printf("Page num : %x\n",page_num);
  // printf("EPT Value1 : 0x%x\n",(unsigned long)ept_L2_root[page_num].bits);
  return &ept_L2_root[page_num];
}
