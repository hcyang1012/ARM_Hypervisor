#ifndef __GUEST_INIT_H
#define __GUEST_INIT_H

void guest_init(void);
void guest_ept_init(void);
void copy_guest(void);
void guest_boot(void);
void copy_dtb(void);
#endif
