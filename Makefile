GENDEV?=/opt/toolchains/gen
GENGCC_BIN=$(GENDEV)/m68k-elf/bin/m68k-elf-
GENBIN=$(GENDEV)/bin/

CC = $(GENGCC_BIN)gcc
AS = $(GENGCC_BIN)as
LD = $(GENGCC_BIN)ld
OBJC = $(GENGCC_BIN)objcopy
RESCOMP= $(GENBIN)rescomp

INCS = -I$(GENDEV)/m68k-elf/include -I$(GENDEV)/m68k-elf/m68k-elf/include -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -Wextra -Wno-missing-field-initializers -O3 -std=c99 -c -fomit-frame-pointer -fno-builtin
ASFLAGS = -m68000 --register-prefix-optional
LIBS = -L$(GENDEV)/m68k-elf/lib -L$(GENDEV)/m68k-elf/m68k-elf/lib -lmd -lnosys 
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/libmd.a $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/*/libgcc.a 

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

.PHONY: all clean
.SECONDARY: doukutsu.elf

all: head-gen doukutsu.bin 

src/boot/sega.o: out/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync

%.elf: $(OBJS) $(BOOT_RESOURCES)
	$(CC) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(ARCHIVES) $(OBJS) $(LIBS)

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

clean:
	rm -f $(RESOURCES)
	rm -f *.o *.bin *.elf *.map
	rm -f src/boot/*.o src/boot/*.bin
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h
