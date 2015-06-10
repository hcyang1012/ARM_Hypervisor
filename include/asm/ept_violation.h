#ifndef __ASM_EPT_VIOLATION_H
#define __ASM_EPT_VIOLATION_H
#include <asm/traps.h>
#include <asm/types.h>
#include <asm/page.h>
#include <asm/vcpu.h>
void ept_violation_handler(struct ept_violation_info_t *info);
int gva_to_ipa(vaddr_t va, paddr_t *paddr);
lpae_t *get_ept_entry(paddr_t gpa);
void advance_pc(struct ept_violation_info_t *info);
void apply_ept(lpae_t *ept);

#define select_user_reg(reg) (&(vcpu.regs[reg]))
#endif
