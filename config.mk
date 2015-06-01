#Directory settings
CURRENT = $(shell pwd)
SRC = $(CURRENT)/src
INCLUDE = $(CURRENT)/include
BIN = $(CURRENT)/bin
BASEDIR = $(CURRENT)

ARCH_BIN = $(BIN)/arch
ARCH_SRC = $(SRC)/arch

COMMON_BIN = $(BIN)/common
COMMON_SRC = $(SRC)/common

DRIVER_BIN = $(BIN)/drivers
DRIVER_SRC = $(SRC)/drivers

XSM_BIN = $(BIN)/xsm
XSM_SRC = $(SRC)/xsm

MODULES_BIN = $(BIN)/modules
MODULES_SRC = $(SRC)/modules


INCLUDE_DIR += -I$(INCLUDE)
#End of directory settings

#Link targets
LINK_TARGETS = $(ARCH_BIN)/*.o
# LINK_TARGETS += $(ARCH_BIN)/*.o
# LINK_TARGETS += $(ARCH_BIN)/platforms/*.o
# LINK_TARGETS += $(ARCH_BIN)/asm/lib/*.o
LINK_TARGETS += $(COMMON_BIN)/*.o
# LINK_TARGETS += $(COMMON_BIN)/libfdt/*.o
# LINK_TARGETS += $(COMMON_BIN)/libelf/*.o
# LINK_TARGETS += $(DRIVER_BIN)/char/*.o
# LINK_TARGETS += $(DRIVER_BIN)/passthrough/*.o
# LINK_TARGETS += $(XSM_BIN)/*.o
LINK_TARGETS += $(MODULES_BIN)/swtpm/*.o
LINK_TARGETS += $(MODULES_BIN)/swtpm/tpmcrypto/*.o
LINK_TARGETS += $(MODULES_BIN)/swtpm/bn/*.o
#End of link targets

# for early printk
ifeq ($(CONFIG_EARLY_PRINTK), exynos5250)
EARLY_PRINTK_INC := exynos4210
EARLY_UART_BASE_ADDRESS := 0x12c20000
endif

ifneq ($(EARLY_PRINTK_INC),)
EARLY_PRINTK := y
endif
#end of early-printk

#Cross-compile tools
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
OBJCOPY=$(CROSS_COMPILE)objcopy

# CFLAGS settings
CFLAGS= -O1 -fno-omit-frame-pointer -marm -g -fno-strict-aliasing -std=gnu99 -Wall -Wstrict-prototypes -Wdeclaration-after-statement -Wno-unused-but-set-variable -Wno-unused-local-typedefs   -fno-stack-protector -fno-exceptions -Wnested-externs -msoft-float -mcpu=cortex-a15 -DGCC_HAS_VISIBILITY_ATTRIBUTE -fno-builtin -fno-common -Werror -Wredundant-decls -Wno-pointer-arith -pipe -g -D__XEN__ -fno-optimize-sibling-calls
CFLAGS+= -DVERBOSE -DHAS_PASSTHROUGH -DHAS_DEVICE_TREE -DHAS_PDX -fno-omit-frame-pointer -DCONFIG_FRAME_POINTER -MMD -MF .$(@F).d

CFLAGS-$(EARLY_PRINTK) += -DCONFIG_EARLY_PRINTK
CFLAGS-$(EARLY_PRINTK_INIT_UART) += -DEARLY_PRINTK_INIT_UART
CFLAGS-$(EARLY_PRINTK) += -DEARLY_PRINTK_INC=\"debug-$(EARLY_PRINTK_INC).inc\"
CFLAGS-$(EARLY_PRINTK) += -DEARLY_PRINTK_BAUD=$(EARLY_PRINTK_BAUD)
CFLAGS-$(EARLY_PRINTK) += -DEARLY_UART_BASE_ADDRESS=$(EARLY_UART_BASE_ADDRESS)
CFLAGS-$(EARLY_PRINTK) += -DEARLY_UART_REG_SHIFT=$(EARLY_UART_REG_SHIFT)

CFLAGS+= -DCONFIG_ARM_32 -DTHIN_HYP
CFLAGS+= $(CFLAGS-y)
CFLAGS+= $(INCLUDE_DIR)
#CFLAGS+= -include include/xen/config.h
#end of CFLAGS setting

#AFLAGS setting
AFLAGS = -DCONFIG_ARM_32 -D__ASSEMBLY__ -O1 -fno-omit-frame-pointer -marm -g -fno-strict-aliasing -Wall -Wstrict-prototypes -Wdeclaration-after-statement -Wno-unused-but-set-variable -Wno-unused-local-typedefs -fno-stack-protector -fno-exceptions -Wnested-externs -msoft-float -mcpu=cortex-a15 -DGCC_HAS_VISIBILITY_ATTRIBUTE -fno-builtin -fno-common -Werror -Wredundant-decls -Wno-pointer-arith -pipe -g -D__XEN__ -nostdinc -fno-optimize-sibling-calls -DCONFIG_EARLY_PRINTK  -DVERBOSE -DHAS_PASSTHROUGH -DHAS_DEVICE_TREE -DHAS_PDX -fno-omit-frame-pointer -DCONFIG_FRAME_POINTER -MMD -MF .$(@F).d

AFLAGS-$(EARLY_PRINTK) += -DCONFIG_EARLY_PRINTK
AFLAGS-$(EARLY_PRINTK_INIT_UART) += -DEARLY_PRINTK_INIT_UART
AFLAGS-$(EARLY_PRINTK) += -DEARLY_PRINTK_INC=\"debug-$(EARLY_PRINTK_INC).inc\"
AFLAGS-$(EARLY_PRINTK) += -DEARLY_PRINTK_BAUD=$(EARLY_PRINTK_BAUD)
AFLAGS-$(EARLY_PRINTK) += -DEARLY_UART_BASE_ADDRESS=$(EARLY_UART_BASE_ADDRESS)
AFLAGS-$(EARLY_PRINTK) += -DEARLY_UART_REG_SHIFT=$(EARLY_UART_REG_SHIFT)

AFLAGS+= -DCONFIG_ARM_32 -DTHIN_HYP
AFLAGS+= $(AFLAGS-y)
AFLAGS+= $(INCLUDE_DIR)
#AFLAGS+= -include include/xen/config.h
#end of AFLAGS setting

#LDFLAGS setting
LDFLAGS = -EL
#end of LDFLAGS setting


CONFIG_LOAD_ADDRESS ?= 0x80000000
