#ifndef __STDIO_H
#define __STDIO_H

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#define printk printf
#define KERN_INFO

typedef __builtin_va_list va_list;
#define va_copy(dest, src)    __builtin_va_copy((dest), (src))
#define va_start(ap, last)    __builtin_va_start((ap), (last))
#define va_end(ap)            __builtin_va_end(ap)
#define va_arg                __builtin_va_arg
#endif
