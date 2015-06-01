#ifndef __STRING_H
#define __STRING_H
#include <asm/types.h>

void malloc_init(void);
void *malloc(int size);
void free(void *where);
void *memset(void * s,int c,size_t count);
void *memcpy(void * dest,const void *src,size_t count);
int memcmp(const void * cs,const void * ct,size_t count);
void * memmove(void * dest,const void *src,size_t count);
size_t strlen(const char * s);
void *memchr(const void *s, int c, size_t n);
#endif
