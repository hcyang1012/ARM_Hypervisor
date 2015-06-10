#ifndef __ASM_MMIO_H
#define __ASM_MMIO_H
#include <asm/ept_violation.h>
int handle_mmio(struct ept_violation_info_t *info);
void init_mmio(void);
#endif
