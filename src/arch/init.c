#include <stdio.h>
#include <asm/exynos4210-uart.h>

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
  while(1);
}

void start_secondary(unsigned long fdt_paddr,unsigned long cpuid)
{
  while(1);
}
