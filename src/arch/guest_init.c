#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/types.h>
#include <asm/page.h>
#include <asm/types.h>
#include <asm/guest_init.h>
#include <string.h>
#include <config.h>
#include <stdio.h>

void vctr_write(unsigned long val)
{
  WRITE_SYSREG32(val, VTCR_EL2);
  isb();
}

void vctr_init(void)
{
  unsigned long vctr_val = VTCR_RES1|VTCR_SH0_IS|VTCR_ORGN0_WBWA|VTCR_IRGN0_WBWA;
  vctr_val |= VTCR_T0SZ(0x18); /* 40 bit IPA */
  vctr_val |= VTCR_SL0(0x1); /* P2M starts at first level */
  vctr_write(vctr_val);
}


static void guest_trap_init(void)
{
  unsigned long hcr;
  hcr = READ_SYSREG(HCR_EL2);
  // WRITE_SYSREG(hcr | HCR_TGE, HCR_EL2);
  // hcr = READ_SYSREG(HCR_EL2);
  printf("HCR : 0x%x\n",hcr);
  isb();
}

void guest_init(void)
{
  vctr_init();
  guest_ept_init();
  guest_trap_init();
  // copy_guest();
  // copy_dtb();
}

extern lpae_t ept_L1[];
lpae_t *ept_L2_root;
void guest_ept_init(void)
{
  int index_l1, index_l2;
  unsigned long gpa = 0;
  lpae_t *guest_ept_L1 = ept_L1;
  unsigned long vttbr_val = (unsigned long)guest_ept_L1;
  unsigned long hcr;
  printf("vttbr_val : %x\n",vttbr_val);
  ept_L2_root = &guest_ept_L1[LPAE_ENTRIES];
  for(index_l1 = 0 ; index_l1 < LPAE_ENTRIES ; index_l1++)
  {
    // lpae_t e;
    // e.bits = 0x4D5;
    // e.bits |= gpa;
    // guest_ept_L1[index_l1] = e;
    // gpa += 0x40000000;

    lpae_t e;
    lpae_t *ept_L2 = (&guest_ept_L1[LPAE_ENTRIES] + LPAE_ENTRIES * index_l1);
    e.bits = 0x3;
    e.bits |= (unsigned long)ept_L2;
    guest_ept_L1[index_l1] = e;
    for(index_l2 = 0 ; index_l2 < LPAE_ENTRIES ; index_l2++)
    {
      lpae_t e;
      e.bits = 0x7FD; /* Read / Write OK */
      //e.bits = 0x73D; /* No Acess permission */
      e.bits |= gpa;
      ept_L2[index_l2] = e;
      gpa += (1024*1024*2);

    }
  }
    // Write EPT to VTTBR
    //WRITE_SYSREG64(vttbr_val,VTTBR_EL2);
    asm volatile(
  		"mov     r1, #0x0\n\t"                    // ; Set high word of physical address
  		"MCRR    p15, 6, %0, r1, c2\n\t"          // ; Write VTTBR
  		:
  		: "r"(vttbr_val)
  		: "r1"
  		);

    // Turn on Stage 2 Address Translation
    hcr = READ_SYSREG(HCR_EL2);
    WRITE_SYSREG(hcr | HCR_PTW | HCR_VM, HCR_EL2);
    isb();
}

void copy_guest(void)
{
  unsigned long *from = (unsigned long*)KERNEL_ADDR;
  unsigned long *to = (unsigned long*)KERNEL_START;
  printf("Copy guest kernel image from %x to %x (%d bytes): 0x%x / 0x%x\n",from,to,KERNEL_SIZE,from[0], from[1]);
  memcpy(to,from,KERNEL_SIZE);
  printf("Copy end : 0x%x / 0x%x\n",to[0], to[1]);
}


void copy_dtb(void)
{
  unsigned long *from = (unsigned long*)DTB_ADDR;
  unsigned long *to = (unsigned long*)DTB_START;
  printf("Copy DTB from %x to %x (%d bytes): 0x%x / 0x%x\n",from,to,DTB_SIZE,from[0], from[1]);
  memcpy(to,from,DTB_SIZE);
  printf("Copy end : 0x%x / 0x%x\n",to[0], to[1]);
}


extern void guest_start(unsigned long addr);
void guest_boot(void)
{
  printf("Jump to %x\n",KERNEL_START);
  guest_start(KERNEL_START);
}
