#include <swtpm/tpm_emulator_extern.h>

#ifdef CONFIG_XEN
#include <xen/xmalloc.h>
#include <xen/lib.h>
#include <xen/types.h>
#include <xen/stdarg.h>
#endif

#ifdef THIN_HYP
#include <asm/types.h>
#include <string.h>
#include <stdio.h>
#endif
//#include <core/string.h>


#ifndef TPM_NO_EXTERN

static int _tpm_extern_init(void)
{
  info("_tpm_extern_init()");
  return 0;
}

static void _tpm_extern_release(void)
{
}

static void _tpm_get_extern_random_bytes(void *buf, size_t nbytes)
{
}

static void *_tpm_malloc(size_t size)
{
  #if defined(CONFIG_XEN)
  return xzalloc_bytes(size);
  #elif defined(THIN_HYP)
  return malloc(size);
  #else //CONFIG_BITVISOR
  return alloc(size);
  #endif
}

static void _tpm_free(/*const*/ void *ptr)
{
  if (ptr != NULL)
  {
    #ifdef CONFIG_XEN
    xfree((void*)ptr);
    #else
    free((void*)ptr);
    #endif
  }
}

static void _tpm_log(int priority, const char *fmt, ...)
{
  #if defined (CONFIG_XEN)
  va_list ap;
  va_start(ap, fmt);
  printk(fmt,ap);
  va_end(ap);
  #elif defined(THIN_HYP)
  va_list ap;
  va_start(ap, fmt);
  printf(fmt,ap);
  va_end(ap);
  #else
  va_list ap;
  va_start(ap, fmt);
  vprintf (fmt, ap);
  va_end(ap);
  #endif
}

static int _tpm_write_to_storage(uint8_t *data, size_t data_length)
{
/*  int fh;
  ssize_t res;

#if defined(_WIN32) || defined(_WIN64)
  fh = open(tpm_storage_file, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, S_IRUSR | S_IWUSR);
#else
  fh = open(tpm_storage_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
#endif
  if (fh < 0) return -1;
  while (data_length > 0) {
    res = write(fh, data, data_length);
    if (res < 0) {
      close(fh);
      return -1;
    }
    data_length -= res;
    data += res;
  }
  close(fh); */
  return 0;
}

static uint64_t _tpm_get_ticks(void)
//uint64_t _tpm_get_ticks(void)
{
//	static uint64_t old_t = 0; // XXX: or 1??
	static uint64_t old_t = 1; // XXX: or 1??
/*	uint64_t new_t, res_t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	new_t = (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
	res_t = (old_t > 0) ? new_t - old_t : 0;
	old_t = new_t;
	return res_t; */

/*	uint64_t new_t, res_t;
	u64 time;

	printf("[wizrampa] tpm_get_ticks\n");
	time = get_time();
	//time = get_currenttime();
	new_t = (uint64_t)time;
	res_t = (old_t > 0) ? new_t - old_t : 0;
	old_t = new_t;
	return res_t;*/ // XXX: wizrampa
	return old_t; // XXX: zstormx
}

static int _tpm_read_from_storage(uint8_t **data, size_t *data_length)
{
/*  int fh;
  ssize_t res;
  size_t total_length;

#if defined(_WIN32) || defined(_WIN64)
  fh = open(tpm_storage_file, O_RDONLY | O_BINARY);
#else
  fh = open(tpm_storage_file, O_RDONLY);
#endif
  if (fh < 0) return -1;
  total_length = lseek(fh, 0, SEEK_END);
  lseek(fh, 0, SEEK_SET);
  *data = tpm_malloc(total_length);
  if (*data == NULL) {
    close(fh);
    return -1;
  }
  *data_length = 0;
  while (total_length > 0) {
    res = read(fh, &(*data)[*data_length], total_length);
    if (res < 0) {
      close(fh);
      tpm_free(*data);
      return -1;
    }
    if (res == 0) break;
    *data_length += res;
    total_length -= res;
  }
  close(fh); */
  return 0;
}

int (*tpm_extern_init)(void)                                      = _tpm_extern_init;
void (*tpm_extern_release)(void)                                  = _tpm_extern_release;
void* (*tpm_malloc)(size_t size)                                  = _tpm_malloc;
void (*tpm_free)(/*const*/ void *ptr)                             = _tpm_free;
void (*tpm_log)(int priority, const char *fmt, ...)               = _tpm_log;
void (*tpm_get_extern_random_bytes)(void *buf, size_t nbytes)     = _tpm_get_extern_random_bytes;
uint64_t (*tpm_get_ticks)(void)                                   = _tpm_get_ticks;
int (*tpm_write_to_storage)(uint8_t *data, size_t data_length)    = _tpm_write_to_storage;
int (*tpm_read_from_storage)(uint8_t **data, size_t *data_length) = _tpm_read_from_storage;

#else /* TPM_NO_EXTERN */

int (*tpm_extern_init)(void)                                      = NULL;
void (*tpm_extern_release)(void)                                  = NULL;
void* (*tpm_malloc)(size_t size)                                  = NULL;
void (*tpm_free)(/*const*/ void *ptr)                             = NULL;
void (*tpm_log)(int priority, const char *fmt, ...)               = NULL;
void (*tpm_get_extern_random_bytes)(void *buf, size_t nbytes)     = NULL;
uint64_t (*tpm_get_ticks)(void)                                   = NULL;
//int (*tpm_write_to_storage)(uint8_t *data, size_t data_length)    = NULL;
//int (*tpm_read_from_storage)(uint8_t **data, size_t *data_length) = NULL;

#endif /* TPM_NO_EXTERN */
