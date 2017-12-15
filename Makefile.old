GENDEV?=/opt/toolchains/gen
GENGCC_BIN=$(GENDEV)/m68k-elf/bin/m68k-elf-
GENBIN=$(GENDEV)/bin

CC = $(GENGCC_BIN)gcc
AS = $(GENGCC_BIN)as
LD = $(GENGCC_BIN)ld
NM = $(GENGCC_BIN)nm
OBJC = $(GENGCC_BIN)objcopy
ASMZ80= bin/sjasm
BINTOS= bin/bintos
RESCOMP= bin/rescomp
XGMTOOL= bin/xgmtool
WAVTORAW= bin/wavtoraw

GCC_VER := $(shell gcc -dumpversion)
GENGCC_VER := $(shell $(CC) -dumpversion)

PLUGIN=$(GENDEV)/m68k-elf/libexec/gcc/m68k-elf/$(GENGCC_VER)

INCS = -Isrc -Ires -Iinc
CCFLAGS = -m68000 -Wall -Wextra -std=c99 -c -fno-builtin -fshort-enums
OPTIONS = 
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(GENDEV)/m68k-elf/m68k-elf/lib -lgcc
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/$(GENGCC_VER)/libgcc.a 
FLAGSZ80 = -isrc/xgm

BOOTSS=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=res/resources.res
Z80S=$(wildcard src/xgm/*.s80)
CS=$(wildcard src/*.c)
CS+=$(wildcard src/ai/*.c)
CS+=$(wildcard src/db/*.c)
CS+=$(wildcard src/xgm/*.c)
SS=$(wildcard src/*.s)
SS+=$(wildcard src/xgm/*.s)
RESOURCES=$(RESS:.res=.o)
RESOURCES+=$(Z80S:.s80=.o)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)

OBJS = $(RESOURCES)

.SECONDARY: doukutsu.elf

.PHONY: all ntsc ntsc-debug ntsc-profile pal pal-debug pal-profile

all: ntsc

ntsc: release
ntsc-profile: profile
ntsc-debug: debug

pal: CCFLAGS += -DPAL
pal: release
pal-profile: CCFLAGS += -DPAL
pal-profile: profile
pal-debug: CCFLAGS += -DPAL
pal-debug: debug

.PHONY: release profile debug main-build head-gen tools

release: OPTIONS = -O3 -fno-web -fno-gcse -fno-unit-at-a-time -flto -fuse-linker-plugin
#release: LINKFLAGS += -s
release: main-build symbol.txt

profile: OPTIONS = -O3 -fno-web -fno-gcse -fno-unit-at-a-time -flto -fuse-linker-plugin
profile: OPTIONS += -fno-omit-frame-pointer
profile: main-build symbol.txt

# Gens-KMod, BlastEm and UMDK support GDB tracing, enabled by this target
debug: OPTIONS = -g -O2 -DDEBUG -DKDEBUG
debug: main-build symbol.txt

main-build: sgdk-tools head-gen doukutsu.bin

.PHONY: tarsaves saves
# This will generate a big folder of SRAM data based on Profile.dat saves on cavestory.org
tarsaves: prof2sram
tarsaves:
	python2 savegen.py
	tar czvf saves.tar.gz save
saves: prof2sram
saves:
	python2 savegen.py
	zip -r saves.zip save

# Cross reference symbol.txt with the addresses displayed in the crash handler
symbol.txt: doukutsu.bin
	$(NM) --plugin=$(PLUGIN)/liblto_plugin.so -n doukutsu.elf > symbol.txt

src/boot/sega.o: src/boot/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync

%.elf: $(OBJS) $(BOOT_RESOURCES)
	$(CC) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(ARCHIVES) $(OBJS) $(LIBS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CCFLAGS) $(OPTIONS) $(INCS) -c $< -o $@

%.o: %.s 
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

%.o80: %.s80
	$(ASMZ80) $(FLAGSZ80) $< $@ out.lst

%.s: %.o80
	$(BINTOS) $<

src/boot/rom_head.o: src/boot/rom_head.c
	$(CC) $(CCFLAGS) -o $@ $<

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

head-gen:
	rm -f inc/ai_gen.h
	python aigen.py


.PHONY: tools sgdk-tools
# CSMD TOOLS

tools: prof2sram

tscomp:
	gcc tools/tscomp/tscomp.c -o tscomp -std=c99

tileopt:
	gcc tools/tileopt/tileopt.c -lSDL2 -lSDL2_image -o tileopt -std=c99

lutgen:
	gcc tools/lutgen/lutgen.c -lm -o lutgen -std=c99

prof2sram:
	gcc tools/prof2sram/prof2sram.c -o prof2sram -std=c99

savetrim:
	gcc tools/savetrim/savetrim.c -o savetrim -std=c99

# SGDK TOOLS

sgdk-tools: $(ASMZ80) $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)

$(ASMZ80):
	make -C tools/sjasm
	cp -f tools/sjasm/sjasm $(ASMZ80)

# gnu99 to remove warnings about strdup()
$(RESCOMP):
	gcc $(wildcard tools/rescomp/*.c) -Itools/rescomp -o $(RESCOMP) -std=gnu99

$(XGMTOOL):
	gcc $(wildcard tools/xgmtool/*.c) -Itools/xgmtool -lm -o $(XGMTOOL) -std=c99

$(BINTOS):
	gcc tools/bintos/bintos.c -o $(BINTOS) -std=c99

$(WAVTORAW):
	gcc tools/wavtoraw/wavtoraw.c -lm -o $(WAVTORAW) -std=c99


.PHONY: clean super-clean

clean:
	rm -f $(RESOURCES)
	rm -f doukutsu.bin doukutsu.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.o src/boot/rom_head.bin
	rm -f src/xgm/z80_xgm.s src/xgm/z80_xgm.o80 src/xgm/z80_xgm.h out.lst
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h

super-clean: clean
	rm -f saves.zip saves.tar.gz
	rm -f prof2sram tileopt tscomp lutgen savetrim
	rm -f $(ASMZ80) $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
