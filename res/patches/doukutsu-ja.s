.macro BIN _name, _file, _align=2
    .globl \_name
    .align \_align
\_name:
    .incbin "\_file"
.endm

.section .text

/* Start of localizable data */
.ascii "LANGDAT\0"

    .globl LANGUAGE
LANGUAGE:
.ascii "JA\0\0"

    .globl BMP_ASCII
BMP_ASCII:
dc.l BMP_Ascii
    .globl BMP_KANJI
BMP_KANJI:
dc.l BMP_Kanji

    .globl STAGE_NAMES
STAGE_NAMES:
dc.l STR_StageNames
    .globl CREDITS_STR
CREDITS_STR:
dc.l STR_Credits
    .globl CONFIG_STR
CONFIG_STR:
dc.l STR_Config

/* Pointer Tables */
    .globl TSC_GLOB
TSC_GLOB:
dc.l TSC_ArmsItem
dc.l TSC_Head
dc.l TSC_StageSelect
dc.l TSC_Credits
    .globl TSC_STAGE
TSC_STAGE:
dc.l 0
dc.l TSC_Pens1
dc.l TSC_Eggs
dc.l TSC_EggX
dc.l TSC_Egg6
dc.l TSC_EggR
dc.l TSC_Weed
dc.l TSC_Santa
dc.l TSC_Chako
dc.l TSC_MazeI
dc.l TSC_Sand
dc.l TSC_Mimi
dc.l TSC_Cave
dc.l TSC_Start
dc.l TSC_Barr
dc.l TSC_Pool
dc.l TSC_Cemet
dc.l TSC_Plant
dc.l TSC_Shelt
dc.l TSC_Comu
dc.l TSC_MiBox
dc.l TSC_EgEnd1
dc.l TSC_Cthu
dc.l TSC_Egg1
dc.l TSC_Pens2
dc.l TSC_Malco
dc.l TSC_WeedS
dc.l TSC_WeedD
dc.l TSC_Frog
dc.l TSC_Curly
dc.l TSC_WeedB
dc.l TSC_Stream
dc.l TSC_CurlyS
dc.l TSC_Jenka1
dc.l TSC_Dark
dc.l TSC_Gard
dc.l TSC_Jenka2
dc.l TSC_SandE
dc.l TSC_MazeH
dc.l TSC_MazeW
dc.l TSC_MazeO
dc.l TSC_MazeD
dc.l TSC_MazeA
dc.l TSC_MazeB
dc.l TSC_MazeS
dc.l TSC_MazeM
dc.l TSC_Drain
dc.l TSC_Almond
dc.l TSC_River
dc.l TSC_Eggs2
dc.l TSC_Cthu2
dc.l TSC_EggR2
dc.l TSC_EggX2
dc.l TSC_Oside
dc.l TSC_EgEnd2
dc.l TSC_Itoh
dc.l TSC_Cent
dc.l TSC_Jail1
dc.l TSC_Momo
dc.l TSC_Lounge
dc.l TSC_CentW
dc.l TSC_Jail2
dc.l TSC_Blcny1
dc.l TSC_Priso1
dc.l TSC_Ring1
dc.l TSC_Ring2
dc.l TSC_Prefa1
dc.l TSC_Priso2
dc.l TSC_Ring3
dc.l TSC_Little
dc.l TSC_Blcny2
dc.l TSC_Fall
dc.l TSC_Kings
dc.l TSC_Pixel
dc.l TSC_e_Maze
dc.l TSC_e_Jenk
dc.l TSC_e_Malc
dc.l TSC_e_Ceme
dc.l TSC_e_Sky
dc.l TSC_Prefa2
dc.l TSC_Hell1
dc.l TSC_Hell2
dc.l TSC_Hell3
dc.l TSC_Mapi
dc.l TSC_Hell4
dc.l TSC_Hell42
dc.l TSC_Statue
dc.l TSC_Ballo1
dc.l TSC_Ostep
dc.l TSC_e_Labo
dc.l TSC_Pole
dc.l TSC_Island
dc.l TSC_Ballo2
dc.l TSC_e_Blcn
dc.l TSC_Clock

# 1bpp CJK fonts
BIN BMP_Ascii   "res/cjk_ascii.dat"
BIN BMP_Kanji   "res/cjk_ja_zpix.dat"

# Strings
BIN STR_StageNames  "res/strings/ja_stagename.dat"
BIN STR_Credits     "res/strings/ja_credits.dat"
BIN STR_Config      "res/strings/ja_config.dat"

# Scripts (TSC)
# Global
BIN TSC_ArmsItem	"res/tsc/ja/ArmsItem.tsb"
BIN TSC_Head		"res/tsc/ja/Head.tsb"
BIN TSC_StageSelect	"res/tsc/ja/StageSelect.tsb"
BIN TSC_Credits		"res/tsc/en/Stage/0.tsb"
# Stage Specific
BIN TSC_Almond	"res/tsc/ja/Stage/Almond.tsb"
BIN TSC_Ballo1	"res/tsc/ja/Stage/Ballo1.tsb"
BIN TSC_Ballo2	"res/tsc/ja/Stage/Ballo2.tsb"
BIN TSC_Barr	"res/tsc/ja/Stage/Barr.tsb"
BIN TSC_Blcny1	"res/tsc/ja/Stage/Blcny1.tsb"
BIN TSC_Blcny2	"res/tsc/ja/Stage/Blcny2.tsb"
BIN TSC_Cave	"res/tsc/ja/Stage/Cave.tsb"
BIN TSC_Cemet	"res/tsc/ja/Stage/Cemet.tsb"
BIN TSC_Cent	"res/tsc/ja/Stage/Cent.tsb"
BIN TSC_CentW	"res/tsc/ja/Stage/CentW.tsb"
BIN TSC_Chako	"res/tsc/ja/Stage/Chako.tsb"
BIN TSC_Clock	"res/tsc/ja/Stage/Clock.tsb"
BIN TSC_Comu	"res/tsc/ja/Stage/Comu.tsb"
BIN TSC_Cthu	"res/tsc/ja/Stage/Cthu.tsb"
BIN TSC_Cthu2	"res/tsc/ja/Stage/Cthu2.tsb"
BIN TSC_Curly	"res/tsc/ja/Stage/Curly.tsb"
BIN TSC_CurlyS	"res/tsc/ja/Stage/CurlyS.tsb"
BIN TSC_Dark	"res/tsc/ja/Stage/Dark.tsb"
BIN TSC_Drain	"res/tsc/ja/Stage/Drain.tsb"
BIN TSC_e_Blcn	"res/tsc/ja/Stage/e_Blcn.tsb"
BIN TSC_e_Ceme	"res/tsc/ja/Stage/e_Ceme.tsb"
BIN TSC_e_Jenk	"res/tsc/ja/Stage/e_Jenk.tsb"
BIN TSC_e_Labo	"res/tsc/ja/Stage/e_Labo.tsb"
BIN TSC_e_Malc	"res/tsc/ja/Stage/e_Malc.tsb"
BIN TSC_e_Maze	"res/tsc/ja/Stage/e_Maze.tsb"
BIN TSC_e_Sky	"res/tsc/ja/Stage/e_Sky.tsb"
BIN TSC_EgEnd1	"res/tsc/ja/Stage/EgEnd1.tsb"
BIN TSC_EgEnd2	"res/tsc/ja/Stage/EgEnd2.tsb"
BIN TSC_Egg1	"res/tsc/ja/Stage/Egg1.tsb"
BIN TSC_Egg6	"res/tsc/ja/Stage/Egg6.tsb"
BIN TSC_EggR	"res/tsc/ja/Stage/EggR.tsb"
BIN TSC_EggR2	"res/tsc/ja/Stage/EggR2.tsb"
BIN TSC_Eggs	"res/tsc/ja/Stage/Eggs.tsb"
BIN TSC_Eggs2	"res/tsc/ja/Stage/Eggs2.tsb"
BIN TSC_EggX	"res/tsc/ja/Stage/EggX.tsb"
BIN TSC_EggX2	"res/tsc/ja/Stage/EggX2.tsb"
BIN TSC_Fall	"res/tsc/ja/Stage/Fall.tsb"
BIN TSC_Frog	"res/tsc/ja/Stage/Frog.tsb"
BIN TSC_Gard	"res/tsc/ja/Stage/Gard.tsb"
BIN TSC_Hell1	"res/tsc/ja/Stage/Hell1.tsb"
BIN TSC_Hell2	"res/tsc/ja/Stage/Hell2.tsb"
BIN TSC_Hell3	"res/tsc/ja/Stage/Hell3.tsb"
BIN TSC_Hell4	"res/tsc/ja/Stage/Hell4.tsb"
BIN TSC_Hell42	"res/tsc/ja/Stage/Hell42.tsb"
BIN TSC_Island	"res/tsc/ja/Stage/Island.tsb"
BIN TSC_Itoh	"res/tsc/ja/Stage/Itoh.tsb"
BIN TSC_Jail1	"res/tsc/ja/Stage/Jail1.tsb"
BIN TSC_Jail2	"res/tsc/ja/Stage/Jail2.tsb"
BIN TSC_Jenka1	"res/tsc/ja/Stage/Jenka1.tsb"
BIN TSC_Jenka2	"res/tsc/ja/Stage/Jenka2.tsb"
BIN TSC_Kings	"res/tsc/ja/Stage/Kings.tsb"
BIN TSC_Little	"res/tsc/ja/Stage/Little.tsb"
BIN TSC_Lounge	"res/tsc/ja/Stage/Lounge.tsb"
BIN TSC_Malco	"res/tsc/ja/Stage/Malco.tsb"
BIN TSC_Mapi	"res/tsc/ja/Stage/Mapi.tsb"
BIN TSC_MazeA	"res/tsc/ja/Stage/MazeA.tsb"
BIN TSC_MazeB	"res/tsc/ja/Stage/MazeB.tsb"
BIN TSC_MazeD	"res/tsc/ja/Stage/MazeD.tsb"
BIN TSC_MazeH	"res/tsc/ja/Stage/MazeH.tsb"
BIN TSC_MazeI	"res/tsc/ja/Stage/MazeI.tsb"
BIN TSC_MazeM	"res/tsc/ja/Stage/MazeM.tsb"
BIN TSC_MazeO	"res/tsc/ja/Stage/MazeO.tsb"
BIN TSC_MazeS	"res/tsc/ja/Stage/MazeS.tsb"
BIN TSC_MazeW	"res/tsc/ja/Stage/MazeW.tsb"
BIN TSC_MiBox	"res/tsc/ja/Stage/MiBox.tsb"
BIN TSC_Mimi	"res/tsc/ja/Stage/Mimi.tsb"
BIN TSC_Momo	"res/tsc/ja/Stage/Momo.tsb"
BIN TSC_Oside	"res/tsc/ja/Stage/Oside.tsb"
BIN TSC_Ostep	"res/tsc/ja/Stage/Ostep.tsb"
BIN TSC_Pens1	"res/tsc/ja/Stage/Pens1.tsb"
BIN TSC_Pens2	"res/tsc/ja/Stage/Pens2.tsb"
BIN TSC_Pixel	"res/tsc/ja/Stage/Pixel.tsb"
BIN TSC_Plant	"res/tsc/ja/Stage/Plant.tsb"
BIN TSC_Pole	"res/tsc/ja/Stage/Pole.tsb"
BIN TSC_Pool	"res/tsc/ja/Stage/Pool.tsb"
BIN TSC_Prefa1	"res/tsc/ja/Stage/Prefa1.tsb"
BIN TSC_Prefa2	"res/tsc/ja/Stage/Prefa2.tsb"
BIN TSC_Priso1	"res/tsc/ja/Stage/Priso1.tsb"
BIN TSC_Priso2	"res/tsc/ja/Stage/Priso2.tsb"
BIN TSC_Ring1	"res/tsc/ja/Stage/Ring1.tsb"
BIN TSC_Ring2	"res/tsc/ja/Stage/Ring2.tsb"
BIN TSC_Ring3	"res/tsc/ja/Stage/Ring3.tsb"
BIN TSC_River	"res/tsc/ja/Stage/River.tsb"
BIN TSC_Sand	"res/tsc/ja/Stage/Sand.tsb"
BIN TSC_SandE	"res/tsc/ja/Stage/SandE.tsb"
BIN TSC_Santa	"res/tsc/ja/Stage/Santa.tsb"
BIN TSC_Shelt	"res/tsc/ja/Stage/Shelt.tsb"
BIN TSC_Start	"res/tsc/ja/Stage/Start.tsb"
BIN TSC_Statue	"res/tsc/ja/Stage/Statue.tsb"
BIN TSC_Stream	"res/tsc/ja/Stage/Stream.tsb"
BIN TSC_Weed	"res/tsc/ja/Stage/Weed.tsb"
BIN TSC_WeedB	"res/tsc/ja/Stage/WeedB.tsb"
BIN TSC_WeedD	"res/tsc/ja/Stage/WeedD.tsb"
BIN TSC_WeedS	"res/tsc/ja/Stage/WeedS.tsb"
