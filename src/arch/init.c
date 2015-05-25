#include <stdio.h>
#include <string.h>
#include <asm/exynos4210-uart.h>
#include <asm/guest_init.h>

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
  unsigned long  *kernel_addr = (unsigned long*)0x44000000;
  printf("Hello,world\n");
  malloc_init();
  guest_init();
  printf("Guest init success\n");
  printf("%x : %x\n",kernel_addr,*kernel_addr);
  while(1);
}

void start_secondary(unsigned long fdt_paddr,unsigned long cpuid)
{
  while(1);
}
