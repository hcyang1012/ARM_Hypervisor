#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/types.h>
#include <asm/page.h>
#include <asm/types.h>
#include <asm/guest_init.h>
#include <asm/mmio.h>
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


/* Return the cache property of the input gpa */
/* It is determined depending on whether the address is for device or memory */
static bool isInMemory(unsigned long gpa)
{
  if(RAM_START <= gpa && gpa <= RAM_END)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

extern lpae_t ept_L1[];
lpae_t *ept_L2_root;
//lpae_t *ept_L3_root;
void guest_ept_init(void)
{
  int index_l1, index_l2;
  unsigned long gpa = 0;
  unsigned long vttbr_val = (unsigned long)ept_L1;
  unsigned long hcr;
  
  /* Calculate next level page table address */
  ept_L2_root = (lpae_t*) (((unsigned long)ept_L1 | (1 << 12)) & ~0xFFF);
   
  printf("EPT root address : 0x%x\n",ept_L1);
  printf("ept_L2_root : 0x%x\n",ept_L2_root);

  for(index_l1 = 0 ; index_l1 < LPAE_L1_SIZE ; index_l1++)
  {
    lpae_t entry_l1;
    lpae_t *ept_l2 = ept_L2_root + LPAE_ENTRIES * index_l1;
    
    /* Set first level page table entries */
    entry_l1.bits = 0;
    entry_l1.p2m.valid = 1;
    entry_l1.p2m.table = 1;
    entry_l1.bits |= (unsigned long)ept_l2;
    ept_L1[index_l1].bits = entry_l1.bits;
    printf("ept_L1[%d] : 0x%x (At 0x%x)\n",index_l1,ept_L1[index_l1].bits,&ept_L1[index_l1]);
    
    for(index_l2 = 0 ; index_l2 < LPAE_ENTRIES ; index_l2++)
    {
      lpae_t entry_l2;
      /* Set second level page table entries */
      /* 2MB Page */
      entry_l2.bits = 0;
      entry_l2.p2m.valid = 1;
      entry_l2.p2m.table = 0;
      entry_l2.p2m.af = 1;
      entry_l2.p2m.read = 1;
      entry_l2.p2m.write = 1;
      
      if(isInMemory(gpa))
      {
        /* RAM area */
        entry_l2.p2m.mattr = 0xF; /* 1111b: Outer Write-back Cacheable / Inner write-back cacheable */
      }
      else
      {
        /* Device area */
        entry_l2.p2m.mattr = 0x1; /* 0001b: Device Memory */
        entry_l2.p2m.sh = 0x0;
        entry_l2.p2m.xn = 1;
      }
      entry_l2.bits |= gpa;
      
      gpa += (2 * 1024 * 1024); /* 2MB section */
      
      ept_l2[index_l2].bits = entry_l2.bits;
      printf("\tept_l2[%d] : 0x%x (At 0x%x)\n",index_l2, ept_l2[index_l2].bits,&ept_l2[index_l2]);
 
    }
  }
    
  //   lpae_t e;
  //   lpae_t *ept_L2 = (&guest_ept_L1[LPAE_L1_SIZE] + LPAE_ENTRIES * index_l1);
  //   e.bits = 0x3; //Valid & Page Table
  //   e.bits |= (unsigned long)ept_L2;
  //   guest_ept_L1[index_l1] = e;
  //   printf("guest_ept_L1[%d] : 0x%x\n",index_l1, guest_ept_L1[index_l1]);
  //   for(index_l2 = 0 ; index_l2 < LPAE_ENTRIES ; index_l2++)
  //   {
  //     lpae_t e;
  //     e.bits = 0
  //     e.p2m.sh = 0x3;
  //     e.p2m.table = 0;
  //     e.p2m.valid = 1;
  //     e.p2m.af = 1;
  //     if(isInMemory(gpa))
  //     {
  //       /* Set attibutes for RAM area */
  //       e.p2m.mattr = 0xF; /* 1111b: Outer Write-back Cacheable / Inner write-back cacheable */
  //       e.p2m.read = 1;
  //       e.p2m.write = 1;
  //       // e.p2m.read = 0;
  //       // e.p2m.write = 0;
  //       e.p2m.xn = 0;
  //     }
  //     else
  //     {
  //       /* Set attributes for Device area */
  //       e.p2m.mattr = 0x1; /* 0001b: Device memory */
  //       e.p2m.read = 1;
  //       e.p2m.write = 1;
  //       e.p2m.xn = 1;
  //       e.p2m.sh = 0x0;
  //     }

  //     //e.bits = 0x7FD; /* Read / Write OK */
  //     //e.bits = 0x73D; /* No Acess permission */
  //     e.bits |= gpa;
  //     ept_L2[index_l2] = e;
  //     gpa += (1024*1024*2);
  //     printf("ept_L2[%d] : 0x%x (at 0x%x)\n",index_l2,ept_L2[index_l2],(unsigned long)&(ept_L2[index_l2]));
  //   }
  // }
  dsb();
  isb();
  //init_mmio();
  dsb();
  isb();
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
