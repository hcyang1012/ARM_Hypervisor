#include <asm/ept_violation.h>
#include <asm/types.h>
#include <asm/page.h>
#include <swtpm/swtpm.h>
#include <stdio.h>

/* If gpa is for mmio, return 1, else: return 0 */
int handle_mmio(struct ept_violation_info_t *info)
{
  paddr_t gpa = info->gpa;
  //SWTPM
  if(TPM_MEM_BASE <= gpa && gpa < (TPM_MEM_BASE + TPM_MEM_LEN))
  {
    if(info->hsr.dabt.write)
    {
      tpm_mmio_write(info);
    }
    else /* Read */
    {
      tpm_mmio_read(info);
    }
    advance_pc(info);
    return 1;
  }

  return 0;
}

/* Add mmio functions */
void init_mmio(void)
{
  //SWTPM
  lpae_t *tpm_entry=get_ept_entry((paddr_t)TPM_MEM_BASE);
  tpm_entry->p2m.read = 0;
  tpm_entry->p2m.write = 0;
  apply_ept(tpm_entry);
  printf("TPM Entry : 0x%x\n",(unsigned long)tpm_entry->bits);
}
