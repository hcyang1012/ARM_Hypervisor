#include <asm/ept_violation.h>
#include <asm/types.h>
#include <swtpm/swtpm.h>
#include <stdio.h>

int handle_mmio(struct ept_violation_info_t *info)
{
  paddr_t gpa = info->gpa;
  //SWTPM
  if(TPM_MEM_BASE <= gpa && gpa <= (TPM_MEM_BASE + TPM_MEM_LEN))
  {
    return 1;
  }

  return 0;
}
