#include <stdio.h>
#include <string.h>
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
  int *testArray;
  printf("Hello,world\n");
  malloc_init();
  testArray = (int*)malloc(sizeof(int) * 5);
  testArray[3] = 2;
  printf("TestArray[3] : %d(%x)\n",testArray[3], testArray);

  while(1);
}

void start_secondary(unsigned long fdt_paddr,unsigned long cpuid)
{
  while(1);
}
