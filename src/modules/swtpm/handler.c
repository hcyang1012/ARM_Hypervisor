
#ifdef CONFIG_XEN
#include <xen/lib.h>
#include <xen/types.h>
#include <xen/sched.h>
#include <xen/domain_page.h>
#endif

#ifdef THIN_HYP
#include <asm/types.h>
#include <asm/ept_violation.h>
#include <stdio.h>
#include <string.h>
#include <asm/traps.h>
#endif

#include <swtpm/tcg.h>
#include <swtpm/swtpm.h>
#include <swtpm/init.h>
#include <swtpm/tpm_emulator.h>
unsigned int sts_cnt = 0;
unsigned int acc_cnt = 0;
unsigned int vmexit_cnt = 0;
u8 cmd_buf[TPM_CMD_SIZE_MAX];
u8 rsp_buf[TPM_RSP_SIZE_MAX];
u32 cmd_offset;
u32 rsp_offset;
u32 current_rsp_offset;
//unsigned int fifo_flag = 0;
//unsigned int sts_flag = 0;

typedef union tpm_input_header {
  u8 _raw[10];
  struct __attribute__ ((packed)) {
    u32 ordinal;
    u32 len;
    u16 tag;
  };
  /*struct __attribute__ ((packed)) {
  u8 ordinal3 : 1;
  u8 ordinal2 : 1;
  u8 ordinal1 : 1;
  u8 ordinal0 : 1; // u32 ordinal
  u8 len3 : 1;
  u8 len2 : 1;
  u8 len1 : 1;
  u8 len0 : 1; // u32 len
  u8 tag1 : 1;
  u8 tag0 : 1; // u16 tag
};*/
} tpm_input_header_t;; // 10 bytes
tpm_input_header_t tpm_input_header;
u32 input_header_offset;

// XXX: support only locality 0 in this time.. needs to expend other localities
tpm_reg_access_t tpm_reg_access[4];
tpm_reg_int_enable_t tpm_reg_int_enable[4];
tpm_reg_sts_t tpm_reg_sts[4];
tpm_reg_data_fifo_t tpm_reg_data_fifo[4];
tpm_reg_rid_t tpm_reg_rid[4];
tpm_reg_did_vid_t tpm_reg_did_vid[4];
tpm_reg_intf_cap_t tpm_reg_intf_cap[4];
tpm_reg_int_vec_t tpm_reg_int_vec[4];

#define reverse_copy(out, in, count) _reverse_copy((u8 *)(out), (u8 *)(in), count)
static inline void _reverse_copy (u8 *out, u8 *in, u32 count)
{
  u32 i;
  for (i = 0; i < count; i++)
  out[i] = in[count-i-1];
}

//int integrate_swtpm (void)
/*void integrate_swtpm (pushad_regs_t regs, u16 es, u16 ds, u16 FLAGS)
{
switch (regs.u.r8.ah)
{
case 0xbb:
// all functions except for TCG_StatusCheck need to have the TCPA in 'ebx'
if (regs.u.r8.al != 0 &&
regs.u.r32.ebx != TCPA)
{
//SET_CF();
//return;
break;
}
switch (regs.u.r8.al)
{
case 0x00: // StatusCheck
regs.u.r32.eax = 0x0;
regs.u.r32.ebx = TCPA;
regs.u.r32.ecx = 0x10102; // fake version 1.2
regs.u.r32.edx = 0x0;
regs.u.r32.esi = 0x0; // XXX
regs.u.r32.edi = 0x0; // XXX
CLEAR_CF();
break;
case 0x01:
case 0x02:
case 0x03:
case 0x04:
case 0x05:
case 0x06:
case 0x07:
CLEAR_CF();
break;

default:
SET_CF();
break;
}
default:
SET_CF();
break;
}

return;
}*/

/*static void print_cmd (void *prtptr, int size)
{
int i;
for (i = 0; i < size; i++)
printf("%02x ", *(unsigned char *)prtptr++);
printf("\n");
}*/

#if defined(CONFIG_XEN)
int tpm_mmio_read(struct vcpu *v, mmio_info_t *info)
{
  paddr_t gphys = info->gpa;
  struct hsr_dabt dabt = info->dabt;
  struct cpu_user_regs *regs = guest_cpu_user_regs();
  register_t *r = select_user_reg(regs, dabt.reg);
  void *buf = (void*)r;
  unsigned long len = 1<<(dabt.size & 0x00000003);
  //unsigned long len = dabt.size;
  u8 *p, x;
  int i;

  x = 0x00;
  //p = mapmem_hphys (TPM_FAKE_MEM_BASE, len, (wr ? MAPMEM_WRITE : 0) | flags);
  //p = mapmem_hphys (gphys, len, (wr ? MAPMEM_WRITE : 0) | flags);
  p = (u8*)map_domain_page(gphys>>PAGE_SHIFT);
  ASSERT(p);

  if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DID_VID)) // Get Vendor and Device ID
  {
    //printf ("[TPM SW HANDLER] READ Vendor and Device ID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      /*if ((gphys + i) * 1)
      *((u8 *)buf + 1) = tpmVendorID[i] ^ x;
      else
      *((u8 *)buf + i) = tpmVendorID[i];*/
      *((u8 *)buf + i) = tpm_reg_did_vid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_RID)) // Get Revision ID
  {
    //printf ("[TPM SW HANDLER] READ RevID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_rid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_ENABLE)) // Get Interrupt Control
  {
    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_int_enable[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Get Access
  {
    //acc_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_access[0]._raw[i];
    }
    //printf("[TPM SW HANDLER] READ TPM Access %d : len = %d\n", acc_cnt, len);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS)) // Get Status
  {
    //sts_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i];
    }

    //printf ("[TPM SW HANDLER] READ TPM status %d : len = %d\n", sts_cnt, len);

    /*if (fifo_flag)
    {
    fifo_flag = 0;
    sts_flag = 1;
  }*/
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+1))) // Get Status + 1
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+1];
  }

  //printf ("[TPM SW HANDLER] READ TPM status+1 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+2))) // Get Status + 2
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+2];
  }


  //printf ("[TPM SW HANDLER] READ TPM status+2 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Get Result
{
  // XXX: for debugging
  vmexit_cnt++;
  // XXX: for debugging
  //printf ("[TPM SW HANDLER] READ TPM FIFO: len = %d, current_rsp_offset = %d, rsp_offset = %d\n",
  //		len, current_rsp_offset, rsp_offset);
  // XXX: read rsp_buf
  /*if (current_rsp_offset > rsp_offset)
  {
  printf("[TPM SW HANDLER] ERROR: current_rsp_offset > rsp_offset\n");
  unmapmem (p, len);
  return -1;
}*/
for (i = 0; i < len; i++)
{
  *((u8 *)buf + i) = rsp_buf[current_rsp_offset];
  current_rsp_offset++;
  /*tpm_reg_sts[0].data_avail = 1; // XXX: end of data*/
}

if (current_rsp_offset >= rsp_offset)
{
  current_rsp_offset = 0;
  rsp_offset = 0;
  tpm_reg_sts[0].data_avail = 0; // end of data

  //tpm_reg_sts[0].sts_valid = 0;
  tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
  // XXX: for debugging
  //printf("[TPM SW HANDLER] VMEXIT: %d\n", vmexit_cnt);
  vmexit_cnt = 0;
  // XXX: for debugging
}
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INTF_CAPS))
{
  //printf ("[TPM SW HANDLER] READ TPM INTF_CAPS: len = %d\n", len);

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_intf_cap[0]._raw[i];
  }
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_VECTOR))
{
  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_int_vec[0]._raw[i];
  }
}
else
{
  for (i = 0; i < len; i++)
  {
    //if ((TPM_FAKE_MEM_BASE + i) * 1)
    printk(" [mmio-read-else-printk] phys[i]: %lx\n" , (unsigned long) p[i]);
    if ((gphys + i) * 1)
    {
      *((u8 *)buf + i) = p[i] ^ x;
    }
    else
    {
      *((u8 *)buf + i) = p[i];
    }
  }
}
unmap_domain_page(p);
return 1;
}

int tpm_mmio_write(struct vcpu *v, mmio_info_t *info)
{
  paddr_t gphys = info->gpa;
  struct hsr_dabt dabt = info->dabt;
  struct cpu_user_regs *regs = guest_cpu_user_regs();
  register_t *r = select_user_reg(regs, dabt.reg);
  void *buf = (void*)r;

  //    debug("TPM_Write hanadler called");
  if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Write TPM commands
  {
    //debug ("[TPM SW HANDLER] WRITE ACCESS : %lx\n", (unsigned long **)buf);

    tpm_reg_access[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_VECTOR))
  {
    //debug ("[TPM SW HANDLER] WRITE TPM_INT_VECTORa\n");
    tpm_reg_int_vec[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_ENABLE))
  {
    //debug ("[TPM SW HANDLER] WRITE TPM_INT_ENABLE\n");
    tpm_reg_int_enable[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Write Data
  {
    //debug ("[TPM SW HANDLER] WRITE DATA FIFO\n");
    //printf ("DATA: 0x%x, cmd_offset: %d\n", *((u8 *)buf), cmd_offset);
    // XXX: for debugging
    vmexit_cnt++;
    // XXX: for debugging

    cmd_buf[cmd_offset] = *((u8 *)buf);
    if (cmd_offset < 10) // header
    {
      tpm_input_header._raw[input_header_offset] = *((u8 *)buf);
      input_header_offset--;
    }
    cmd_offset++;

    if (cmd_offset == 10)
    {
      //printf ("tpm_input_header.len = %d\n", tpm_input_header.len);
      input_header_offset = 9;
    }

    tpm_reg_sts[0].expect = 1;
    tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

    if (cmd_offset >= 10)
    {
      if (tpm_input_header.len == cmd_offset) // no more data
      {
        tpm_reg_sts[0].expect = 0;

        //tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;
        tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
      }
    }

    /*if (sts_flag)
    {
    tpm_reg_sts[0].expect = 0;
    //tpm_reg_sts[0].command_ready = 0; // XXX: right?
    sts_flag = 0;
  }
  else
  fifo_flag = 1;*/
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS))
{
  //debug ("[TPM SW HANDLER] WRITE STS\n");
  //printf ("TPM_REG_STS: 0x%x, Buf: 0x%x\n", tpm_reg_sts[0]._raw[0], *((u8 *)buf));

  tpm_reg_sts[0]._raw[0] |= *((u8 *)buf);
  //printf ("TPM_REG_STS RESULT: 0x%x\n", tpm_reg_sts[0]._raw[0]);

  if (tpm_reg_sts[0].tpm_go) // Transmit swtpm
  {
    //printf ("TPM Status: tpm_go!! - cmd_offset: %d\n", cmd_offset);
    //tpm_reg_sts[0].expect = 0;

    int res;

    current_rsp_offset = 0;
    //rsp_offset = 0;
    rsp_offset = TPM_RSP_SIZE_MAX;

    res = integrate_swtpm (cmd_buf, &cmd_offset, rsp_buf, &rsp_offset); // XXX: HERE
    //printf("---rsp_buf (length: %d) ---\n", rsp_offset);
    //for (i = 0; i < rsp_offset; i++)
    //	printf("0x%x ", rsp_buf[i]);
    //printf("\n");
    tpm_reg_sts[0].data_avail = 1;
    tpm_reg_sts[0].expect = 0;
    tpm_reg_sts[0].tpm_go = 0;
    tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

    cmd_offset = 0;
  }
}
else
{
  debug("===== gphys: %llx =====\n", gphys);
}
return 1;
}
#elif defined (THIN_HYP)
extern cpu_t vcpu;
void tpm_mmio_read(const struct ept_violation_info_t *info)
{
  paddr_t gphys;
  unsigned long regNum;
  register_t *r;
  void *buf;
  unsigned long len;
  //unsigned long len = dabt.size;
  u8 *p, x;
  int i;
  x = 0x00;
  gphys = info->gpa;
  regNum = info->hsr.dabt.reg;
  r = (register_t*)select_user_reg(regNum);
  len = 1<<(info->hsr.dabt.size & 0x00000003);
  buf = (void*)r;
  //p = mapmem_hphys (TPM_FAKE_MEM_BASE, len, (wr ? MAPMEM_WRITE : 0) | flags);
  //p = mapmem_hphys (gphys, len, (wr ? MAPMEM_WRITE : 0) | flags);
  p = (u8*)(unsigned long)gphys;
  printf("Read Violation from 0x%x to R%d at (0x%x)\n",gphys,regNum,vcpu.hyp_lr);

  if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DID_VID)) // Get Vendor and Device ID
  {
    //printf ("[TPM SW HANDLER] READ Vendor and Device ID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      /*if ((gphys + i) * 1)
      *((u8 *)buf + 1) = tpmVendorID[i] ^ x;
      else
      *((u8 *)buf + i) = tpmVendorID[i];*/
      *((u8 *)buf + i) = tpm_reg_did_vid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_RID)) // Get Revision ID
  {
    //printf ("[TPM SW HANDLER] READ RevID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_rid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_ENABLE)) // Get Interrupt Control
  {
    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_int_enable[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Get Access
  {
    //acc_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_access[0]._raw[i];
    }
    //printf("[TPM SW HANDLER] READ TPM Access %d : len = %d\n", acc_cnt, len);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS)) // Get Status
  {
    //sts_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i];
    }

    //printf ("[TPM SW HANDLER] READ TPM status %d : len = %d\n", sts_cnt, len);

    /*if (fifo_flag)
    {
    fifo_flag = 0;
    sts_flag = 1;
  }*/
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+1))) // Get Status + 1
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+1];
  }

  //printf ("[TPM SW HANDLER] READ TPM status+1 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+2))) // Get Status + 2
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+2];
  }


  //printf ("[TPM SW HANDLER] READ TPM status+2 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Get Result
{
  // XXX: for debugging
  vmexit_cnt++;
  // XXX: for debugging
  //printf ("[TPM SW HANDLER] READ TPM FIFO: len = %d, current_rsp_offset = %d, rsp_offset = %d\n",
  //		len, current_rsp_offset, rsp_offset);
  // XXX: read rsp_buf
  /*if (current_rsp_offset > rsp_offset)
  {
  printf("[TPM SW HANDLER] ERROR: current_rsp_offset > rsp_offset\n");
  unmapmem (p, len);
  return -1;
}*/
for (i = 0; i < len; i++)
{
  *((u8 *)buf + i) = rsp_buf[current_rsp_offset];
  current_rsp_offset++;
  /*tpm_reg_sts[0].data_avail = 1; // XXX: end of data*/
}

if (current_rsp_offset >= rsp_offset)
{
  current_rsp_offset = 0;
  rsp_offset = 0;
  tpm_reg_sts[0].data_avail = 0; // end of data

  //tpm_reg_sts[0].sts_valid = 0;
  tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
  // XXX: for debugging
  //printf("[TPM SW HANDLER] VMEXIT: %d\n", vmexit_cnt);
  vmexit_cnt = 0;
  // XXX: for debugging
}
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INTF_CAPS))
{
  //printf ("[TPM SW HANDLER] READ TPM INTF_CAPS: len = %d\n", len);

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_intf_cap[0]._raw[i];
  }
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_VECTOR))
{
  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_int_vec[0]._raw[i];
  }
}
else
{
  for (i = 0; i < len; i++)
  {
    //if ((TPM_FAKE_MEM_BASE + i) * 1)
    printk(" [mmio-read-else-printk] phys[i]: %lx\n" , (unsigned long) p[i]);
    if ((gphys + i) * 1)
    {
      *((u8 *)buf + i) = p[i] ^ x;
    }
    else
    {
      *((u8 *)buf + i) = p[i];
    }
  }
}

}
void tpm_mmio_write(const struct ept_violation_info_t *info)
{
  paddr_t gphys;
  unsigned long regNum;
  register_t *r;
  void *buf;
  unsigned long len;
  //unsigned long len = dabt.size;
  u8 x;
  x = 0x00;
  gphys = info->gpa;
  regNum = info->hsr.dabt.reg;
  r = (register_t*)select_user_reg(regNum);
  len = 1<<(info->hsr.dabt.size & 0x00000003);
  buf = (void*)r;
  
  //    debug("TPM_Write hanadler called");
  if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Write TPM commands
  {
    //debug ("[TPM SW HANDLER] WRITE ACCESS : %lx\n", (unsigned long **)buf);

    tpm_reg_access[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_VECTOR))
  {
    //debug ("[TPM SW HANDLER] WRITE TPM_INT_VECTORa\n");
    tpm_reg_int_vec[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INT_ENABLE))
  {
    //debug ("[TPM SW HANDLER] WRITE TPM_INT_ENABLE\n");
    tpm_reg_int_enable[0]._raw[0] |= *((u8 *)buf);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Write Data
  {
    //debug ("[TPM SW HANDLER] WRITE DATA FIFO\n");
    //printf ("DATA: 0x%x, cmd_offset: %d\n", *((u8 *)buf), cmd_offset);
    // XXX: for debugging
    vmexit_cnt++;
    // XXX: for debugging

    cmd_buf[cmd_offset] = *((u8 *)buf);
    if (cmd_offset < 10) // header
    {
      tpm_input_header._raw[input_header_offset] = *((u8 *)buf);
      input_header_offset--;
    }
    cmd_offset++;

    if (cmd_offset == 10)
    {
      //printf ("tpm_input_header.len = %d\n", tpm_input_header.len);
      input_header_offset = 9;
    }

    tpm_reg_sts[0].expect = 1;
    tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

    if (cmd_offset >= 10)
    {
      if (tpm_input_header.len == cmd_offset) // no more data
      {
        tpm_reg_sts[0].expect = 0;

        //tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;
        tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
      }
    }

    /*if (sts_flag)
    {
    tpm_reg_sts[0].expect = 0;
    //tpm_reg_sts[0].command_ready = 0; // XXX: right?
    sts_flag = 0;
  }
  else
  fifo_flag = 1;*/
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS))
{
  //debug ("[TPM SW HANDLER] WRITE STS\n");
  //printf ("TPM_REG_STS: 0x%x, Buf: 0x%x\n", tpm_reg_sts[0]._raw[0], *((u8 *)buf));

  tpm_reg_sts[0]._raw[0] |= *((u8 *)buf);
  //printf ("TPM_REG_STS RESULT: 0x%x\n", tpm_reg_sts[0]._raw[0]);

  if (tpm_reg_sts[0].tpm_go) // Transmit swtpm
  {
    //printf ("TPM Status: tpm_go!! - cmd_offset: %d\n", cmd_offset);
    //tpm_reg_sts[0].expect = 0;

    int res;

    current_rsp_offset = 0;
    //rsp_offset = 0;
    rsp_offset = TPM_RSP_SIZE_MAX;

    res = integrate_swtpm (cmd_buf, &cmd_offset, rsp_buf, &rsp_offset); // XXX: HERE
    //printf("---rsp_buf (length: %d) ---\n", rsp_offset);
    //for (i = 0; i < rsp_offset; i++)
    //	printf("0x%x ", rsp_buf[i]);
    //printf("\n");
    tpm_reg_sts[0].data_avail = 1;
    tpm_reg_sts[0].expect = 0;
    tpm_reg_sts[0].tpm_go = 0;
    tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

    cmd_offset = 0;
  }
}
else
{
  debug("===== gphys: %llx =====\n", gphys);
}
}
#else
int tpm_fake_handler (void *data, phys_t gphys, bool wr, void *buf, uint len, u32 flags)
{
  uint i;
  u8 *p, x;
  x = *(u8 *) data;

  //p = mapmem_hphys (TPM_FAKE_MEM_BASE, len, (wr ? MAPMEM_WRITE : 0) | flags);
  p = mapmem_hphys (gphys, len, (wr ? MAPMEM_WRITE : 0) | flags);
  ASSERT (p);

  // XXX: for debugging
  /*if (vmexit_cnt >= 10000)
  vmexit_cnt = 0;
  printf ("S/W TPM VMExit count = %d\n", vmexit_cnt++);*/

  if (wr) // write
  {
    if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Write TPM commands
    {
      //printf ("[TPM SW HANDLER] WRITE ACCESS: len = %d\n", len);

      tpm_reg_access[0]._raw[0] |= *((u8 *)buf);
    }
    else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Write Data
    {
      //printf ("[TPM SW HANDLER] WRITE DATA FIFO: len = %d\n", len);
      //printf ("DATA: 0x%x, cmd_offset: %d\n", *((u8 *)buf), cmd_offset);
      // XXX: for debugging
      vmexit_cnt++;
      // XXX: for debugging

      cmd_buf[cmd_offset] = *((u8 *)buf);
      if (cmd_offset < 10) // header
      {
        tpm_input_header._raw[input_header_offset] = *((u8 *)buf);
        input_header_offset--;
      }
      cmd_offset++;

      if (cmd_offset == 10)
      {
        //printf ("tpm_input_header.len = %d\n", tpm_input_header.len);
        input_header_offset = 9;
      }

      tpm_reg_sts[0].expect = 1;
      tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

      if (cmd_offset >= 10)
      {
        if (tpm_input_header.len == cmd_offset) // no more data
        {
          tpm_reg_sts[0].expect = 0;

          //tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;
          tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
        }
      }

      /*if (sts_flag)
      {
      tpm_reg_sts[0].expect = 0;
      //tpm_reg_sts[0].command_ready = 0; // XXX: right?
      sts_flag = 0;
    }
    else
    fifo_flag = 1;*/
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS))
  {
    //printf ("[TPM SW HANDLER] WRITE STS: len = %d\n", len);
    //printf ("TPM_REG_STS: 0x%x, Buf: 0x%x\n", tpm_reg_sts[0]._raw[0], *((u8 *)buf));

    tpm_reg_sts[0]._raw[0] |= *((u8 *)buf);
    //printf ("TPM_REG_STS RESULT: 0x%x\n", tpm_reg_sts[0]._raw[0]);

    if (tpm_reg_sts[0].tpm_go) // Transmit swtpm
    {
      //printf ("TPM Status: tpm_go!! - cmd_offset: %d\n", cmd_offset);
      //tpm_reg_sts[0].expect = 0;

      int res;

      current_rsp_offset = 0;
      rsp_offset = 0;

      res = integrate_swtpm (cmd_buf, &cmd_offset, rsp_buf, &rsp_offset); // XXX: HERE
      //printf("---rsp_buf (length: %d) ---\n", rsp_offset);
      //for (i = 0; i < rsp_offset; i++)
      //	printf("0x%x ", rsp_buf[i]);
      //printf("\n");
      tpm_reg_sts[0].data_avail = 1;
      tpm_reg_sts[0].expect = 0;
      tpm_reg_sts[0].tpm_go = 0;
      tpm_reg_sts[0].sts_valid = tpm_reg_sts[0].expect | tpm_reg_sts[0].data_avail;

      cmd_offset = 0;
    }
  }
  else
  {
    //printf("===== gphys: %llx =====\n", gphys);
  }
}
else // read
{
  if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DID_VID)) // Get Vendor and Device ID
  {
    //printf ("[TPM SW HANDLER] READ Vendor and Device ID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      /*if ((gphys + i) * 1)
      *((u8 *)buf + 1) = tpmVendorID[i] ^ x;
      else
      *((u8 *)buf + i) = tpmVendorID[i];*/
      *((u8 *)buf + i) = tpm_reg_did_vid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_RID)) // Get Revision ID
  {
    //printf ("[TPM SW HANDLER] READ RevID: len = %d\n", len);

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_rid[0]._raw[i];
    }
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_ACCESS)) // Get Access
  {
    //acc_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_access[0]._raw[i];
    }
    //printf("[TPM SW HANDLER] READ TPM Access %d : len = %d\n", acc_cnt, len);
  }
  else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_STS)) // Get Status
  {
    //sts_cnt++;

    for (i = 0; i < len; i++)
    {
      *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i];
    }

    //printf ("[TPM SW HANDLER] READ TPM status %d : len = %d\n", sts_cnt, len);

    /*if (fifo_flag)
    {
    fifo_flag = 0;
    sts_flag = 1;
  }*/
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+1))) // Get Status + 1
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+1];
  }

  //printf ("[TPM SW HANDLER] READ TPM status+1 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | (TPM_REG_STS+2))) // Get Status + 2
{
  //sts_cnt++;

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_sts[0]._raw[i+2];
  }


  //printf ("[TPM SW HANDLER] READ TPM status+2 %d : len = %d\n", sts_cnt, len);
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_DATA_FIFO)) // Get Result
{
  // XXX: for debugging
  vmexit_cnt++;
  // XXX: for debugging
  //printf ("[TPM SW HANDLER] READ TPM FIFO: len = %d, current_rsp_offset = %d, rsp_offset = %d\n",
  //		len, current_rsp_offset, rsp_offset);
  // XXX: read rsp_buf
  /*if (current_rsp_offset > rsp_offset)
  {
  printf("[TPM SW HANDLER] ERROR: current_rsp_offset > rsp_offset\n");
  unmapmem (p, len);
  return -1;
}*/
for (i = 0; i < len; i++)
{
  *((u8 *)buf + i) = rsp_buf[current_rsp_offset];
  current_rsp_offset++;
  /*tpm_reg_sts[0].data_avail = 1; // XXX: end of data*/
}

if (current_rsp_offset >= rsp_offset)
{
  current_rsp_offset = 0;
  rsp_offset = 0;
  tpm_reg_sts[0].data_avail = 0; // end of data

  //tpm_reg_sts[0].sts_valid = 0;
  tpm_reg_sts[0].sts_valid = 1; // XXX: debugging - 20141106
  // XXX: for debugging
  printf("[TPM SW HANDLER] VMEXIT: %d\n", vmexit_cnt);
  vmexit_cnt = 0;
  // XXX: for debugging
}
}
else if (gphys == (TPM_LOCALITY_BASE_N(0) | TPM_REG_INTF_CAPS))
{
  //printf ("[TPM SW HANDLER] READ TPM INTF_CAPS: len = %d\n", len);

  for (i = 0; i < len; i++)
  {
    *((u8 *)buf + i) = tpm_reg_intf_cap[0]._raw[i];
  }
}
else
{
  for (i = 0; i < len; i++)
  {
    //if ((TPM_FAKE_MEM_BASE + i) * 1)
    if ((gphys + i) * 1)
    *((u8 *)buf + i) = p[i] ^ x;
    else
    *((u8 *)buf + i) = p[i];
  }
}
} 
unmapmem (p, len);

return 1;
}
#endif

int integrate_swtpm (u8 *in, u32 *in_size, u8 *out, u32 *out_size)
{
  int res;
  //in = command buf
  //out = response buf
  res = tpm_handle_command (in, *in_size, &out, out_size);
  return 0;
}



// static void install_int0x1a_hook (void)
// {
// 	u64 int0x1a_code, int0x1a_base;
// 	u64 int0x1a_vector_phys = 0x1a * 4; /* interrupt vector table */
// 	int len1;
// 	void *p;
//
// 	len1 = guest_int0x1a_hook_end - guest_int0x1a_hook;
// 	int0x1a_code = alloc_realmodemem (len1);
//
// 	int0x1a_base = int0x1a_code;
// 	int0x1a_base &= 0xFFFF0;
//
// 	// save old interrupt vector
// 	read_hphys_l (int0x1a_vector_phys, &guest_int0x1a_orig, 0);
//
// 	// copy the program code
// 	p = mapmem_hphys (int0x1a_code, len1, MAPMEM_WRITE);
// 	memcpy (p, guest_int0x1a_hook, len1);
// 	unmapmem (p, len1);
//
// 	// set interrupt vector
// 	write_hphys_l (int0x1a_vector_phys, (int0x1a_code - int0x1a_base) | (int0x1a_base << 12), 0);
// }
//
void init_swtpm_reg (void)
// static void init_swtpm_reg (void)
{
  // XXX: support only locality 0 in this time.. needs to expend other localities
  debug("init_swtpm_reg\n");
  // Access register
  tpm_reg_access[0].tpm_establishment = 1;
  //tpm_reg_access[0].request_use = 1; // XXX: default 0 ...
  tpm_reg_access[0].request_use = 0; // XXX: default 0 ...
  tpm_reg_access[0].pending_request = 0;
  tpm_reg_access[0].seize = 0;
  tpm_reg_access[0].been_seized = 0;
  //tpm_reg_access[0].active_locality = 0;
  tpm_reg_access[0].active_locality = 1;
  tpm_reg_access[0].reserved = 0;
  tpm_reg_access[0].tpm_reg_valid_sts = 1;

  //debug("TPM_REG_ACCESS :%x", (tpm_reg_access[0]._raw[0]&0x000000FF));
  // Interrupt Enable Register
  tpm_reg_int_enable[0].dataAvailIntEnable = 1;
  //tpm_reg_int_enable[0].dataAvailIntEnable = 0;
  tpm_reg_int_enable[0].stsValidIntEnable  = 1;
  //tpm_reg_int_enable[0].stsValidIntEnable  = 0;
  tpm_reg_int_enable[0].localityChangeIntEnable = 1;
  //tpm_reg_int_enable[0].localityChangeIntEnable = 0;
  tpm_reg_int_enable[0].typePolarity = 01;
  tpm_reg_int_enable[0].reserved1 = 0;
  //tpm_reg_int_enable[0].commandReadyEnable = 0;
  tpm_reg_int_enable[0].commandReadyEnable = 1;
  tpm_reg_int_enable[0].reserved2 = 0;
  //tpm_reg_int_enable[0].globalIntEnable = 0;
  tpm_reg_int_enable[0].globalIntEnable = 1;

  // Status register
  tpm_reg_sts[0].reserved1 = 0;
  tpm_reg_sts[0].response_retry = 0;
  tpm_reg_sts[0].reserved2 = 0;
  //tpm_reg_sts[0].expect = 1;
  tpm_reg_sts[0].expect = 0;
  tpm_reg_sts[0].data_avail = 0; // XXX
  //tpm_reg_sts[0].data_avail = 1;
  tpm_reg_sts[0].tpm_go = 0;
  //tpm_reg_sts[0].command_ready = 1;
  tpm_reg_sts[0].command_ready = 0;
  //tpm_reg_sts[0].sts_valid = 1;
  tpm_reg_sts[0].sts_valid = 0;
  //tpm_reg_sts[0].burst_count = 32; // XXX: exact number...?
  tpm_reg_sts[0].burst_count = 512; // XXX: exact number...? (for performance)

  // FIFO register
  tpm_reg_data_fifo[0]._raw[0] = 0;

  // DID_VID register
  tpm_reg_did_vid[0].value3 = 0x5A;
  tpm_reg_did_vid[0].value2 = 0x48;
  tpm_reg_did_vid[0].value1 = 0x54;
  tpm_reg_did_vid[0].value0 = 0x45;

  // RID register
  tpm_reg_rid[0]._raw[0] = 0x01;

  // Interface Capability
  tpm_reg_intf_cap[0].reserved2 = 0;
  //tpm_reg_intf_cap[0].interface_version = 0x010;
  tpm_reg_intf_cap[0].interface_version = 2;
  tpm_reg_intf_cap[0].reserved1 = 0;
  tpm_reg_intf_cap[0].data_transfer_size = 0x00;
  tpm_reg_intf_cap[0].burst_count = 0;
  tpm_reg_intf_cap[0].cmd_ready = 1;
  tpm_reg_intf_cap[0].int_edge_falling = 0;
  tpm_reg_intf_cap[0].int_edge_rising = 0;
  tpm_reg_intf_cap[0].int_level_low = 1;
  tpm_reg_intf_cap[0].int_level_high = 1;
  tpm_reg_intf_cap[0].locality_change = 1;
  tpm_reg_intf_cap[0].sts_valid = 1;
  tpm_reg_intf_cap[0].data_avail = 1;

  // Interrupt Vector
  tpm_reg_int_vec[0].reserved0 = 0;
  tpm_reg_int_vec[0].sirqVec   = 15;

  cmd_offset = 0;
  rsp_offset = 0;
  current_rsp_offset = 0;
  input_header_offset = 9;
}

// INITFUNC ("bsp0", install_int0x1a_hook);
// INITFUNC ("bsp1", init_swtpm_reg);
