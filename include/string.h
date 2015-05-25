#ifndef __STRING_H
#define __STRING_H
#include <asm/types.h>

void malloc_init(void);
void *malloc(int size);
void free(void *where);
void *memset(void * s,int c,size_t count);
#endif
