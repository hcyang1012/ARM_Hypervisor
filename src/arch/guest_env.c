#include <asm/page.h>
#include <asm/types.h>
#include <asm/processor.h>
#include <config.h>


//lpae_t guest_ept_L1[LPAE_ENTRIES]                               __attribute__((__aligned__(4096)));
// lpae_t guest_ept_L2[LPAE_ENTRIES][LPAE_ENTRIES]                 __attribute__((__aligned__(4096)));
// lpae_t guest_ept_L3[LPAE_ENTRIES][LPAE_ENTRIES][LPAE_ENTRIES]   __attribute__((__aligned__(4096)));
