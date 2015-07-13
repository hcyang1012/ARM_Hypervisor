A Tiny ARM Hypervisor

Written by Heecheol, Yang (heecheol.yang@outlook.com)

This is a project for small TCB and lightweight hypervisor which supports minimum features for enhancement in security of embedded system.

Features
- Single guest VM boot
- No scheduler, No multiple VM support for lightweight execution environment
- Very small TCB(Trusted Computing Base) size (<4K LoC)
- No(or minimum) device driver
- Catching security-sensitive events in guest VM
 - MMIO Access
 - Unauthorized memory access
 - Systsem call (not completly supported yet)
 - Interrupt (to be supported)
 - Control register access (to be supported)

The basic concept of this hypervisor comes from Bitvisor (http://www.bitvisor.org/)

How to Build

1. Boot loader
 - This project has been developed on Odroid XU by Hardkernel (http://www.hardkernel.com/main/products/prdt_info.php?g_code=G137510300620)
 - Because ARM Virtualization Extension hardly depends on ARM Trustzone, you need ARM TZ&VE supported bootloader. Xen on ARM Project provides the modified U-Boot for ARM VE. (http://wiki.xenproject.org/wiki/Xen_ARM_with_Virtualization_Extensions/OdroidXU)
 - Therefore, You are highly recommed to read the related page for initial booting environment setup. 

2. Build
 - make CONFIG_EARLY_PRINTK=exynos5250 CROSS_COMPILE=arm-linux-gnueabihf-
 - After build process, uImage is the built image.
 - Copy the file to the boot partiton in booting SD Card with proper name. (Please see the Xen on ARM Odroid Page)

4. Guest VM & DTB
 - You need DTB-supported linux.
 - You can get the ODROID XU Linux from (https://github.com/hardkernel/linux.git)
 - This repository also provides related DTS fils.
 - Build & copy the zImage & DTB file to the boot partition. (Please see the Xen on ARM Odroid Page) 


Demo: https://www.youtube.com/watch?v=WMU8FPr6JnE

ToDo
 - Interrupt catching & VIRQ injection
 - GIC setup & vGIC support




※ Most boot codes are based on Xen on ARM project, but I don't know how to determine the license of this project. Please send me a message if someone knows something about this issue.
