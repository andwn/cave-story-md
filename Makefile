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

INCS = -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -Wextra -std=c99 -c -fno-builtin
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(GENDEV)/m68k-elf/m68k-elf/lib -lgcc -lnosys 
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/*/libgcc.a 
FLAGSZ80 = -isrc/xgm

BOOTSS=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=$(wildcard res/*.res)
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
tarsaves: prof2sram
tarsaves:
	python2 savegen.py
	tar czvf saves.tar.gz save
saves: prof2sram
saves:
	python2 savegen.py
	zip -r saves.zip save
ntsc: release

ntsc-debug: debug

pal: CCFLAGS += -DPAL
pal: release

pal-debug: CCFLAGS += -DPAL
pal-debug: debug

release: $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
release: CCFLAGS += -O2
release: main-build

debug: $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
debug: CCFLAGS += -g -O1 -DDEBUG -DKDEBUG
debug: main-build

main-build: head-gen doukutsu.bin symbol.txt

symbol.txt: doukutsu.bin
	$(NM) -n doukutsu.elf > symbol.txt

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

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

head-gen:
	rm -f inc/ai_gen.h
	python aigen.py
	
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

$(RESCOMP):
	gcc $(wildcard tools/rescomp/*.c) -Itools/rescomp -o $(RESCOMP)

$(XGMTOOL):
	gcc $(wildcard tools/xgmtool/*.c) -Itools/xgmtool -lm -o $(XGMTOOL)

$(BINTOS):
	gcc tools/bintos/bintos.c -o $(BINTOS)

$(WAVTORAW):
	gcc tools/wavtoraw/wavtoraw.c -lm -o $(WAVTORAW)

clean: clean-tools
	rm -f $(RESOURCES)
	rm -f doukutsu.bin doukutsu.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.o src/boot/rom_head.bin
	rm -f src/xgm/z80_drv.s src/xgm/z80_drv.o80
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h
	rm -f saves.zip saves.tar.gz

clean-tools:
	rm -f prof2sram tileopt tscomp lutgen savetrim
	rm -f $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)
