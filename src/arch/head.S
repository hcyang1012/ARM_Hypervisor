#include <config.h>
#include <asm/processor.h>
#include <asm/cpregs.h>
#include <asm/vcpu.h>
#include <asm/page.h>

#define ZIMAGE_MAGIC_NUMBER 0x016f2818

#define PT_PT     0xf7f /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=1 P=1 */
#define PT_MEM    0xf7d /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=0 P=1 */
#define PT_MEM2   0xffd /* nG=1 AF=1 SH=11 AP=11 NS=1 ATTR=111 T=0 P=1 */
#define PT_MEM3   0x745
#define PT_MEM_L3 0xf7f /* nG=1 AF=1 SH=11 AP=01 NS=1 ATTR=111 T=1 P=1 */
#define PT_DEV    0xe71 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=100 T=0 P=1 */
#define PT_DEV2   0xF71 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=100 T=0 P=1 */
#define PT_DEV_L3 0xe73 /* nG=1 AF=1 SH=10 AP=01 NS=1 ATTR=100 T=1 P=1 */

#define PT_UPPER(x) (PT_##x & 0xf00)
#define PT_LOWER(x) (PT_##x & 0x0ff)

#if (defined (CONFIG_EARLY_PRINTK)) && (defined (EARLY_PRINTK_INC))
#include EARLY_PRINTK_INC
#endif

/*
 * Common register usage in this file:
 *   r0  -
 *   r1  -
 *   r2  -
 *   r3  -
 *   r4  -
 *   r5  -
 *   r6  - identity map in place
 *   r7  - CPUID
 *   r8  - DTB address (boot CPU only)
 *   r9  - paddr(start)
 *   r10 - phys offset
 *   r11 - UART address
 *   r12 - is_secondary_cpu
 *   r13 - SP
 *   r14 - LR
 *   r15 - PC
 */
/* Macro to print a string to the UART, if there is one.
 * Clobbers r0-r3. */
#ifdef CONFIG_EARLY_PRINTK
#define PRINT(_s)       \
        adr   r0, 98f ; \
        bl    puts    ; \
        b     99f     ; \
98:     .asciz _s     ; \
        .align 2      ; \
99:
#else /* CONFIG_EARLY_PRINTK */
#define PRINT(s)
#endif /* !CONFIG_EARLY_PRINTK */
.section ".start", #alloc, #execinstr
        .arm

        /* This must be the very first address in the loaded image.
         * It should be linked at XEN_VIRT_START, and loaded at any
         * 4K-aligned address.  All of text+data+bss must fit in 2MB,
         * or the initial pagetable code below will need adjustment. */
GLOBAL(start)
        /* zImage magic header, see:
         * http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html#d0e309
         */
        .rept 8
        mov   r0, r0
        .endr
        b     past_zImage

        .word ZIMAGE_MAGIC_NUMBER    /* Magic numbers to help the loader */
        .word 0x00000000             /* absolute load/run zImage address or
                                      * 0 for PiC */
        .word (_end - start)         /* zImage end address */

past_zImage:
        cpsid aif                    /* Disable all interrupts */

        /* Save the bootloader arguments in less-clobberable registers */
        mov   r8, r2                 /* r8 := DTB base address */

        /* Relocation to start of the RAM */
        /* r0 : source */
        /* r1 : destination */
        /* r2 : end */
        /* r3, r4, r5, r6 : buffer */
        adr r0, start
        ldr r1, =start
        ldr r2, =_end
1:
        ldmia r0!, {r3 ,r4, r5, r6}
        stmia r1!, {r3 ,r4, r5, r6}
        cmp r1, r2
        blt 1b
        ldr pc, =end_location
        /* End of relocation */

end_location:
        /* Find out where we are */
        ldr   r0, =start
        adr   r9, start              /* r9  := paddr (start) */
        sub   r10, r9, r0            /* r10 := phys-offset */

        /* Using the DTB in the .dtb section? */
#ifdef CONFIG_DTB_FILE
        ldr   r8, =_sdtb
        add   r8, r10                /* r8 := paddr(DTB) */
#endif

        mov   r12, #0                /* r12 := is_secondary_cpu */

        b     common_start

GLOBAL(init_secondary)
        cpsid aif                    /* Disable all interrupts */

        /* Find out where we are */
        ldr   r0, =start
        adr   r9, start              /* r9  := paddr (start) */
        sub   r10, r9, r0            /* r10 := phys-offset */

        mov   r12, #1                /* r12 := is_secondary_cpu */

common_start:
        mov   r7, #0                 /* r7 := CPU ID. Initialy zero until we
                                      * find that multiprocessor extensions are
                                      * present and the system is SMP */
        mrc   CP32(r1, MPIDR)
        tst   r1, #MPIDR_SMP         /* Multiprocessor extension supported? */
        beq   1f
        tst   r1, #MPIDR_UP          /* Uniprocessor system? */
        bne   1f
        bic   r7, r1, #(~MPIDR_HWID_MASK) /* Mask out flags to get CPU ID */
1:

        /* Non-boot CPUs wait here until __cpu_up is ready for them */
        teq   r12, #0
        beq   1f

        ldr   r0, =smp_up_cpu
        add   r0, r0, r10            /* Apply physical offset */
        dsb
2:      ldr   r1, [r0]
        cmp   r1, r7
        beq   1f
        wfe
        b     2b
1:

#ifdef CONFIG_EARLY_PRINTK
        ldr   r11, =EARLY_UART_BASE_ADDRESS  /* r11 := UART base address */
        teq   r12, #0                /* Boot CPU sets up the UART too */
        bleq  init_uart
        PRINT("- CPU ")
        mov   r0, r7
        bl    putn
        PRINT(" booting -\r\n")
#endif
        /* Check that this CPU has Hyp mode */
        mrc   CP32(r0, ID_PFR1)
        and   r0, r0, #0xf000        /* Bits 12-15 define virt extensions */
        teq   r0, #0x1000            /* Must == 0x1 or may be incompatible */
        beq   1f
        PRINT("- CPU doesn't support the virtualization extensions -\r\n")
        b     fail
1:

        /* Check that we're already in Hyp mode */
        mrs   r0, cpsr
        and   r0, r0, #0x1f          /* Mode is in the low 5 bits of CPSR */
        teq   r0, #0x1a              /* Hyp Mode? */
        beq   hyp

        /* OK, we're boned. */
        PRINT("- Xen must be entered in NS Hyp mode -\r\n" \
              "- Please update the bootloader -\r\n")
        b     fail

hyp:    PRINT("- Xen starting in Hyp mode -\r\n")

        /* Zero BSS On the boot CPU to avoid nasty surprises */
        teq   r12, #0
        bne   skip_bss

        PRINT("- Zero BSS -\r\n")
        ldr   r0, =__bss_start       /* Load start & end of bss */
        ldr   r1, =__bss_end
        add   r0, r0, r10            /* Apply physical offset */
        add   r1, r1, r10

        mov   r2, #0
1:      str   r2, [r0], #4
        cmp   r0, r1
        blo   1b

skip_bss:
        PRINT("- Setting up control registers -\r\n")
        /* But do nothing in thin-hypervisor */

cpu_init_done:
        /* Set up memory attribute type tables */
        ldr   r0, =MAIR0VAL
        ldr   r1, =MAIR1VAL
        mcr   CP32(r0, MAIR0)
        mcr   CP32(r1, MAIR1)
        mcr   CP32(r0, HMAIR0)
        mcr   CP32(r1, HMAIR1)

        /* Set up the HTCR:
         * PT walks use Inner-Shareable accesses,
         * PT walks are write-back, write-allocate in both cache levels,
         * Full 32-bit address space goes through this table. */
        ldr   r0, =(TCR_RES1|TCR_SH0_IS|TCR_ORGN0_WBWA|TCR_IRGN0_WBWA|TCR_T0SZ(0))
        mcr   CP32(r0, HTCR)

        /* Set up the HSCTLR:
         * Exceptions in LE ARM,
         * Low-latency IRQs disabled,
         * Write-implies-XN disabled (for now),
         * D-cache disabled (for now),
         * I-cache enabled,
         * Alignment checking enabled,
         * MMU translation disabled (for now). */
        ldr   r0, =(HSCTLR_BASE|SCTLR_A)
        mcr   CP32(r0, HSCTLR)

        /* Set up page table */
        /* Write Xen's PT's paddr into the HTTBR */
        ldr   r4, =boot_pgtable
        add   r4, r4, r10            /* r4 := paddr (boot_pagetable) */
        mov   r5, #0                 /* r4:r5 is paddr (boot_pagetable) */
        mcrr  CP64(r4, r5, HTTBR)


        /* Thin hypervisor's page table has 4 entries and each entry maps 1GB memory area */

        mov   r9, #0x00000000
        mov   r3, #0x0

        lsr   r1, r9, #FIRST_SHIFT
        lsl   r2, r1, #FIRST_SHIFT   /* Base address for 1GB mapping */
        orr   r2, r2, #PT_UPPER(DEV2) /* r2:r3 := section map */
        orr   r2, r2, #PT_LOWER(DEV2)
        mov   r1, #0x0
1:
        strd  r2, r3, [r4, r1]
        add   r2, r2, #0x40000000
        add   r1, r1, #8
        cmp   r1, #24
        bne   1b

        PRINT("- Turning on paging -\r\n")

        mrc   CP32(r0, HSCTLR)
        orr   r0, r0, #(SCTLR_M|SCTLR_C) /* Enable MMU and D-cache */
        dsb                          /* Flush PTE writes and finish reads */
        mcr   CP32(r0, HSCTLR)       /* now paging is enabled */
        isb                          /* Now, flush the icache */

        PRINT("- Ready -\r\n")
        /*
         * Flush the TLB in case the 1:1 mapping happens to clash with
         * the virtual addresses used by the fixmap or DTB.
         */
        dsb                          /* Ensure any page table updates made above
                                      * have occurred. */

        isb
        mcr   CP32(r0, TLBIALLH)     /* Flush hypervisor TLB */
        dsb                          /* Ensure completion of TLB flush */
        isb

        PRINT("- Launch -\r\n")


        /* The boot CPU should go straight into C now */
        teq   r12, #0
        beq   launch

        /* Non-boot CPUs need to move on to the proper pagetables, which were
         * setup in init_secondary_pagetables. */

        ldr   r4, =init_ttbr         /* VA of HTTBR value stashed by CPU 0 */
        ldrd  r4, r5, [r4]           /* Actual value */
        dsb
        mcrr  CP64(r4, r5, HTTBR)
        dsb
        isb
        mcr   CP32(r0, TLBIALLH)     /* Flush hypervisor TLB */
        mcr   CP32(r0, ICIALLU)      /* Flush I-cache */
        mcr   CP32(r0, BPIALL)       /* Flush branch predictor */
        dsb                          /* Ensure completion of TLB+BP flush */
        isb

launch:
        ldr   sp, =cpu0_stack
        add   sp, #STACK_SIZE        /* (which grows down from the top). */
        sub   sp, #CPUINFO_sizeof    /* Make room for CPU save record */
        mov   r0, r8                 /*               - DTB address */
        mov   r1, r7                 /*               - CPU ID */
        teq   r12, #0
        beq   start_hyp              /* and disappear into the land of C */
        b     start_secondary        /* (to the appropriate entry point) */


/* Fail-stop */
fail:   PRINT("- Boot failed -\r\n")
1:      wfe
        b     1b


GLOBAL(_end_boot)

/* void guest_start(unsigned long addr) */
ENTRY(guest_start)
  ldr r11, =EARLY_UART_BASE_ADDRESS

  ldr r12, =KERNEL_ADDR
  msr elr_hyp, r12

  ldr r12, =PSR_GUEST32_INIT
  msr	spsr_hyp, r12

  mov r0, #0x0
  ldr r1, =MACHINE_NUM
  mov r2, #DTB_ADDR
  eret

#ifdef CONFIG_EARLY_PRINTK
/* Bring up the UART.
 * r11: Early UART base address
 * Clobbers r0-r2 */
init_uart:
#ifdef EARLY_PRINTK_INIT_UART
        early_uart_init r11, r1, r2
#endif
        adr   r0, 1f
        b     puts                  /* Jump to puts */
1:      .asciz "- UART enabled -\r\n"
        .align 4

/* Print early debug messages.
 * r0: Nul-terminated string to print.
 * r11: Early UART base address
 * Clobbers r0-r1 */
puts:
        early_uart_ready r11, r1
        ldrb  r1, [r0], #1           /* Load next char */
        teq   r1, #0                 /* Exit on nul */
        moveq pc, lr
        early_uart_transmit r11, r1
        b puts

/* Print a 32-bit number in hex.  Specific to the PL011 UART.
 * r0: Number to print.
 * r11: Early UART base address
 * Clobbers r0-r3 */
.globl putn
putn:
        adr   r1, hex
        mov   r3, #8
1:
        early_uart_ready r11, r2
        and   r2, r0, #0xf0000000    /* Mask off the top nybble */
        ldrb  r2, [r1, r2, lsr #28]  /* Convert to a char */
        early_uart_transmit r11, r2
        lsl   r0, #4                 /* Roll it through one nybble at a time */
        subs  r3, r3, #1
        bne   1b
        mov   pc, lr

hex:    .ascii "0123456789abcdef"
        .align 2

#else  /* CONFIG_EARLY_PRINTK */

ENTRY(early_puts)
init_uart:
puts:
putn:   mov   pc, lr

#endif /* !CONFIG_EARLY_PRINTK */
/*
 * Local variables:
 * mode: ASM
 * indent-tabs-mode: nil
 * End:
 */
