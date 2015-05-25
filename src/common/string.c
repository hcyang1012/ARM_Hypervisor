#include <config.h>
#include <asm/types.h>
static unsigned long malloc_ptr;
static int malloc_count;
extern unsigned char heap;
static unsigned long free_mem_ptr;
static unsigned long free_mem_end_ptr;


void malloc_init(void)
{
  free_mem_ptr = (unsigned long)&heap;
  free_mem_end_ptr = free_mem_ptr + HEAP_SIZE;
}

void *malloc(int size)
{
  void *p;

  if (size < 0) return 0;
  if (!malloc_ptr) malloc_ptr = free_mem_ptr;

  malloc_ptr = (malloc_ptr + 3) & ~3;     /* Align */

  p = (void *)malloc_ptr;
  malloc_ptr += size;

  if (free_mem_end_ptr && malloc_ptr >= free_mem_end_ptr) return 0;

  malloc_count++;
  return p;
}

void free(void *where)
{
  malloc_count--;
  if (!malloc_count)
  malloc_ptr = free_mem_ptr;
}


void *memset(void * s,int c,size_t count)
{
	char *xs = (char *) s;

	while (count--)
		*xs++ = c;

	return s;
}

void *memcpy(void * dest,const void *src,size_t count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}
