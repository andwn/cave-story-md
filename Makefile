GENDEV?=/opt/toolchains/gen
GENGCC_BIN=$(GENDEV)/m68k-elf/bin/m68k-elf-
GENBIN=$(GENDEV)/bin

CC = $(GENGCC_BIN)gcc
AS = $(GENGCC_BIN)as
LD = $(GENGCC_BIN)ld
NM = $(GENGCC_BIN)nm
OBJC = $(GENGCC_BIN)objcopy
ASMZ80= $(GENBIN)/sjasm
BINTOS= bin/bintos
RESCOMP= bin/rescomp
XGMTOOL= bin/xgmtool
WAVTORAW= bin/wavtoraw

GCC_VER := $(shell gcc -dumpversion)
GENGCC_VER := $(shell $(CC) -dumpversion)

PLUGIN=$(GENDEV)/m68k-elf/libexec/gcc/m68k-elf/$(GENGCC_VER)

INCS = -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -Wextra -std=c99 -c -fno-builtin 
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(GENDEV)/m68k-elf/m68k-elf/lib -lgcc -lnosys
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/$(GENGCC_VER)/libgcc.a 
FLAGSZ80 = -isrc/xgm

BOOTSS=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=res/resources.res res/tsc_en.res res/bgm_ntsc.res #res/tsc_ja.res
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

.PHONY: all release debug ntsc pal ntsc-debug pal-debug tools clean clean-tools
.SECONDARY: doukutsu.elf

all: ntsc

ntsc: release

ntsc-debug: debug

pal: CCFLAGS += -DPAL
pal: release

pal-debug: CCFLAGS += -DPAL
pal-debug: debug

release: $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
release: CCFLAGS += -O3 -fno-web -fno-gcse -fno-unit-at-a-time -flto -fuse-linker-plugin
release: main-build

# Gens-KMod, BlastEm and UMDK support GDB tracing, enabled by this target
debug: $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
debug: CCFLAGS += -g -O1 -DDEBUG -DKDEBUG
debug: main-build

main-build: head-gen doukutsu.bin symbol.txt

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
	@$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

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
	$(CC) -m68000 -Wall -Wextra -std=c99 -c -fno-builtin -o $@ $<

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

head-gen:
	rm -f inc/ai_gen.h
	python aigen.py

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

# gnu99 to remove warnings about strdup()
$(RESCOMP):
	gcc $(wildcard tools/rescomp/*.c) -Itools/rescomp -o $(RESCOMP) -std=gnu99

$(XGMTOOL):
	gcc $(wildcard tools/xgmtool/*.c) -Itools/xgmtool -lm -o $(XGMTOOL) -std=c99

$(BINTOS):
	gcc tools/bintos/bintos.c -o $(BINTOS) -std=c99

$(WAVTORAW):
	gcc tools/wavtoraw/wavtoraw.c -lm -o $(WAVTORAW) -std=c99

clean: clean-tools
	rm -f $(RESOURCES)
	rm -f doukutsu.bin doukutsu.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.o src/boot/rom_head.bin
	rm -f src/xgm/z80_drv.s src/xgm/z80_drv.o80 src/xgm/z80_drv.h out.lst
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h
	rm -f saves.zip saves.tar.gz

clean-tools:
	rm -f prof2sram tileopt tscomp lutgen savetrim
	rm -f $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
