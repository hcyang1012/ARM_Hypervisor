#ifndef __COMPILER_H
#define __COMPILER_H

#define __packed      __attribute__((packed))
#define barrier()     __asm__ __volatile__("": : :"memory")
#endif
