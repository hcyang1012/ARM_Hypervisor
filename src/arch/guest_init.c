#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/types.h>
#include <asm/page.h>
#include <asm/types.h>
#include <asm/guest_init.h>
#include <asm/ept_violation.h>
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
  if(RAM_START <= gpa && gpa < RAM_END)
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
lpae_t *ept_L3_root;
void guest_ept_init(void)
{
  int index_l1, index_l2, index_l3;
  unsigned long long  gpa = 0;
  unsigned long vttbr_val = (unsigned long)ept_L1;
  unsigned long hcr;
  
  /* Calculate next level page table address */
  /* Level 2 */
  ept_L2_root = &ept_L1[LPAE_L1_SIZE];
  ept_L2_root = (lpae_t*) (((unsigned long)ept_L2_root + (1 << 12)) & ~0xFFF); /* Align */  
  //ept_L2_root = (lpae_t*) (((unsigned long)ept_L1 | (1 << 12)) & ~0xFFF);
  /* Level 3 */
  ept_L3_root = &ept_L2_root[LPAE_L2_SIZE];
  //ept_L3_root = (lpae_t*) (((unsigned long)ept_L3_root + (1 << 12)) & ~0xFFF); /* Align */
  
   
  printf("EPT root address : 0x%x\n",ept_L1);
  printf("ept_L2_root : 0x%x\n",ept_L2_root);
  printf("ept_L3_root : 0x%x\n",ept_L3_root);
  printf("LPAE_L1_SIZE : %d\n",LPAE_L1_SIZE);
  printf("LPAE_L2_SIZE : %d\n",LPAE_L2_SIZE);
  printf("LPAE_L3_SIZE : %d\n",LPAE_L3_SIZE);
  
  for(index_l1 = 0 ; index_l1 < LPAE_L1_SIZE ; index_l1++)
  {
    lpae_t entry_l1;
    lpae_t *ept_l2 = &ept_L2_root[LPAE_ENTRIES * index_l1];
    
    /* Set first level page table entries */
    entry_l1.bits = 0;
    entry_l1.p2m.valid = 1;
    entry_l1.p2m.table = 1;
    entry_l1.bits |= (unsigned long)ept_l2;
    ept_L1[index_l1].bits = entry_l1.bits;
    for(index_l2 = 0 ; index_l2 < LPAE_ENTRIES ; index_l2++)
    {
      lpae_t entry_l2;
      lpae_t *ept_l3 = &ept_L3_root[LPAE_ENTRIES * LPAE_ENTRIES * index_l1  + LPAE_ENTRIES * index_l2];
      //printf("(EPT_L3)0x%x - %d/%d (%d)\n",ept_l3, index_l1,index_l2, LPAE_L2_SIZE * LPAE_ENTRIES * index_l1  + LPAE_ENTRIES * index_l2);
    
      /* Set second level page table entries */
      entry_l2.bits = 0;
      entry_l2.p2m.valid = 1;
      entry_l2.p2m.table = 1;
      entry_l2.bits |= (unsigned long)ept_l3;
      ept_l2[index_l2].bits = entry_l2.bits;
      
      for(index_l3 = 0 ; index_l3 < LPAE_ENTRIES ; index_l3++)
      {
        lpae_t entry_l3;
        /* Set third level page table entries */
        /* 4KB Page */
        entry_l3.bits = 0;
        entry_l3.p2m.valid = 1;
        entry_l3.p2m.table = 1;
        entry_l3.p2m.af = 1;
        entry_l3.p2m.read = 0;
        entry_l3.p2m.write = 0;
        entry_l3.p2m.mattr = 0xF;
        entry_l3.p2m.sh = 0x03;
        entry_l3.p2m.xn = 0x0;
            
        if(isInMemory(gpa))
        {
          /* RAM area */
          entry_l3.p2m.sh = 0x03;
          entry_l3.p2m.mattr = 0xF; /* 1111b: Outer Write-back Cacheable / Inner write-back cacheable */
          entry_l3.p2m.read = 1;
          entry_l3.p2m.write = 1;            
        }
        else
        {
          /* Device area */
          if(isMMIO(gpa))
          {
            printf("Write MMIO Area : 0x%x\n",(unsigned long)gpa);
            entry_l3.p2m.read = 0;
            entry_l3.p2m.write = 0;            
          }
          else
          {
            entry_l3.p2m.read = 1;
            entry_l3.p2m.write = 1;            
          }
          entry_l3.p2m.mattr = 0x1; /* 0001b: Device Memory */
          entry_l3.p2m.sh = 0x0;
          entry_l3.p2m.xn = 1;   
        }
        entry_l3.bits |= gpa;
        ept_l3[index_l3].bits = entry_l3.bits;
        // {
        //   /* For logging.. */
        //   lpae_t *pept;
        //   pept = get_ept_entry(gpa);
        //   if(pept != &ept_l3[index_l3])
        //   {
        //     printf("(Index)%d/%d/%d - ", index_l1,index_l2,index_l3);
        //     printf("(L1)0x%x - ",(unsigned long)entry_l1.bits);
        //     printf("(L2Adr)0x%x - ",(unsigned long)&ept_l2[index_l2]);
        //     printf("(L2)0x%x - ",(unsigned long)entry_l2.bits);
        //     printf("(L3Adr)0x%x - ",(unsigned long)&ept_l3[index_l3]);
        //     printf("(L3)0x%x - ",(unsigned long)entry_l3.bits);
        //     printf("(GPA)0x%x - ",(unsigned long)gpa);            
        //     printf("Error - ");
        //     printf("(EPT)0x%x - ",ept_l3);
        //     printf("(PAddr)0x%x - (PVAL)0x%x\n",pept,(unsigned long)pept->bits);            
        //   }            
        // }
        gpa += (4*1024); /* 4KB page frame */
      }
      apply_ept(ept_l3);
    }
    apply_ept(ept_l2);
  }
  apply_ept(ept_L1);
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
    //hcr = READ_SYSREG(HCR_EL2);
    hcr = 0x0;
    WRITE_SYSREG(hcr | HCR_BSU_INNER | HCR_SWIO | HCR_TGE| HCR_VM, HCR_EL2);
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
