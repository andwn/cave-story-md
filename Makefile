ifdef MARSDEV
	MDROOT ?= $(MARSDEV)/m68k-elf
else ifeq ($(shell which m68k-elf-gcc),)
	MDROOT ?= /opt/toolchains/mars/m68k-elf
else
	MDROOT ?= $(subst /bin/m68k-elf-gcc,,$(shell which m68k-elf-gcc))
endif

MDBIN  := $(MDROOT)/bin
MDLIB  := $(MDROOT)/lib
MDINC  := $(MDROOT)/include

CC   := $(MDBIN)/m68k-elf-gcc
AS   := $(MDBIN)/m68k-elf-as
LD   := $(MDBIN)/m68k-elf-ld
NM   := $(MDBIN)/m68k-elf-nm
OBJC := $(MDBIN)/m68k-elf-objcopy

# For building native code (tools)
HOSTCC  ?= cc
HOSTCXX ?= c++

GCCVER := $(shell $(CC) -dumpversion)
PLUGIN  := $(MDROOT)/libexec/gcc/m68k-elf/$(GCCVER)

# Tools
ASMZ80   := bin/sjasm
BINTOS   := bin/bintos
RESCOMP  := bin/rescomp
WAVTORAW := bin/wavtoraw
XGMTOOL  := bin/xgmtool
MDTILER  := bin/mdtiler
UFTC     := bin/uftc
SALVADOR := bin/salvador
MDLOADER := bin/mdloader

HPPGEN   := bin/hppgen
PATCHROM := bin/patchrom
TSCOMP   := bin/tscomp

BUILD_DIR := build
TARGET    := doukutsu

INCS     = -Isrc -Ires -I$(BUILD_DIR)/src -I$(BUILD_DIR)/res
CCFLAGS  = -m68000 -mshort -std=c23 -Ofast
CCFLAGS += -ffreestanding --embed-dir=$(BUILD_DIR)/res
OPTIONS  = -frename-registers -fshort-enums
OPTIONS += -ffunction-sections -fdata-sections
OPTIONS += -fno-tree-loop-ivcanon -fno-ivopts
OPTIONS += -fno-web -fno-builtin -fno-gcse
WARNINGS = -Wall -Wextra -Wshadow -Wundef -Wcast-qual
WARNINGS+= -Wstack-usage=1024 -Wwrite-strings
WARNINGS+= -Wunsafe-loop-optimizations
WARNINGS+= -Wno-unused-function -fanalyzer
ASFLAGS  = -m68000 --register-prefix-optional --bitwise-or
ASFLAGS += -Isrc/md -I$(BUILD_DIR)
LDFLAGS  = -T md.ld -nostdlib -Xlinker -Map=$(TARGET).map
LDFLAGS += -Wl,--gc-sections
Z80FLAGS = -isrc/xgm

# Stage layout files to compress
PXMS   = $(wildcard res/Stage/*.pxm)
CPXMS  = $(addprefix $(BUILD_DIR)/,$(PXMS:.pxm=.cpxm))

# Tilesets to convert without compression
TSETS  = $(wildcard res/tiles/*.png)
TSETO  = $(addprefix $(BUILD_DIR)/,$(TSETS:.png=.pat))

# Tilesets to convert and compress
CTSETS  = $(wildcard res/tiles_c/*.png)
CTSETS += $(wildcard res/tiles_loc/*.png)
CTSETO  = $(addprefix $(BUILD_DIR)/,$(CTSETS:.png=.uftc))

# Sprites to convert without compression
SPRS  = $(wildcard res/sprite/*.png)
SPRS += $(wildcard res/sprite_loc/*.png)
SPRO  = $(addprefix $(BUILD_DIR)/,$(SPRS:.png=.spr))

# Sprites to convert and compress
CSPRS  = $(wildcard res/sprite_c/*.png)
CSPRO  = $(addprefix $(BUILD_DIR)/,$(CSPRS:.png=.cspr))

# TSC to convert to TSB
TSCS   = $(wildcard res/tsc/en/*.txt)
TSCS  += $(wildcard res/tsc/en/Stage/*.txt)
TSBS   = $(addprefix $(BUILD_DIR)/,$(TSCS:.txt=.tsb))

# TSBs for translations
TL_TSCS  = $(wildcard res/tsc/*/*.txt)
TL_TSCS += $(wildcard res/tsc/*/Stage/*.txt)
TL_TSBS  = $(addprefix $(BUILD_DIR)/,$(TL_TSCS:.txt=.tsb))

# mdtiler scripts to generate tile patterns & mappings
MDTS  = $(wildcard res/*.mdt)
MDTS += $(wildcard res/*/*.mdt)
PATS  = $(addprefix $(BUILD_DIR)/,$(MDTS:.mdt=.pat))
MAPS  = $(addprefix $(BUILD_DIR)/,$(MDTS:.mdt=.map))

# Script to generate palettes
PALS  = res/pal/palettes.mdt
PALO  = $(addprefix $(BUILD_DIR)/,$(PALS:.mdt=.pal))

# VGM files to convert to XGC
VGMS  = $(wildcard res/bgm/*.vgm)
XGCS  = $(addprefix $(BUILD_DIR)/,$(VGMS:.vgm=.xgc))

# WAV files to convert to raw PCM
WAVS  = $(wildcard res/sfx/*.wav)
PCMS  = $(addprefix $(BUILD_DIR)/,$(WAVS:.wav=.pcm))

CS    = $(wildcard src/*.c)
CS   += $(wildcard src/ai/*.c)
CS   += $(wildcard src/db/*.c)
CS   += $(wildcard src/md/*.c)
SS    = $(wildcard src/*.s)
SS   += $(wildcard src/md/*.s)
SS   += $(wildcard src/xgm/*.s)
SS   += $(wildcard src/res/*.s)
OBJS  = $(addprefix $(BUILD_DIR)/,$(CS:.c=.o))
OBJS += $(addprefix $(BUILD_DIR)/,$(SS:.s=.o))

RESS  = res/resources.res
RESO  = $(addprefix $(BUILD_DIR)/,$(RESS:.res=.o))

# Z80 source for XGM driver
ZSRC  = $(wildcard src/xgm/*.s80)
ZOBJ  = $(addprefix $(BUILD_DIR)/,$(ZSRC:.s80=.o80))

ASMO  = $(addprefix $(BUILD_DIR)/,$(RESS:.res=.o))
ASMO += $(addprefix $(BUILD_DIR)/,$(Z80S:.s80=.o))
ASMO += $(CS:%.c=asmout/%.s)

.SECONDARY:

.PHONY: all sega profile release asm debug translate
all: release
sega: release

mi68: OPTIONS += -DSHOW_MYCOM_LOGO
mi68: release

profile: OPTIONS += -flto=auto -DPROFILE
profile: release

release: OPTIONS += -flto=auto
release: $(TARGET)-en.gen

asm: OPTIONS += -fverbose-asm
asm: $(ASMO)

debug: OPTIONS += -flto=auto -DDEBUG -DKDEBUG
debug: $(TARGET)-en.gen

translate: $(TARGET)-es.gen $(TARGET)-fr.gen $(TARGET)-de.gen $(TARGET)-it.gen
translate: $(TARGET)-pt.gen $(TARGET)-br.gen $(TARGET)-ja.gen $(TARGET)-zh.gen
translate: $(TARGET)-ko.gen $(TARGET)-fi.gen $(TARGET)-ru.gen

assets.d: $(CPXMS) $(CPXES) $(CPXAS)
assets.d: $(XGCS) $(PCMS) $(ZOBJ)
assets.d: $(TSBS) $(PATS)
assets.d: $(TSETO) $(CTSETO)
assets.d: $(SPRO) $(CSPRO)
assets.d: $(PALO) $(CMAPS)
	touch assets.d


# Directory rules

asmout:
	mkdir -p asmout/src/act
	mkdir -p asmout/src/db
	mkdir -p asmout/src/xgm
	mkdir -p asmout/src/md

bin:
	mkdir -p bin

.PRECIOUS: $(BUILD_DIR)/ $(BUILD_DIR)%/

$(BUILD_DIR)/:
	mkdir -p $@

$(BUILD_DIR)%/:
	mkdir -p $@

.SECONDEXPANSION:


# Rules for compilation / assembling / linking

%.gen: $(BUILD_DIR)/%.elf
	$(OBJC) -O binary $< $(BUILD_DIR)/temp.bin
	dd if=$(BUILD_DIR)/temp.bin of=$@ bs=512K conv=sync
	rm -f $(BUILD_DIR)/temp.bin
	$(NM) --plugin=$(PLUGIN)/liblto_plugin.so -n $< > $*.lst

$(BUILD_DIR)/%.elf: $(OBJS) | $$(@D)/
	$(CC) -o $@ $(LDFLAGS) $(OPTIONS) $(RESO) $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/src/ai_gen.h $(RESO) assets.d $$(@D)/
	$(CC) $(CCFLAGS) $(OPTIONS) $(WARNINGS) $(INCS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | assets.d $$(@D)/
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.s: %.res | $(RESCOMP) $(SALVADOR) $$(@D)/
	$(RESCOMP) $< $@

# Special case for rescomp output
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	$(AS) -m68000 $(INCS) -c $< -o $@

$(BUILD_DIR)/%.o80: %.s80 | $(ASMZ80) $$(@D)/
	$(ASMZ80) $(Z80FLAGS) $< $@ z80_xgm.lst


# Inspectable assembly output for asm target

asmout/%.s: %.c | $(BUILD_DIR)/src/ai_gen.h $(RESO) assets.d asmout
	$(CC) $(CCFLAGS) $(OPTIONS) $(INCS) -S $< -o $@


# Tool building rules

$(ASMZ80): | bin
	$(HOSTCXX) -w -DMAX_PATH=MAXPATHLEN tools/sjasm/*.cpp -o $(ASMZ80)

$(BINTOS): | bin
	$(HOSTCC) tools/bintos.c -o $@

$(RESCOMP): | bin
	$(HOSTCC) tools/rescomp/src/*.c -Itools/rescomp/inc -o $@

$(XGMTOOL): | bin
	$(HOSTCC) tools/xgmtool/src/*.c -Itools/xgmtool/inc -o $@ -lm

$(WAVTORAW): | bin
	$(HOSTCC) tools/wavtoraw.c -o $@ -lm

$(MDTILER): | bin
	$(HOSTCC) tools/mdtiler/*.c -o $(MDTILER) -lpng

$(SALVADOR): | bin
	$(HOSTCC) tools/salvador/*.c -o $(SALVADOR)

$(UFTC): | bin
	$(HOSTCC) tools/uftc/*.c -o $(UFTC)

$(TSCOMP): | bin
	$(HOSTCC) $(WARNINGS) tools/tscomp/tscomp.c -o $@

$(PATCHROM): | bin
	$(HOSTCC) $(WARNINGS) tools/patchrom.c -o $@

$(MDLOADER): | bin
	$(HOSTCC) $(WARNINGS) tools/mdloader.c -o $@

$(HPPGEN): | bin
	$(HOSTCC) $(WARNINGS) tools/hppgen.c -o $@


# Compression of stage layouts
$(BUILD_DIR)/%.cpxm: %.pxm | $(SALVADOR) $$(@D)/
	$(SALVADOR) -c "$<" "$@"

$(BUILD_DIR)/%.pat: %.mdt | $(MDTILER) $$(@D)/
	$(MDTILER) -b "$<"

# Compression of tilesets
$(BUILD_DIR)/%.uftc: %.pat | $(UFTC) $$(@D)/
	$(UFTC) -c "$<" "$@"

%.uftc: %.pat | $(UFTC) $$(@D)/
	$(UFTC) -c "$<" "$@"

%.cspr: %.spr | $(UFTC) $$(@D)/
	$(UFTC) -c "$<" "$@"

$(BUILD_DIR)/%.pat: %.png | $(MDTILER) $$(@D)/
	$(MDTILER) -t "$<" "$@"

$(BUILD_DIR)/%.spr: %.png | $(MDTILER) $$(@D)/
	$(MDTILER) -s "$<" "$@"

$(BUILD_DIR)/%.pal: %.mdt | $(MDTILER) $$(@D)/
	$(MDTILER) -b "$<"

# Convert VGM
$(BUILD_DIR)/%.xgc: %.vgm | $(XGMTOOL) $$(@D)/
	$(XGMTOOL) "$<" "$@" -s

# Convert WAV
$(BUILD_DIR)/%.pcm: %.wav | $(WAVTORAW) $$(@D)/
	$(WAVTORAW) "$<" "$@" 14000

# Convert TSC
$(BUILD_DIR)/res/tsc/en/%.tsb: res/tsc/en/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=en "$<" "$@"
$(BUILD_DIR)/res/tsc/ja/%.tsb: res/tsc/ja/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=ja "$<" "$@"
$(BUILD_DIR)/res/tsc/es/%.tsb: res/tsc/es/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=es "$<" "$@"
$(BUILD_DIR)/res/tsc/pt/%.tsb: res/tsc/pt/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=pt "$<" "$@"
$(BUILD_DIR)/res/tsc/fr/%.tsb: res/tsc/fr/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=fr "$<" "$@"
$(BUILD_DIR)/res/tsc/it/%.tsb: res/tsc/it/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=it "$<" "$@"
$(BUILD_DIR)/res/tsc/de/%.tsb: res/tsc/de/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=de "$<" "$@"
$(BUILD_DIR)/res/tsc/br/%.tsb: res/tsc/br/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=br "$<" "$@"
$(BUILD_DIR)/res/tsc/fi/%.tsb: res/tsc/fi/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=fi "$<" "$@"
$(BUILD_DIR)/res/tsc/zh/%.tsb: res/tsc/zh/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=zh "$<" "$@"
$(BUILD_DIR)/res/tsc/ko/%.tsb: res/tsc/ko/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=ko "$<" "$@"
$(BUILD_DIR)/res/tsc/ru/%.tsb: res/tsc/ru/%.txt | $(TSCOMP) $$(@D)/
	$(TSCOMP) -l=ru "$<" "$@"

# Generate patches
$(BUILD_DIR)/res/patches/$(TARGET)-%.patch: res/patches/$(TARGET)-%.s | $$(@D)/
	$(AS) $(ASFLAGS) "$<" -o "temp.o"
	$(LD) -T md.ld -nostdlib "temp.o" -o "temp.elf"
	$(OBJC) -O binary "temp.elf" "$@"

# Apply patches
$(TARGET)-ja.gen: res/patches/$(TARGET)-ja.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-es.gen: res/patches/$(TARGET)-es.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-fr.gen: res/patches/$(TARGET)-fr.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-de.gen: res/patches/$(TARGET)-de.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-it.gen: res/patches/$(TARGET)-it.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-pt.gen: res/patches/$(TARGET)-pt.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-br.gen: res/patches/$(TARGET)-br.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-fi.gen: res/patches/$(TARGET)-fi.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-zh.gen: res/patches/$(TARGET)-zh.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-ko.gen: res/patches/$(TARGET)-ko.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"
$(TARGET)-ru.gen: res/patches/$(TARGET)-ru.patch | $(PATCHROM) $(TL_TSBS) $(TARGET)-en.gen
	$(PATCHROM) $(TARGET)-en.gen "$<" "$@"

.PHONY: flash clean

AICS  = $(wildcard src/ai/*.c)

$(BUILD_DIR)/src/ai_gen.h: $(AICS) | $(HPPGEN) $$(@D)/
	$(HPPGEN) $@ $^

flash: $(MDLOADER)
	sudo $(MDLOADER) md $(TARGET)-en.gen /dev/ttyUSB0 2> /dev/null

clean:
	-rm -r build/
	-rm -r bin/
	-rm -r asmout/
	-rm assets.d
