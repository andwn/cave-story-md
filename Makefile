MARSDEV ?= ${HOME}/mars
MARSBIN  = $(MARSDEV)/m68k-elf/bin
TOOLSBIN = $(MARSDEV)/bin

TARGET = doukutsu

CC   = $(MARSBIN)/m68k-elf-gcc
AS   = $(MARSBIN)/m68k-elf-as
LD   = $(MARSBIN)/m68k-elf-ld
NM   = $(MARSBIN)/m68k-elf-nm
OBJC = $(MARSBIN)/m68k-elf-objcopy

# Z80 Assembler to build XGM driver
ASMZ80   = $(TOOLSBIN)/sjasm
# SGDK Tools
BINTOS   = bin/bintos
RESCOMP  = bin/rescomp
WAVTORAW = bin/wavtoraw
XGMTOOL  = bin/xgmtool
# Sik's Tools
MDTILER  = $(TOOLSBIN)/mdtiler
SLZ      = $(TOOLSBIN)/slz
UFTC     = $(TOOLSBIN)/uftc
# Cave Story Tools
AIGEN    = python3 tools/aigen.py
PATCHROM = bin/patchrom
TSCOMP   = bin/tscomp

# Some files needed are in a versioned directory
GCC_VER := $(shell $(CC) -dumpversion)
PLUGIN   = $(MARSDEV)/m68k-elf/libexec/gcc/m68k-elf/$(GCC_VER)
LTO_SO   = liblto_plugin.so

INCS     = -Isrc -Ires
CCFLAGS  = -m68000 -mshort -ffreestanding -fshort-enums -fno-unit-at-a-time -fcommon
OPTIONS  = -O3 -fomit-frame-pointer -fno-web -fno-gcse
WARNINGS = -Wall -Wextra -Wshadow -Wundef
ASFLAGS  = -m68000 --register-prefix-optional --bitwise-or
LDFLAGS  = -T md.ld -nostdlib
Z80FLAGS = -isrc/xgm

# Stage layout files to compress
PXMS  = $(wildcard res/Stage/*.pxm)
PXMS += $(wildcard res/Stage/*/*.pxm)
CPXMS = $(PXMS:.pxm=.cpxm)

# Tilesets to compress
TSETS  = $(wildcard res/Stage/*_vert.png)
TSETS += $(wildcard res/Stage/**/*_vert.png)
PTSETS = $(TSETS:.png=.pat)
CTSETS = $(TSETS:.png=.uftc)

# TSC to convert to TSB
TSCS  = $(wildcard res/tsc/en/*.txt)
TSCS += $(wildcard res/tsc/en/Stage/*.txt)
TSBS  = $(TSCS:.txt=.tsb)

# TSBs for translations
TL_TSCS  = $(wildcard res/tsc/*/*.txt)
TL_TSCS += $(wildcard res/tsc/*/Stage/*.txt)
TL_TSBS  = $(TL_TSCS:.txt=.tsb)

# mdtiler scripts to generate tile patterns & mappings
MDTS  = $(wildcard res/*.mdt)
MDTS += $(wildcard res/*/*.mdt)
PATS  = $(MDTS:.mdt=.pat)
MAPS  = $(MDTS:.mdt=.map)

# VGM files to convert to XGC
VGMS  = $(wildcard res/bgm/*.vgm)
XGCS  = $(VGMS:.vgm=.xgc)

# WAV files to convert to raw PCM
WAVS  = $(wildcard res/sfx/*.wav)
PCMS  = $(WAVS:.wav=.pcm)

RESS  = res/resources.res
CS    = $(wildcard src/*.c)
CS   += $(wildcard src/ai/*.c)
CS   += $(wildcard src/db/*.c)
SS    = $(wildcard src/*.s)
SS   += $(wildcard src/xgm/*.s)
OBJS  = $(RESS:.res=.o)
OBJS += $(CS:.c=.o)
OBJS += $(SS:.s=.o)

# Z80 source for XGM driver
ZSRC  = $(wildcard src/xgm/*.s80)
ZOBJ  = $(ZSRC:.s80=.o80)

ASMO  = $(RESS:.res=.o)
ASMO += $(Z80S:.s80=.o)
ASMO += $(CS:%.c=asmout/%.s)

.PHONY: all sega profile release asm debug translate prereq
all: release

sega: OPTIONS += -DSEGA_LOGO
sega: release

profile: OPTIONS += -DPROFILE
profile: release

release: OPTIONS += -flto -fuse-linker-plugin
release: prereq head-gen $(TARGET)-en.bin $(TARGET)-en.lst

asm: OPTIONS += -fverbose-asm
asm: prereq head-gen asm-dir $(PATS) $(ASMO)

debug: OPTIONS += -DDEBUG -DKDEBUG
debug: prereq head-gen $(TARGET)-en.bin $(TARGET)-en.lst

translate: $(PATCHROM) $(TL_TSBS)
translate: $(TARGET)-es.bin $(TARGET)-fr.bin $(TARGET)-de.bin $(TARGET)-it.bin
translate: $(TARGET)-pt.bin $(TARGET)-br.bin $(TARGET)-ja.bin $(TARGET)-zh.bin
translate: $(TARGET)-ko.bin

prereq: $(BINTOS) $(RESCOMP) $(XGMTOOL) $(WAVTORAW) $(TSCOMP)
prereq: $(CPXMS) $(XGCS) $(PCMS) $(CTSETS) $(ZOBJ) $(TSBS)

# Cross reference symbol list with the addresses displayed in the crash handler
%.lst: %.elf
	$(NM) --plugin=$(PLUGIN)/$(LTO_SO) -n $< > $@

%.bin: %.elf
	@echo "Stripping ELF header, pad to 512K"
	@$(OBJC) -O binary $< temp.bin
	@dd if=temp.bin of=$@ bs=512K conv=sync
	@rm -f temp.bin

%.elf: $(PATS) $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CCFLAGS) $(OPTIONS) $(WARNINGS) $(INCS) -c $< -o $@

%.o: %.s 
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

%.o80: %.s80
	$(ASMZ80) $(Z80FLAGS) $< $@ z80_xgm.lst

# Old SGDK tools
bin:
	mkdir -p bin

$(BINTOS): bin
	cc tools/bintos.c -o $@
	
$(RESCOMP): bin
	cc tools/rescomp/src/*.c -Itools/rescomp/inc -o $@

$(XGMTOOL): bin
	cc tools/xgmtool/src/*.c -Itools/xgmtool/inc -o $@ -lm

$(WAVTORAW): bin
	cc tools/wavtoraw.c -o $@ -lm

# Cave Story tools
$(TSCOMP): bin
	cc tools/tscomp/tscomp.c -o $@

$(PATCHROM): bin
	cc tools/patchrom.c -o $@

$(HPPGEN): bin
	cc tools/hppgen.c -o $@

# For asm target
asm-dir:
	mkdir -p asmout/src/{ai,db,xgm}

asmout/%.s: %.c
	$(CC) $(CCFLAGS) $(OPTIONS) $(INCS) -S $< -o $@

# Compression of stage layouts
%.cpxm: %.pxm
	$(SLZ) -c "$<" "$@"

%.pat: %.mdt
	$(MDTILER) -b "$<"

# Compression of tilesets
%.uftc: %.pat
	$(UFTC) -c "$<" "$@"

%.pat: %.png
	$(MDTILER) -t "$<" "$@"

# Convert VGM
%.xgc: %.vgm
	$(XGMTOOL) "$<" "$@" -s

# Convert WAV
%.pcm: %.wav
	$(WAVTORAW) "$<" "$@" 14000

# Convert TSC
res/tsc/en/%.tsb: res/tsc/en/%.txt
	$(TSCOMP) -l=en "$<"
res/tsc/ja/%.tsb: res/tsc/ja/%.txt
	$(TSCOMP) -l=ja "$<"
res/tsc/es/%.tsb: res/tsc/es/%.txt
	$(TSCOMP) -l=es "$<"
res/tsc/pt/%.tsb: res/tsc/pt/%.txt
	$(TSCOMP) -l=pt "$<"
res/tsc/fr/%.tsb: res/tsc/fr/%.txt
	$(TSCOMP) -l=fr "$<"
res/tsc/it/%.tsb: res/tsc/it/%.txt
	$(TSCOMP) -l=it "$<"
res/tsc/de/%.tsb: res/tsc/de/%.txt
	$(TSCOMP) -l=de "$<"
res/tsc/br/%.tsb: res/tsc/br/%.txt
	$(TSCOMP) -l=br "$<"
res/tsc/zh/%.tsb: res/tsc/zh/%.txt
	$(TSCOMP) -l=zh "$<"
res/tsc/ko/%.tsb: res/tsc/ko/%.txt
	$(TSCOMP) -l=ko "$<"

# Generate patches
res/patches/$(TARGET)-%.patch: res/patches/$(TARGET)-%.s
	$(AS) $(ASFLAGS) "$<" -o "temp.o"
	$(LD) $(LDFLAGS) "temp.o" -o "temp.elf"
	$(OBJC) -O binary "temp.elf" "$@"

# Apply patches
$(TARGET)-ja.bin: res/patches/$(TARGET)-ja.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-es.bin: res/patches/$(TARGET)-es.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-fr.bin: res/patches/$(TARGET)-fr.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-de.bin: res/patches/$(TARGET)-de.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-it.bin: res/patches/$(TARGET)-it.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-pt.bin: res/patches/$(TARGET)-pt.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-br.bin: res/patches/$(TARGET)-br.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-zh.bin: res/patches/$(TARGET)-zh.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"
$(TARGET)-ko.bin: res/patches/$(TARGET)-ko.patch
	$(PATCHROM) $(TARGET)-en.bin "$<" "$@"

.PHONY: head-gen clean
head-gen:
	rm -f src/ai_gen.h
	$(AIGEN) src/ai/ src/ai_gen.h

clean:
	@rm -f $(CPXMS) $(XGCS) $(PCMS) $(PATS) $(MAPS) $(PTSETS) $(CTSETS) $(ZOBJ) $(OBJS)
	@rm -f $(TSBS) $(TL_TSBS)
	@rm -f $(TARGET)-*.bin $(TARGET)-en.elf $(TARGET)-en.lst temp.elf temp.o
	@rm -f res/patches/*.patch
	@rm -f src/xgm/z80_xgm.s src/xgm/z80_xgm.o80 src/xgm/z80_xgm.h z80_xgm.lst
	@rm -f res/resources.h res/resources.s src/ai_gen.h
	@rm -rf asmout
