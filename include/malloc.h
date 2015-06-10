#ifndef __MALLOC_H
#define __MALLOC_H

void MSYS_Init( void *heap, unsigned len );
void MSYS_Free( void *ptr );
void *MSYS_Alloc( unsigned size );
#endif
