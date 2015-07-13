#include <asm/ept_violation.h>
#include <asm/types.h>
#include <asm/page.h>
#include <stdio.h>
#include <asm/spinlock.h>
#include <asm/mmio.h>

/* If gpa is for mmio, return 1, else: return 0 */
extern cpu_t vcpu;

int handle_mmio(struct ept_violation_info_t *info)
{
  paddr_t gpa = info->gpa;
  if (ODRID_BASE <= gpa && gpa <= (ODRID_BASE + 4096))
  {
    if(info->hsr.dabt.write)
    {

    }
    else{
      unsigned long regNum;
      volatile register_t *r;
      volatile void *buf;
      volatile unsigned long *src;
      volatile unsigned long len;
      volatile unsigned long dat;
      spin_lock(&vcpu.lock);

      regNum = info->hsr.dabt.reg;
      r = (register_t*)select_user_reg(regNum);
      len = 1<<(info->hsr.dabt.size & 0x00000003);
      buf = (void*)r;
      src = (unsigned long*)(unsigned long)gpa;
      printf("(%d bytes)Read from 0x%x to R%d\n",(unsigned long)len, (unsigned long)gpa, (unsigned long)regNum);
      dat = *src;

      // if((unsigned char)(gpa & 0xF) == 0x4)
      // {
      //   printf("Data read, change data from 0x%x to 0x%x \n",(unsigned long)*src, ~(*src));
      //   dat = ~(*src);
      // }
      // else
      // {
      //   dat = (*src);
      // }

      if(regNum != 14)
      {
        *(unsigned long*)buf = dat;
      }
      else
      {
        __asm__ __volatile__(
          "msr lr_svc,%0 \r\n"
          : /* Output */
          : "r"(dat) /* Input */
          : /* Clobber */
          );
      }
      dsb();
      isb();
      spin_unlock(&vcpu.lock);
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
  // lpae_t *tpm_entry=get_ept_entry((paddr_t)TPM_MEM_BASE);
  // tpm_entry->p2m.read = 0;
  // tpm_entry->p2m.write = 0;
  // apply_ept(tpm_entry);
  // printf("TPM Entry : 0x%x\n",(unsigned long)tpm_entry->bits);

  //OdroidXU-AVR
  lpae_t *avr_entry=get_ept_entry((paddr_t)ODRID_BASE);
  avr_entry->p2m.read = 0;
  avr_entry->p2m.write = 1;
  apply_ept(avr_entry);
  printf("GPIO Pin Disabled\n");
}
