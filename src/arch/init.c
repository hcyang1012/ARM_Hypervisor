#include <stdio.h>
#include <string.h>
#include <asm/exynos4210-uart.h>
#include <asm/guest_init.h>
#include <asm/traps.h>
#include <asm/cpregs.h>
#include <asm/processor.h>
#include <asm/mmio.h>
#include <swtpm/init.h>

int putchar(int c)
{
  unsigned char volatile *uart = (unsigned char*)UART_BASE_ADDRESS;

  while(!(uart[UTRSTAT] & UTRSTAT_TXFE))
  {

  }
  uart[UTXH] = c & 0xFF;
  if(c == '\n') putchar('\r');

  return 0;
}

size_t  cacheline_bytes;
void  setup_cache(void)
{
    uint32_t ccsid;

    /* Read the cache size ID register for the level-0 data cache */
    WRITE_SYSREG32(0, CSSELR_EL1);
    ccsid = READ_SYSREG32(CCSIDR_EL1);

    /* Low 3 bits are log2(cacheline size in words) - 2. */
    cacheline_bytes = 1U << (4 + (ccsid & 0x7));
}

void start_hyp(unsigned long fdt_paddr,unsigned long cpuid)
{
  printf("Hello,world\n");
  printf("We are currently in : %x\n",start_hyp);
  setup_cache();
  malloc_init();
  init_traps();
  guest_init();
  //swtpm_init();
  printf("Guest init success\n");
  printf("Boot guest kernel\n");
  guest_boot();
  while(1);
}

void start_secondary(unsigned long fdt_paddr,unsigned long cpuid)
{
  while(1);
}
