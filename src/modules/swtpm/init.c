#include <swtpm/tpm_emulator.h>
#include <swtpm/swtpm.h>
#include <swtpm/tcg.h>
#include <swtpm/init.h>

#ifdef CONFIG_XEN
#include <xen/lib.h>
#include <xen/sched.h>
#include <xen/types.h>
#include <asm/mmio.h>

static const struct mmio_handler_ops tpm_mmio_handler = {
    .read_handler  = tpm_mmio_read,
    .write_handler = tpm_mmio_write,
};

#endif
#if defined(THIN_HYP)
#include <stdio.h>
#endif

extern struct domain *global_dom0;
void swtpm_init(void)
{
  uint32_t tpm_startup = 1; //startup: 1-"clear", 2-"save", 3-"deactivated"
  uint32_t tpm_config =	TPM_CONF_USE_INTERNAL_PRNG |		//0x08
			TPM_CONF_ALLOW_PRNG_STATE_SETTING | 	//0x10
			TPM_CONF_GENERATE_SEED_DAA;		//0X04

  if (tpm_emulator_init (tpm_startup, tpm_config) != 0)
  {
    #ifdef CONFIG_XEN
    printk("swtpm initialization failed\n");
    #else
    printf("swtpm initialization failed\n");
    #endif
  }

  #ifdef CONFIG_XEN
  //Add MMIO Handler
  register_mmio_handler(global_dom0, &tpm_mmio_handler, TPM_MEM_BASE, TPM_MEM_LEN);
  #endif
  init_swtpm_reg();
}
