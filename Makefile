GENDEV?=/opt/toolchains/gen/
GCC_VER?=4.8.2
MAKE?=make

GENGCC_BIN=$(GENDEV)/m68k-elf/bin
GENBIN=$(GENDEV)/bin

CC = $(GENGCC_BIN)/m68k-elf-gcc 
AS = $(GENGCC_BIN)/m68k-elf-as
AR = $(GENGCC_BIN)/m68k-elf-ar 
LD = $(GENGCC_BIN)/m68k-elf-ld
RANLIB = $(GENGCC_BIN)/m68k-elf-ranlib 
OBJC = $(GENGCC_BIN)/m68k-elf-objcopy 
BINTOS = $(GENBIN)/bintos 
RESCOMP= $(GENBIN)/rescomp
XGMTOOL= $(GENBIN)/xgmtool
PCMTORAW = $(GENBIN)/pcmtoraw
WAVTORAW = $(GENBIN)/wavtoraw
SIZEBND = $(GENBIN)/sizebnd
ASMZ80 = $(GENBIN)/zasm

RM = rm -f 
NM = nm
NM2WCH = nm2wch

OPTION =
INCS = -I. -I$(GENDEV)/m68k-elf/include -I$(GENDEV)/m68k-elf/m68k-elf/include -Isrc -Ires -Iinc
CCFLAGS = $(OPTION) -m68000 -Wall -O2 -std=gnu99 -c -fomit-frame-pointer -fno-builtin
HWCCFLAGS = $(OPTION) -m68000 -Wall -O1 -c -fomit-frame-pointer
Z80FLAGS = -vb2
ASFLAGS = -m68000 --register-prefix-optional
LIBS =  -L$(GENDEV)/m68k-elf/lib -L$(GENDEV)/m68k-elf/lib/gcc/m68k-elf/* -L$(GENDEV)/m68k-elf/m68k-elf/lib -lmd -lnosys 
LINKFLAGS = -T $(GENDEV)/ldscripts/sgdk.ld -nostdlib 
SCDLINKFLAGS = -T scd/mdcd.ld -nostdlib 
ARCHIVES = $(GENDEV)/m68k-elf/lib/libmd.a $(GENDEV)/m68k-elf/lib/gcc/m68k-elf/$(GCC_VER)/libgcc.a 

BOOTSS=$(wildcard boot/*.s)
BOOTSS+=$(wildcard src/boot/*.s)
BOOT_RESOURCES=$(BOOTSS:.s=.o)

RESS=$(wildcard res/*.res)
RESS+=$(wildcard *.res)

CS=$(wildcard src/*.c)
CS+=$(wildcard src/ai/*.c)
CS+=$(wildcard src/db/*.c)

SS=$(wildcard src/*.s)
SS+=$(wildcard *.s)

S80S=$(wildcard src/*.s80)
S80S+=$(wildcard *.s80)

RESOURCES=$(RESS:.res=.o)
RESOURCES+=$(CS:.c=.o)
RESOURCES+=$(SS:.s=.o)
RESOURCES+=$(S80S:.s80=.o)

OBJS = $(RESOURCES)

.SECONDARY: out.elf

all: out.bin 

src/boot/sega.o: out/rom_head.bin
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

%.s: %.res
	$(RESCOMP) $< $@

out/rom_head.bin: src/boot/rom_head.o
	mkdir -p out/boot
	$(LD) $(LINKFLAGS) --oformat binary -o $@ $<
	

clean:
	$(RM) $(RESOURCES)
	$(RM) *.o *.bin *.elf *.map
	$(RM) src/boot/*.o src/boot/*.bin

