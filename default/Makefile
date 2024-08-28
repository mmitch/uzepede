###############################################################################
# Makefile for the project uzepede
###############################################################################

## Configure this
#UZEBOX_DIR = ../../uzebox
UZEBOX_DIR = /home/mitch/git/uzebox

VERSION=v1.01-GIT

## General Flags
PROJECT = uzepede
GAME = uzepede
MCU = atmega644
TARGET = $(GAME).elf
CC = avr-gcc
INFO=../gameinfo.properties

COMPILEDATE=$(shell date +%Y/%m/%d)
GITCOMMIT=$(shell git rev-parse HEAD | cut -c -20)

## Kernel settings
KERNEL_DIR = $(UZEBOX_DIR)/kernel
KERNEL_OPTIONS = -DVIDEO_MODE=1 -DINTRO_LOGO=0 -DMODE1_FAST_VSYNC=1

GCONVERT_BIN = $(UZEBOX_DIR)/bin/gconvert
PACKROM_BIN = $(UZEBOX_DIR)/bin/packrom

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=28636360UL -Os -mcall-prologues -fsigned-char -ffunction-sections -fno-toplevel-reorder
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d
CFLAGS += $(KERNEL_OPTIONS)


## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(GAME).map
LDFLAGS += -Wl,-gc-sections


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## changing the Makefile should rebuild everything to let things like build flags propagate properly
MAKEFILE = Makefile

## Objects that must be built in order to link
OBJECTS = uzeboxVideoEngineCore.o uzeboxCore.o uzeboxSoundEngine.o uzeboxSoundEngineCore.o uzeboxVideoEngine.o $(GAME).o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Include Directories
INCLUDES = -I"$(KERNEL_DIR)"

## Build
all: ../data/tiles.inc $(TARGET) $(GAME).hex $(GAME).eep $(GAME).lss $(GAME).uze size

## Rebuild graphic ressources
../data/tiles.inc: ../data/tiles.png ../data/tiles.xml $(GCONVERT_BIN) $(MAKEFILE)
	$(GCONVERT_BIN) ../data/tiles.xml

## Compile Kernel files
uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s $(MAKEFILE)
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s $(MAKEFILE)
	$(CC) $(INCLUDES) $(ASMFLAGS) -c $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c $(MAKEFILE)
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c $(MAKEFILE)
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c $(MAKEFILE)
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

## Compile game sources

$(GAME).o: ../$(GAME).c $(MAKEFILE)
	$(CC) -DVERSION=\"$(VERSION)\" -DCOMPILEDATE=\"$(COMPILEDATE)\" -DGITCOMMIT=\"$(GITCOMMIT)\" $(INCLUDES) $(CFLAGS) -c $<

##Link
$(TARGET): $(OBJECTS) $(MAKEFILE)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

%.uze: $(TARGET) $(PACKROM_BIN)
	-$(PACKROM_BIN) $(GAME).hex $@ $(INFO)

UNAME := $(shell sh -c 'uname -s 2>/dev/null || echo not')
AVRSIZEFLAGS := -A ${TARGET}
ifneq (,$(findstring MINGW,$(UNAME)))
AVRSIZEFLAGS := -C --mcu=${MCU} ${TARGET}
endif

size: ${TARGET}
	@echo
	@avr-size ${AVRSIZEFLAGS}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(GAME).* dep/* *.uze ../data/tiles.inc


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)


## Auto-compile the tools needed from Uzebox
$(PACKROM_BIN): $(UZEBOX_DIR)
	make -C $(UZEBOX_DIR) tools/packrom

$(GCONVERT_BIN): $(UZEBOX_DIR)
	make -C $(UZEBOX_DIR) tools/gconvert
