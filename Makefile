GENDEV?=/opt/toolchains/gen
GENGCC_BIN=$(GENDEV)/m68k-elf/bin/m68k-elf-
GENBIN=$(GENDEV)/bin/

CC = $(GENGCC_BIN)gcc
AS = $(GENGCC_BIN)as
LD = $(GENGCC_BIN)ld
NM = $(GENGCC_BIN)nm
OBJC = $(GENGCC_BIN)objcopy
RESCOMP= $(GENBIN)rescomp

INCS = -I$(GENDEV)/m68k-elf/include -I$(GENDEV)/m68k-elf/m68k-elf/include -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -Wextra -Wno-missing-field-initializers -std=c99 -c -fno-builtin
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(GENDEV)/m68k-elf/lib -L$(GENDEV)/m68k-elf/m68k-elf/lib -lmd -lnosys 
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/*/libgcc.a 

BOOTSS=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=$(wildcard res/*.res)
CS=$(wildcard src/*.c)
CS+=$(wildcard src/ai/*.c)
CS+=$(wildcard src/db/*.c)
SS=$(wildcard src/*.s)
RESOURCES=$(RESS:.res=.o)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)

OBJS = $(RESOURCES)

.PHONY: all release debug ntsc pal ntsc-debug pal-debug tools clean
.SECONDARY: doukutsu.elf

all: ntsc

ntsc: release

ntsc-debug: debug

pal: CCFLAGS += -DPAL
pal: release

pal-debug: CCFLAGS += -DPAL
pal-debug: debug

release: CCFLAGS += -O3 -fomit-frame-pointer
release: LIBMD = $(GENDEV)/m68k-elf/lib/libmd.a
release: main-build

debug: CCFLAGS += -g -O1 -DDEBUG -DKDEBUG
debug: LIBMD = $(GENDEV)/m68k-elf/lib/libmd_debug.a
debug: main-build

main-build: head-gen doukutsu.bin symbol.txt

symbol.txt: doukutsu.bin
	$(NM) -n doukutsu.elf > symbol.txt

src/boot/sega.o: out/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync

%.elf: $(OBJS) $(BOOT_RESOURCES)
	$(CC) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(LIBMD) $(ARCHIVES) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

%.o: %.s 
	$(AS) $(ASFLAGS) $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

out/rom_head.bin: src/boot/rom_head.o
	mkdir -p out/boot
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

head-gen:
	rm -f inc/ai_gen.h
	python aigen.py
	
tools: tscomp tileopt prof2sram

tscomp:
	gcc tools/tscomp/tscomp.c -o tscomp

tileopt:
	gcc tools/tileopt/tileopt.c -lSDL2 -lSDL2_image -o tileopt

prof2sram:
	gcc tools/prof2sram/prof2sram.c -o prof2sram

clean:
	rm -f $(RESOURCES)
	rm -f doukutsu.bin doukutsu.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.bin
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h
