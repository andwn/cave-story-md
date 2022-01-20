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
.ascii "FR\0\0"

    .globl BMP_ASCII
BMP_ASCII:
dc.l 0
    .globl BMP_KANJI
BMP_KANJI:
dc.l 0

    .globl STAGE_NAMES
STAGE_NAMES:
dc.l STR_StageNames
    .globl CREDITS_STR
CREDITS_STR:
dc.l 0
    .globl CONFIG_STR
CONFIG_STR:
dc.l 0

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

# Strings
BIN STR_StageNames  "res/strings/fr_stagename.dat"

# Scripts (TSC)
# Global
BIN TSC_ArmsItem	"res/tsc/fr/ArmsItem.tsb"
BIN TSC_Head		"res/tsc/fr/Head.tsb"
BIN TSC_StageSelect	"res/tsc/fr/StageSelect.tsb"
BIN TSC_Credits		"res/tsc/en/Stage/0.tsb"
# Stage Specific
BIN TSC_Almond	"res/tsc/fr/Stage/Almond.tsb"
BIN TSC_Ballo1	"res/tsc/fr/Stage/Ballo1.tsb"
BIN TSC_Ballo2	"res/tsc/fr/Stage/Ballo2.tsb"
BIN TSC_Barr	"res/tsc/fr/Stage/Barr.tsb"
BIN TSC_Blcny1	"res/tsc/fr/Stage/Blcny1.tsb"
BIN TSC_Blcny2	"res/tsc/fr/Stage/Blcny2.tsb"
BIN TSC_Cave	"res/tsc/fr/Stage/Cave.tsb"
BIN TSC_Cemet	"res/tsc/fr/Stage/Cemet.tsb"
BIN TSC_Cent	"res/tsc/fr/Stage/Cent.tsb"
BIN TSC_CentW	"res/tsc/fr/Stage/CentW.tsb"
BIN TSC_Chako	"res/tsc/fr/Stage/Chako.tsb"
BIN TSC_Clock	"res/tsc/fr/Stage/Clock.tsb"
BIN TSC_Comu	"res/tsc/fr/Stage/Comu.tsb"
BIN TSC_Cthu	"res/tsc/fr/Stage/Cthu.tsb"
BIN TSC_Cthu2	"res/tsc/fr/Stage/Cthu2.tsb"
BIN TSC_Curly	"res/tsc/fr/Stage/Curly.tsb"
BIN TSC_CurlyS	"res/tsc/fr/Stage/CurlyS.tsb"
BIN TSC_Dark	"res/tsc/fr/Stage/Dark.tsb"
BIN TSC_Drain	"res/tsc/fr/Stage/Drain.tsb"
BIN TSC_e_Blcn	"res/tsc/fr/Stage/e_Blcn.tsb"
BIN TSC_e_Ceme	"res/tsc/fr/Stage/e_Ceme.tsb"
BIN TSC_e_Jenk	"res/tsc/fr/Stage/e_Jenk.tsb"
BIN TSC_e_Labo	"res/tsc/fr/Stage/e_Labo.tsb"
BIN TSC_e_Malc	"res/tsc/fr/Stage/e_Malc.tsb"
BIN TSC_e_Maze	"res/tsc/fr/Stage/e_Maze.tsb"
BIN TSC_e_Sky	"res/tsc/fr/Stage/e_Sky.tsb"
BIN TSC_EgEnd1	"res/tsc/fr/Stage/EgEnd1.tsb"
BIN TSC_EgEnd2	"res/tsc/fr/Stage/EgEnd2.tsb"
BIN TSC_Egg1	"res/tsc/fr/Stage/Egg1.tsb"
BIN TSC_Egg6	"res/tsc/fr/Stage/Egg6.tsb"
BIN TSC_EggR	"res/tsc/fr/Stage/EggR.tsb"
BIN TSC_EggR2	"res/tsc/fr/Stage/EggR2.tsb"
BIN TSC_Eggs	"res/tsc/fr/Stage/Eggs.tsb"
BIN TSC_Eggs2	"res/tsc/fr/Stage/Eggs2.tsb"
BIN TSC_EggX	"res/tsc/fr/Stage/EggX.tsb"
BIN TSC_EggX2	"res/tsc/fr/Stage/EggX2.tsb"
BIN TSC_Fall	"res/tsc/fr/Stage/Fall.tsb"
BIN TSC_Frog	"res/tsc/fr/Stage/Frog.tsb"
BIN TSC_Gard	"res/tsc/fr/Stage/Gard.tsb"
BIN TSC_Hell1	"res/tsc/fr/Stage/Hell1.tsb"
BIN TSC_Hell2	"res/tsc/fr/Stage/Hell2.tsb"
BIN TSC_Hell3	"res/tsc/fr/Stage/Hell3.tsb"
BIN TSC_Hell4	"res/tsc/fr/Stage/Hell4.tsb"
BIN TSC_Hell42	"res/tsc/fr/Stage/Hell42.tsb"
BIN TSC_Island	"res/tsc/fr/Stage/Island.tsb"
BIN TSC_Itoh	"res/tsc/fr/Stage/Itoh.tsb"
BIN TSC_Jail1	"res/tsc/fr/Stage/Jail1.tsb"
BIN TSC_Jail2	"res/tsc/fr/Stage/Jail2.tsb"
BIN TSC_Jenka1	"res/tsc/fr/Stage/Jenka1.tsb"
BIN TSC_Jenka2	"res/tsc/fr/Stage/Jenka2.tsb"
BIN TSC_Kings	"res/tsc/fr/Stage/Kings.tsb"
BIN TSC_Little	"res/tsc/fr/Stage/Little.tsb"
BIN TSC_Lounge	"res/tsc/fr/Stage/Lounge.tsb"
BIN TSC_Malco	"res/tsc/fr/Stage/Malco.tsb"
BIN TSC_Mapi	"res/tsc/fr/Stage/Mapi.tsb"
BIN TSC_MazeA	"res/tsc/fr/Stage/MazeA.tsb"
BIN TSC_MazeB	"res/tsc/fr/Stage/MazeB.tsb"
BIN TSC_MazeD	"res/tsc/fr/Stage/MazeD.tsb"
BIN TSC_MazeH	"res/tsc/fr/Stage/MazeH.tsb"
BIN TSC_MazeI	"res/tsc/fr/Stage/MazeI.tsb"
BIN TSC_MazeM	"res/tsc/fr/Stage/MazeM.tsb"
BIN TSC_MazeO	"res/tsc/fr/Stage/MazeO.tsb"
BIN TSC_MazeS	"res/tsc/fr/Stage/MazeS.tsb"
BIN TSC_MazeW	"res/tsc/fr/Stage/MazeW.tsb"
BIN TSC_MiBox	"res/tsc/fr/Stage/MiBox.tsb"
BIN TSC_Mimi	"res/tsc/fr/Stage/Mimi.tsb"
BIN TSC_Momo	"res/tsc/fr/Stage/Momo.tsb"
BIN TSC_Oside	"res/tsc/fr/Stage/Oside.tsb"
BIN TSC_Ostep	"res/tsc/fr/Stage/Ostep.tsb"
BIN TSC_Pens1	"res/tsc/fr/Stage/Pens1.tsb"
BIN TSC_Pens2	"res/tsc/fr/Stage/Pens2.tsb"
BIN TSC_Pixel	"res/tsc/fr/Stage/Pixel.tsb"
BIN TSC_Plant	"res/tsc/fr/Stage/Plant.tsb"
BIN TSC_Pole	"res/tsc/fr/Stage/Pole.tsb"
BIN TSC_Pool	"res/tsc/fr/Stage/Pool.tsb"
BIN TSC_Prefa1	"res/tsc/fr/Stage/Prefa1.tsb"
BIN TSC_Prefa2	"res/tsc/fr/Stage/Prefa2.tsb"
BIN TSC_Priso1	"res/tsc/fr/Stage/Priso1.tsb"
BIN TSC_Priso2	"res/tsc/fr/Stage/Priso2.tsb"
BIN TSC_Ring1	"res/tsc/fr/Stage/Ring1.tsb"
BIN TSC_Ring2	"res/tsc/fr/Stage/Ring2.tsb"
BIN TSC_Ring3	"res/tsc/fr/Stage/Ring3.tsb"
BIN TSC_River	"res/tsc/fr/Stage/River.tsb"
BIN TSC_Sand	"res/tsc/fr/Stage/Sand.tsb"
BIN TSC_SandE	"res/tsc/fr/Stage/SandE.tsb"
BIN TSC_Santa	"res/tsc/fr/Stage/Santa.tsb"
BIN TSC_Shelt	"res/tsc/fr/Stage/Shelt.tsb"
BIN TSC_Start	"res/tsc/fr/Stage/Start.tsb"
BIN TSC_Statue	"res/tsc/fr/Stage/Statue.tsb"
BIN TSC_Stream	"res/tsc/fr/Stage/Stream.tsb"
BIN TSC_Weed	"res/tsc/fr/Stage/Weed.tsb"
BIN TSC_WeedB	"res/tsc/fr/Stage/WeedB.tsb"
BIN TSC_WeedD	"res/tsc/fr/Stage/WeedD.tsb"
BIN TSC_WeedS	"res/tsc/fr/Stage/WeedS.tsb"
