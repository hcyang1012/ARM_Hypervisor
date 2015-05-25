include config.mk
include $(ARCH_SRC)/build.mk
include $(COMMON_SRC)/build.mk
# include $(DRIVER_SRC)/build.mk
# include $(XSM_SRC)/build.mk

all:clean elf
	$(OBJCOPY) -O binary -S boot.o hypervisor
	mkimage -A arm -T kernel -a 0x80200000 -e 0x80200000 -C none -d "hypervisor" xen4.5-uImage

elf: build
	$(LD) $(LDFLAGS) -T xen.lds -o boot.o  $(LINK_TARGETS)

build: build-arch build-common drivers xsm

build-arch:$(BUILD-ARCH) $(BUILD-ARCH-ASM)

$(BUILD-ARCH):
	$(CC) $(CFLAGS) -c $(SRC)/$*.c -o $(BIN)/$*.o

$(BUILD-ARCH-ASM):
	$(CC) $(AFLAGS) -c $(SRC)/$*.S	-o $(BIN)/$*.o

build-common:$(BUILD-COMMON)

$(BUILD-COMMON):
	$(CC) $(CFLAGS) -c $(SRC)/$*.c -o $(BIN)/$*.o

drivers:$(BUILD-DRIVER)

$(BUILD-DRIVER):
	$(CC) $(CFLAGS) -c $(SRC)/$*.c -o $(BIN)/$*.o

xsm:$(BUILD-XSM)

$(BUILD-XSM):
	$(CC) $(CFLAGS) -c $(SRC)/$*.c -o $(BIN)/$*.o


xen.lds: xen.lds.S
	@echo "################Xen.lds buliding..##################"
	$(CC)  -P -E -Ui386 $(AFLAGS)   -DXEN_PHYS_START=$(CONFIG_LOAD_ADDRESS) -o $@ $<
	sed -e 's/xen\.lds\.o:/xen\.lds:/g' <.xen.lds.d >.xen.lds.d.new
	mv -f .xen.lds.d.new .xen.lds.d

clean:
	rm -rf *.d *.o hypervisor
	rm -rf $(LINK_TARGETS)
