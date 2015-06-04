#ifndef __CONFIG_H
#define __CONFIG_H


#define __ALIGN .align 2
#define __ALIGN_STR ".align 2"

#ifdef __ASSEMBLY__
#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR
#define ENTRY(name)                             \
  .globl name;                                  \
  ALIGN;                                        \
  name:
#define GLOBAL(name)                            \
  .globl name;                                  \
  name:
#define END(name) \
  .size name, .-name
#define ENDPROC(name) \
  .type name, %function; \
  END(name)
#endif

#define PAGE_SHIFT              12

#ifndef __ASSEMBLY__
#define PAGE_SIZE           (1L << PAGE_SHIFT)
#else
#define PAGE_SIZE           (1 << PAGE_SHIFT)
#endif
#define PAGE_MASK           (~(PAGE_SIZE-1))
#define PAGE_FLAG_MASK      (~0)

#define STACK_ORDER 3
#define STACK_SIZE  (PAGE_SIZE << STACK_ORDER)
#define HEAP_SIZE   (4096 * 8)

#define RAM_START   0x20000000
#define RAM_END     (0x40000000 + 0x80000000)

#define KERNEL_ADDR 0x44000000
#define KERNEL_START 0x82000000
#define KERNEL_SIZE 3893640

#define DTB_ADDR    0x42000000
#define DTB_START   0x80000000
#define DTB_SIZE    8260

#define MACHINE_NUM 0xffffffff



#endif
