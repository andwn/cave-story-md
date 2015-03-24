CC = m68k-elf-gcc
AS = m68k-elf-as
OBJC = m68k-elf-objcopy
LD = m68k-elf-ld
RM = rm -f
ASMZ80 = zasm
BINTOS = bintos
PCMTORAW = pcmtoraw
WAVTORAW = wavtoraw
NM = nm
NM2WCH = nm2wch
SIZEBND = sizebnd
RESCOMP= rescomp

SCD_LOADER = scd/LukeProjectCD

OPTION =
INCS = -I. -I$(GENDEV)/m68k-elf/include -I$(GENDEV)/m68k-elf/m68k-elf/include -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -std=gnu99 -O2 -c -fomit-frame-pointer
HWCCFLAGS = $(OPTION) -m68000 -Wall -O1 -c -fomit-frame-pointer
Z80FLAGS = -vb2
ASFLAGS = -m68000 --register-prefix-optional
LIBS =  -L$(GENDEV)/m68k-elf/lib -L$(GENDEV)/m68k-elf/lib/gcc/m68k-elf/* -L$(GENDEV)/m68k-elf/m68k-elf/lib -lmd -lnosys 
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib
ARCHIVES = $(GENDEV)/m68k-elf/lib/libmd.a $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/*/libgcc.a

RESOURCES=
BOOT_RESOURCES=

BOOTSS=$(wildcard boot/*.s)
BOOTSS+=$(wildcard src/boot/*.s)
BOOT_RESOURCES+=$(BOOTSS:.s=.o)

RESS=$(wildcard res/*.res)
RESS+=$(wildcard *.res)

RESOURCES+=$(RESS:.res=.o)

CS=$(wildcard src/*.c)
SS=$(wildcard src/*.s)
S80S=$(wildcard src/*.s80)
CS+=$(wildcard *.c)
SS+=$(wildcard *.s)
S80S+=$(wildcard *.s80)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)
RESOURCES+=$(S80S:.s80=.o)

OBJS = $(RESOURCES)

all: out.bin 

src/boot/sega.o: src/boot/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync

%.elf: $(OBJS) $(BOOT_RESOURCES)
	$(CC) -o $@ $(LINKFLAGS) $(BOOT_RESOURCES) $(ARCHIVES) $(OBJS) $(LIBS)

%.o80: %.s80
	$(ASMZ80) $(Z80FLAGS) -o $@ $<

%.c: %.o80
	$(BINTOS) $<

%.o: %.c
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

%.o: %.s 
	$(AS) $(ASFLAGS) $< -o $@

%.s: %.bmp
	bintos -bmp $<

%.rawpcm: %.pcm
	$(PCMTORAW) $< $@

%.raw: %.wav
	$(WAVTORAW) $< $@ 16000

%.pcm: %.wavpcm
	$(WAVTORAW) $< $@ 22050

%.s: %.tfd
	$(BINTOS) -align 32768 $<

%.s: %.mvs
	$(BINTOS) -align 256 $<

%.s: %.esf
	$(BINTOS) -align 32768 $<

%.s: %.eif
	$(BINTOS) -align 256 $<

%.s: %.vgm 
	$(BINTOS) -align 256 $<

%.s: %.raw
	$(BINTOS) -align 256 -sizealign 256 $<

%.s: %.rawpcm
	$(BINTOS) -align 128 -sizealign 128 -nullfill 136 $<

%.s: %.rawpcm
	$(BINTOS) -align 128 -sizealign 128 -nullfill 136 $<

%.s: %.res
	$(RESCOMP) $< $@

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<

clean:
	$(RM) $(RESOURCES)
	$(RM) *.o *.bin *.elf *.elf_scd *.map *.iso
	$(RM) src/boot/*.o src/boot/*.bin
