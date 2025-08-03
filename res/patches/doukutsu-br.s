    .include "macros.i"

/* Start of localizable data */
    .section .text
    .align 2

        .ascii "LANGDAT\0"

    .globl LANGUAGE
LANGUAGE:
        .ascii "BR\0\0"

PTR BMP_ASCII,      0
PTR BMP_KANJI,      0
PTR STAGE_NAMES,    STR_StageNames
PTR MUSIC_NAMES,    STR_MusicNames
PTR CREDITS_STR,    STR_Credits
PTR MENU_STR,       STR_Menus

PTR TS_TITLE,       UFTC_Title
PTR TS_MENUTEXT,    UFTC_MenuText
PTR TS_MSGTEXT,     UFTC_MsgText

PTR SPR_AIR,        SPR_Air
PTR SPR_PROMPT,     SPR_Prompt
PTR SPR_LEVELUP,    SPR_LevelUp
PTR SPR_LEVELDOWN,  SPR_LevelDown
PTR SPR_EMPTY,      SPR_Empty
PTR SPR_AHCHOO,     SPR_Ahchoo
PTR SPR_HEY,        SPR_Hey

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

PTRTAB_END:
        dc.l 0xFFFFFFFF /* Sentinel value for patchrom to stop relocating */

# Strings
BIN STR_StageNames      "res/strings/br_stagename.dat"
BIN STR_MusicNames      "res/strings/en_music.dat"
BIN STR_Credits         "res/strings/en_credits.dat"
BIN STR_Menus           "res/strings/en_menus.dat"

# Scripts (TSC)
# Global
BIN TSC_ArmsItem	"res/tsc/br/ArmsItem.tsb"
BIN TSC_Head		"res/tsc/br/Head.tsb"
BIN TSC_StageSelect	"res/tsc/br/StageSelect.tsb"
BIN TSC_Credits		"res/tsc/en/Stage/0.tsb"
# Stage Specific
BIN TSC_Almond	"res/tsc/br/Stage/Almond.tsb"
BIN TSC_Ballo1	"res/tsc/br/Stage/Ballo1.tsb"
BIN TSC_Ballo2	"res/tsc/br/Stage/Ballo2.tsb"
BIN TSC_Barr	"res/tsc/br/Stage/Barr.tsb"
BIN TSC_Blcny1	"res/tsc/br/Stage/Blcny1.tsb"
BIN TSC_Blcny2	"res/tsc/br/Stage/Blcny2.tsb"
BIN TSC_Cave	"res/tsc/br/Stage/Cave.tsb"
BIN TSC_Cemet	"res/tsc/br/Stage/Cemet.tsb"
BIN TSC_Cent	"res/tsc/br/Stage/Cent.tsb"
BIN TSC_CentW	"res/tsc/br/Stage/CentW.tsb"
BIN TSC_Chako	"res/tsc/br/Stage/Chako.tsb"
BIN TSC_Clock	"res/tsc/br/Stage/Clock.tsb"
BIN TSC_Comu	"res/tsc/br/Stage/Comu.tsb"
BIN TSC_Cthu	"res/tsc/br/Stage/Cthu.tsb"
BIN TSC_Cthu2	"res/tsc/br/Stage/Cthu2.tsb"
BIN TSC_Curly	"res/tsc/br/Stage/Curly.tsb"
BIN TSC_CurlyS	"res/tsc/br/Stage/CurlyS.tsb"
BIN TSC_Dark	"res/tsc/br/Stage/Dark.tsb"
BIN TSC_Drain	"res/tsc/br/Stage/Drain.tsb"
BIN TSC_e_Blcn	"res/tsc/br/Stage/e_Blcn.tsb"
BIN TSC_e_Ceme	"res/tsc/br/Stage/e_Ceme.tsb"
BIN TSC_e_Jenk	"res/tsc/br/Stage/e_Jenk.tsb"
BIN TSC_e_Labo	"res/tsc/br/Stage/e_Labo.tsb"
BIN TSC_e_Malc	"res/tsc/br/Stage/e_Malc.tsb"
BIN TSC_e_Maze	"res/tsc/br/Stage/e_Maze.tsb"
BIN TSC_e_Sky	"res/tsc/br/Stage/e_Sky.tsb"
BIN TSC_EgEnd1	"res/tsc/br/Stage/EgEnd1.tsb"
BIN TSC_EgEnd2	"res/tsc/br/Stage/EgEnd2.tsb"
BIN TSC_Egg1	"res/tsc/br/Stage/Egg1.tsb"
BIN TSC_Egg6	"res/tsc/br/Stage/Egg6.tsb"
BIN TSC_EggR	"res/tsc/br/Stage/EggR.tsb"
BIN TSC_EggR2	"res/tsc/br/Stage/EggR2.tsb"
BIN TSC_Eggs	"res/tsc/br/Stage/Eggs.tsb"
BIN TSC_Eggs2	"res/tsc/br/Stage/Eggs2.tsb"
BIN TSC_EggX	"res/tsc/br/Stage/EggX.tsb"
BIN TSC_EggX2	"res/tsc/br/Stage/EggX2.tsb"
BIN TSC_Fall	"res/tsc/br/Stage/Fall.tsb"
BIN TSC_Frog	"res/tsc/br/Stage/Frog.tsb"
BIN TSC_Gard	"res/tsc/br/Stage/Gard.tsb"
BIN TSC_Hell1	"res/tsc/br/Stage/Hell1.tsb"
BIN TSC_Hell2	"res/tsc/br/Stage/Hell2.tsb"
BIN TSC_Hell3	"res/tsc/br/Stage/Hell3.tsb"
BIN TSC_Hell4	"res/tsc/br/Stage/Hell4.tsb"
BIN TSC_Hell42	"res/tsc/br/Stage/Hell42.tsb"
BIN TSC_Island	"res/tsc/br/Stage/Island.tsb"
BIN TSC_Itoh	"res/tsc/br/Stage/Itoh.tsb"
BIN TSC_Jail1	"res/tsc/br/Stage/Jail1.tsb"
BIN TSC_Jail2	"res/tsc/br/Stage/Jail2.tsb"
BIN TSC_Jenka1	"res/tsc/br/Stage/Jenka1.tsb"
BIN TSC_Jenka2	"res/tsc/br/Stage/Jenka2.tsb"
BIN TSC_Kings	"res/tsc/br/Stage/Kings.tsb"
BIN TSC_Little	"res/tsc/br/Stage/Little.tsb"
BIN TSC_Lounge	"res/tsc/br/Stage/Lounge.tsb"
BIN TSC_Malco	"res/tsc/br/Stage/Malco.tsb"
BIN TSC_Mapi	"res/tsc/br/Stage/Mapi.tsb"
BIN TSC_MazeA	"res/tsc/br/Stage/MazeA.tsb"
BIN TSC_MazeB	"res/tsc/br/Stage/MazeB.tsb"
BIN TSC_MazeD	"res/tsc/br/Stage/MazeD.tsb"
BIN TSC_MazeH	"res/tsc/br/Stage/MazeH.tsb"
BIN TSC_MazeI	"res/tsc/br/Stage/MazeI.tsb"
BIN TSC_MazeM	"res/tsc/br/Stage/MazeM.tsb"
BIN TSC_MazeO	"res/tsc/br/Stage/MazeO.tsb"
BIN TSC_MazeS	"res/tsc/br/Stage/MazeS.tsb"
BIN TSC_MazeW	"res/tsc/br/Stage/MazeW.tsb"
BIN TSC_MiBox	"res/tsc/br/Stage/MiBox.tsb"
BIN TSC_Mimi	"res/tsc/br/Stage/Mimi.tsb"
BIN TSC_Momo	"res/tsc/br/Stage/Momo.tsb"
BIN TSC_Oside	"res/tsc/br/Stage/Oside.tsb"
BIN TSC_Ostep	"res/tsc/br/Stage/Ostep.tsb"
BIN TSC_Pens1	"res/tsc/br/Stage/Pens1.tsb"
BIN TSC_Pens2	"res/tsc/br/Stage/Pens2.tsb"
BIN TSC_Pixel	"res/tsc/br/Stage/Pixel.tsb"
BIN TSC_Plant	"res/tsc/br/Stage/Plant.tsb"
BIN TSC_Pole	"res/tsc/br/Stage/Pole.tsb"
BIN TSC_Pool	"res/tsc/br/Stage/Pool.tsb"
BIN TSC_Prefa1	"res/tsc/br/Stage/Prefa1.tsb"
BIN TSC_Prefa2	"res/tsc/br/Stage/Prefa2.tsb"
BIN TSC_Priso1	"res/tsc/br/Stage/Priso1.tsb"
BIN TSC_Priso2	"res/tsc/br/Stage/Priso2.tsb"
BIN TSC_Ring1	"res/tsc/br/Stage/Ring1.tsb"
BIN TSC_Ring2	"res/tsc/br/Stage/Ring2.tsb"
BIN TSC_Ring3	"res/tsc/br/Stage/Ring3.tsb"
BIN TSC_River	"res/tsc/br/Stage/River.tsb"
BIN TSC_Sand	"res/tsc/br/Stage/Sand.tsb"
BIN TSC_SandE	"res/tsc/br/Stage/SandE.tsb"
BIN TSC_Santa	"res/tsc/br/Stage/Santa.tsb"
BIN TSC_Shelt	"res/tsc/br/Stage/Shelt.tsb"
BIN TSC_Start	"res/tsc/br/Stage/Start.tsb"
BIN TSC_Statue	"res/tsc/br/Stage/Statue.tsb"
BIN TSC_Stream	"res/tsc/br/Stage/Stream.tsb"
BIN TSC_Weed	"res/tsc/br/Stage/Weed.tsb"
BIN TSC_WeedB	"res/tsc/br/Stage/WeedB.tsb"
BIN TSC_WeedD	"res/tsc/br/Stage/WeedD.tsb"
BIN TSC_WeedS	"res/tsc/br/Stage/WeedS.tsb"

# Tilesets
BIN UFTC_Title		"res/tiles_loc/title_en.uftc"
BIN UFTC_MenuText	"res/tiles_loc/menutext_br.uftc"
BIN UFTC_MsgText    "res/tiles_loc/font_blue.uftc"

# Sprites
SPRITE SPR_Air			"res/sprite_loc/air_br.spr"			7 1 1
SPRITE SPR_Prompt		"res/sprite_loc/prompt_br.spr" 		8 3 1
SPRITE SPR_LevelUp		"res/sprite_loc/levelup_br.spr"		7 2 2
SPRITE SPR_LevelDown	"res/sprite_loc/leveldown_br.spr"	7 2 2
SPRITE SPR_Empty		"res/sprite_loc/empty_br.spr"		5 1 2
SPRITE SPR_Ahchoo		"res/sprite_loc/ahchoo_br.spr"		2 2 2
SPRITE SPR_Hey		    "res/sprite_loc/hey_br.spr"		    2 2 1
