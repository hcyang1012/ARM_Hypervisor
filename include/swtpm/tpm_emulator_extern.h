/* Software-based Trusted Platform Module (TPM) Emulator
 * Copyright (C) 2004-2010 Mario Strasser <mast@gmx.net>
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * $Id: tpm_emulator_extern.h 440 2010-03-17 22:53:07Z mast $
 */

#ifndef _TPM_EMULATOR_EXTERN_H_
#define _TPM_EMULATOR_EXTERN_H_

#ifdef CONFIG_XEN
#include <xen/types.h>
#endif

#ifdef THIN_HYP
#include <asm/types.h>
#endif


/* log functions */

enum {
  TPM_LOG_DEBUG,
  TPM_LOG_INFO,
  TPM_LOG_ERROR
};

#define debug(fmt, ...) tpm_log(TPM_LOG_DEBUG, "SWTPM Debug: " fmt "\n", \
                                ## __VA_ARGS__)
#define info(fmt, ...)  tpm_log(TPM_LOG_INFO, "SWTPM Info: " fmt "\n", \
                                ## __VA_ARGS__)
#define error(fmt, ...) tpm_log(TPM_LOG_ERROR, "SWTPM Error: " fmt "\n", \
                                ## __VA_ARGS__)

extern int (*tpm_extern_init)(void);
extern void (*tpm_extern_release)(void);
extern void* (*tpm_malloc)(size_t size);
extern void (*tpm_free)(/*const*/ void *ptr);
extern void (*tpm_log)(int priority, const char *fmt, ...);
extern void (*tpm_get_extern_random_bytes)(void *buf, size_t nbytes);
extern uint64_t (*tpm_get_ticks)(void);
#endif /* _TPM_EMULATOR_EXTERN_H_ */
