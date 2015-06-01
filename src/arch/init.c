#include <stdio.h>
#include <string.h>
#include <asm/exynos4210-uart.h>
#include <asm/guest_init.h>
#include <asm/traps.h>
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


void start_hyp(unsigned long fdt_paddr,unsigned long cpuid)
{
  printf("Hello,world\n");
  printf("We are currently in : %x\n",start_hyp);
  malloc_init();
  init_traps();
  guest_init();
  swtpm_init();
  printf("Guest init success\n");
  printf("Boot guest kernel\n");
  guest_boot();
  while(1);
}

void start_secondary(unsigned long fdt_paddr,unsigned long cpuid)
{
  while(1);
}
