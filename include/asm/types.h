#ifndef __ASM_TYPES_H
#define __ASM_TYPES_H


#ifndef __ASSEMBLY__
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#if defined(CONFIG_ARM_32)
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#elif defined (CONFIG_ARM_64)
typedef __signed__ long __s64;
typedef unsigned long __u64;
#endif
#endif

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

#if defined(CONFIG_ARM_32)
typedef signed long long s64;
typedef unsigned long long u64;
typedef u32 vaddr_t;
#define PRIvaddr PRIx32
typedef u64 paddr_t;
#define INVALID_PADDR (~0ULL)
#define PRIpaddr "016llx"
typedef u32 register_t;
#define PRIregister "x"
#elif defined (CONFIG_ARM_64)
typedef signed long s64;
typedef unsigned long u64;
typedef u64 vaddr_t;
#define PRIvaddr PRIx64
typedef u64 paddr_t;
#define INVALID_PADDR (~0UL)
#define PRIpaddr "016lx"
typedef u64 register_t;
#define PRIregister "lx"
#endif

#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ size_t;
#else
typedef unsigned long size_t;
#endif
typedef signed long ssize_t;

typedef char bool_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#define INT_MAX         ((int)(~0U>>1))
#define INT_MIN         (-INT_MAX - 1)
#define UINT_MAX        (~0U)
#define LONG_MAX        ((long)(~0UL>>1))
#define LONG_MIN        (-LONG_MAX - 1)
#define ULONG_MAX       (~0UL)

/* bsd */
typedef unsigned char           u_char;
typedef unsigned short          u_short;
typedef unsigned int            u_int;
typedef unsigned long           u_long;

/* sysv */
typedef unsigned char           unchar;
typedef unsigned short          ushort;
typedef unsigned int            uint;
typedef unsigned long           ulong;

typedef         __u8            uint8_t;
typedef         __u8            u_int8_t;
typedef         __s8            int8_t;

typedef         __u16           uint16_t;
typedef         __u16           u_int16_t;
typedef         __s16           int16_t;

typedef         __u32           uint32_t;
typedef         __u32           u_int32_t;
typedef         __s32           int32_t;

typedef         __u64           uint64_t;
typedef         __u64           u_int64_t;
typedef         __s64           int64_t;


typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;

typedef unsigned long uintptr_t;

#endif /* __ASSEMBLY__ */


#endif
