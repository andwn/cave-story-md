GENDEV?=/opt/toolchains/gen
GENGCC_BIN=$(GENDEV)/m68k-elf/bin/m68k-elf-
GENBIN=$(GENDEV)/bin

AR= $(GENGCC_BIN)ar
CC= $(GENGCC_BIN)gcc
ASMZ80= $(GENBIN)/sjasm
BINTOS= bin/bintos
RESCOMP= bin/rescomp
XGMTOOL= bin/xgmtool
WAVTORAW= bin/wavtoraw

SRC_LIB_C= $(wildcard src/*.c)
SRC_LIB_S= $(wildcard src/*.s)
SRC_LIB_S80= $(wildcard src/*.s80)

RES_LIB_RES= $(wildcard res/*.res)

OBJ_LIB= $(RES_LIB_RES:.res=.o)
OBJ_LIB+= $(SRC_LIB_S80:.s80=.o)
OBJ_LIB+= $(SRC_LIB_S:.s=.o)
OBJ_LIB+= $(SRC_LIB_C:.c=.o)

INCS_LIB= -Iinc -Isrc -Ires
DEFAULT_FLAGS_LIB= -m68000 -Wall -std=gnu99 -fno-builtin $(INCS_LIB)
FLAGSZ80_LIB= -isrc

all: tools-build
all: release

release: FLAGS_LIB= $(DEFAULT_FLAGS_LIB) -O3
release: libmd.a

debug: FLAGS_LIB= $(DEFAULT_FLAGS_LIB) -O1 -ggdb -DDEBUG=1
debug: libmd_debug.a

libmd.a: cmd_
	$(AR) rs libmd.a @cmd_
	rm cmd_

libmd_debug.a: cmd_
	$(AR) rs libmd_debug.a @cmd_
	rm cmd_

cmd_ : $(OBJ_LIB)
	echo "$(OBJ_LIB)" > cmd_

%.o: %.c
	$(CC) $(FLAGS_LIB) -c $< -o $@

%.o: %.s
	$(CC) $(FLAGS_LIB) -c $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

%.o80: %.s80
	$(ASMZ80) $(FLAGSZ80_LIB) $< $@ out.lst

%.s: %.o80
	$(BINTOS) $<

clean:
	rm -f $(OBJ_LIB)
	rm -f libmd.a libmd_debug.a out.lst cmd_

tools-build: $(RESCOMP) $(BINTOS) $(XGMTOOL) $(WAVTORAW)

$(RESCOMP):
	gcc $(wildcard tools/rescomp/*.c) -Itools/rescomp -o $(RESCOMP)

$(XGMTOOL):
	gcc $(wildcard tools/xgmtool/*.c) -Itools/xgmtool -lm -o $(XGMTOOL)

$(BINTOS):
	gcc tools/bintos/bintos.c -o $(BINTOS)

$(WAVTORAW):
	gcc tools/wavtoraw/wavtoraw.c -lm -o $(WAVTORAW)

tools-clean:
	rm -f $(RESCOMP) $(XGMTOOL) $(BINTOS) $(WAVTORAW)
