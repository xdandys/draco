TOP := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

HOSTCC=gcc
TCPREFIX=arm-none-eabi-
CC=$(TCPREFIX)gcc
LD=$(TCPREFIX)ld
GDB=$(TCPREFIX)gdb
OBJCOPY=$(TCPREFIX)objcopy
OBJDUMP=$(TCPREFIX)objdump
AR=$(TCPREFIX)ar
AS=$(TCPREFIX)as
LD=$(TCPREFIX)ld

SRCDIR := src
TARGET_BOOTLOADER := osd_bootloader
TARGET_FIRMWARE := osd_firmware
RESOURCEDIR := $(SRCDIR)/resources
SCRIPTDIR := scripts
UTILSDIR := utils
STMLIBDIR :=$(SRCDIR)/stm32_lib
FONTGEN := $(UTILSDIR)/fontgen.py
BITMAPGEN := $(UTILSDIR)/bitmapgen.py

#linker script
ifeq ($(TARGET_TYPE), $(TARGET_BOOTLOADER))
	LINKERSCRIPT=$(SRCDIR)/linker_bootloader.ld
	SOURCES+=$(SRCDIR)/startup.c
	SOURCES+=$(SRCDIR)/bootloader.c
	SOURCES+=$(SRCDIR)/vectors.c
	SOURCES+=$(SRCDIR)/debug.c
	SOURCES+=$(SRCDIR)/board.c
	SOURCES+=$(SRCDIR)/delay.c
	SOURCES+=$(SRCDIR)/swtimer.c
	SOURCES+=$(SRCDIR)/led.c
	SOURCES+=$(SRCDIR)/utils.c
	SOURCES+=$(SRCDIR)/flashupdate.c
	SOURCES+=$(wildcard $(SRCDIR)/stm32_lib/peripherals/src/*.c)
else ifeq ($(TARGET_TYPE), $(TARGET_FIRMWARE))
	SOURCES+=$(SRCDIR)/startup.c
	SOURCES+=$(SRCDIR)/main.c
	SOURCES+=$(SRCDIR)/board.c
	SOURCES+=$(SRCDIR)/debug.c
	SOURCES+=$(SRCDIR)/delay.c
	SOURCES+=$(SRCDIR)/osdspi.c
	SOURCES+=$(SRCDIR)/osdpainter.c
	SOURCES+=$(SRCDIR)/osdwidget.c
	SOURCES+=$(SRCDIR)/osdscreen.c
	SOURCES+=$(SRCDIR)/swtimer.c
	SOURCES+=$(SRCDIR)/vectors.c
	SOURCES+=$(SRCDIR)/spicommio_stm32f3.c
	SOURCES+=$(SRCDIR)/spicomm.c
	SOURCES+=$(SRCDIR)/comm.c
	SOURCES+=$(SRCDIR)/utils.c
	SOURCES+=$(SRCDIR)/led.c
	SOURCES+=$(SRCDIR)/hud.c
	SOURCES+=$(wildcard $(RESOURCEDIR)/*.c)
	SOURCES+=$(wildcard $(SRCDIR)/widgets/*.c)
	SOURCES+=$(wildcard $(SRCDIR)/stm32_lib/peripherals/src/*.c)
	LINKERSCRIPT=$(SRCDIR)/linker.ld
endif

BUILDDIR := build/$(TARGET_TYPE)
DEPDIR := $(BUILDDIR)/.dep
#create .dep directory
DUMMY:=$(shell mkdir -p $(DEPDIR) )
EXECUTABLE:=$(TARGET_TYPE)

INCLUDE+=-I$(STMLIBDIR)/CMSIS/Include/
INCLUDE+=-I$(STMLIBDIR)/CMSIS/Device/ST/STM32F30x/Include
INCLUDE+=-I$(STMLIBDIR)/peripherals/inc
INCLUDE+=-I$(STMLIBDIR)/usb/inc
INCLUDE+=-Isrc

CPU:=cortex-m4
CFLAGS	+=-O2
CFLAGS  +=-mcpu=$(CPU) -mthumb -march=armv7e-m -mno-unaligned-access
CFLAGS  +=-fomit-frame-pointer -falign-functions=16
CFLAGS	+=-Wall $(INCLUDE)
CFLAGS 	+= -MMD -MP -MF $(DEPDIR)/$(@F).d
CFLAGS	+=-D"assert_param(expr)=((void)0)"
CFLAGS  += -fdata-sections -ffunction-sections
CFLAGS  +=-DSTM32F30X
CFLAGS  +=-DTARGET_TYPE_$(TARGET_TYPE)
CFLAGS  +=-mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant

LDFLAGS+=$(COMMONFLAGS) -fno-exceptions -ffunction-sections -fdata-sections -nostartfiles
LDFLAGS+=-mcpu=$(CPU) -mthumb -mno-unaligned-access
LDFLAGS+=-Wl,-T$(LINKERSCRIPT)
LDFLAGS+=-Wl,--gc-sections
LDFLAGS+=-Wl,-Map=$(BUILDDIR)/$(EXECUTABLE).map
LDLIBS=-lm

OBJECTS=$(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SOURCES:.c=.o))
VERSIONOBJECT=$(BUILDDIR)/version.o
VERSIONSRC=$(SRCDIR)/version.c

all : $(TARGET_BOOTLOADER) $(TARGET_FIRMWARE)

target : $(BUILDDIR)/$(EXECUTABLE).elf

$(TARGET_BOOTLOADER):
	$(MAKE) TARGET_TYPE=$(TARGET_BOOTLOADER) target

$(TARGET_FIRMWARE):
	$(MAKE) TARGET_TYPE=$(TARGET_FIRMWARE) target

$(BUILDDIR)/$(EXECUTABLE).elf : $(OBJECTS) $(VERSIONOBJECT) $(LINKERSCRIPT) $(BUILDDIR)/stampelf.exe
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(VERSIONOBJECT) $(LDLIBS)
	$(BUILDDIR)/stampelf.exe $@
	$(OBJCOPY) -O ihex --gap-fill 0   $(BUILDDIR)/$(EXECUTABLE).elf $(BUILDDIR)/$(EXECUTABLE).hex
	$(OBJCOPY) -O binary --gap-fill 0 $(BUILDDIR)/$(EXECUTABLE).elf $(BUILDDIR)/$(EXECUTABLE).bin
	$(OBJDUMP) -d $(BUILDDIR)/$(EXECUTABLE).elf > $(BUILDDIR)/$(EXECUTABLE).dis

FORCE:

$(VERSIONOBJECT) : FORCE
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DGIT_DESCRIBE="$(shell git describe --abbrev=4 --dirty --always --tags)" -c $(VERSIONSRC) -o $@

$(OBJECTS) : $(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/stampelf.exe : $(SRCDIR)/stampelf.c
	$(HOSTCC) $< -o $@

.PHONY : all clean program bitmaps target $(TARGET_BOOTLOADER) $(TARGET_FIRMWARE)

program : all
	openocd -f$(SCRIPTDIR)/oocdflash.cfg

bitmaps:
	$(FONTGEN) $(RESOURCEDIR)/src/fonts.ini $(RESOURCEDIR)
	$(BITMAPGEN) $(RESOURCEDIR)/src/bitmaps.ini $(RESOURCEDIR)

clean :
	rm -rf build/*