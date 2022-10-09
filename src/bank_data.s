.macro BIN _name, _file, _align=2
    .globl \_name
    .align \_align
\_name:
    .incbin "\_file"
.endm

.macro PCM _name, _file, _align=256
    .globl \_name
    .globl \_name\()_end
    .align \_align
\_name:
    .incbin "\_file"
\_name\()_end:
.endm

.section .text

# Compressed tileset patterns
BIN UFTC_Almond "res/Stage/PrtAlmond_vert.uftc"
BIN UFTC_Barr   "res/Stage/PrtBarr_vert.uftc"
BIN UFTC_Cave   "res/Stage/PrtCave_vert.uftc"
BIN UFTC_Cent   "res/Stage/PrtCent_vert.uftc"
BIN UFTC_EggIn  "res/Stage/PrtEggIn_vert.uftc"
BIN UFTC_Eggs   "res/Stage/Eggs/PrtEggs_vert.uftc"
BIN UFTC_EggX   "res/Stage/PrtEggX1_vert.uftc"
BIN UFTC_Fall   "res/Stage/PrtFall_vert.uftc"
BIN UFTC_Gard   "res/Stage/PrtGard_vert.uftc"
BIN UFTC_Hell   "res/Stage/Hell/PrtHell_vert.uftc"
BIN UFTC_Jail   "res/Stage/PrtJail_vert.uftc"
BIN UFTC_Labo   "res/Stage/PrtLabo_vert.uftc"
BIN UFTC_Maze   "res/Stage/Maze/PrtMaze_vert.uftc"
BIN UFTC_Mimi   "res/Stage/Mimi/PrtMimi_vert.uftc"
BIN UFTC_Oside  "res/Stage/PrtOside_vert.uftc"
BIN UFTC_Pens   "res/Stage/PrtPens_vert.uftc"
BIN UFTC_River  "res/Stage/PrtRiver_vert.uftc"
BIN UFTC_Sand   "res/Stage/PrtSand_vert.uftc"
BIN UFTC_Store  "res/Stage/PrtStore_vert.uftc"
BIN UFTC_Weed   "res/Stage/PrtWeed_vert.uftc"
BIN UFTC_Blcny  "res/Stage/White/PrtBlcny_vert.uftc"

BIN UFTC_EggX2  "res/Stage/PrtEggX2_vert.uftc"
BIN UFTC_Eggs2  "res/Stage/Eggs/PrtEggs2_vert.uftc"
BIN UFTC_MazeM  "res/Stage/Maze/PrtMazeM_vert.uftc"
BIN UFTC_Kings  "res/Stage/White/PrtKings_vert.uftc"
BIN UFTC_Statue "res/Stage/Hell/PrtStatue_vert.uftc"
BIN UFTC_Ring2  "res/Stage/White/PrtRing2_vert.uftc"
BIN UFTC_Ring3  "res/Stage/White/PrtRing3_vert.uftc"

# Tile Attributes
BIN PXA_Almond	"res/Stage/Almond.pxa"
BIN PXA_Barr	"res/Stage/Barr.pxa"
BIN PXA_Cave	"res/Stage/Cave.pxa"
BIN PXA_Cent	"res/Stage/Cent.pxa"
BIN PXA_EggIn	"res/Stage/EggIn.pxa"
BIN PXA_EggX	"res/Stage/EggX.pxa"
BIN PXA_Fall	"res/Stage/Fall.pxa"
BIN PXA_Gard	"res/Stage/Gard.pxa"
BIN PXA_Jail	"res/Stage/Jail.pxa"
BIN PXA_Labo	"res/Stage/Labo.pxa"
BIN PXA_Oside	"res/Stage/Oside.pxa"
BIN PXA_Pens	"res/Stage/Pens.pxa"
BIN PXA_River	"res/Stage/River.pxa"
BIN PXA_Sand	"res/Stage/Sand.pxa"
BIN PXA_Store	"res/Stage/Store.pxa"
BIN PXA_Weed	"res/Stage/Weed.pxa"

# Optimized Tileset Stages
BIN PXA_Mimi		"res/Stage/Mimi/Mimi.pxa"
BIN PXM_Barr		"res/Stage/Mimi/Barr.cpxm"
BIN PXM_Cemet		"res/Stage/Mimi/Cemet.cpxm"
BIN PXM_e_Ceme		"res/Stage/Mimi/e_Ceme.cpxm"
BIN PXM_MiBox		"res/Stage/Mimi/MiBox.cpxm"
BIN PXM_Mimi		"res/Stage/Mimi/Mimi.cpxm"
BIN PXM_Plant		"res/Stage/Mimi/Plant.cpxm"
BIN PXM_Pool		"res/Stage/Mimi/Pool.cpxm"

BIN PXA_Eggs		"res/Stage/Eggs/Eggs.pxa"
BIN PXM_Eggs		"res/Stage/Eggs/Eggs.cpxm"

BIN PXA_Eggs2		"res/Stage/Eggs/Eggs2.pxa"
BIN PXM_Eggs2		"res/Stage/Eggs/Eggs2.cpxm"

BIN PXA_Maze		"res/Stage/Maze/Maze.pxa"
BIN PXM_e_Maze		"res/Stage/Maze/e_Maze.cpxm"
BIN PXM_MazeB		"res/Stage/Maze/MazeB.cpxm"
BIN PXM_MazeD		"res/Stage/Maze/MazeD.cpxm"
BIN PXM_MazeH		"res/Stage/Maze/MazeH.cpxm"
BIN PXM_MazeI		"res/Stage/Maze/MazeI.cpxm"
BIN PXM_MazeO		"res/Stage/Maze/MazeO.cpxm"
BIN PXM_MazeS		"res/Stage/Maze/MazeS.cpxm"
BIN PXM_MazeW		"res/Stage/Maze/MazeW.cpxm"

BIN PXA_MazeM		"res/Stage/Maze/MazeM.pxa"
BIN PXM_MazeM		"res/Stage/Maze/MazeM.cpxm"

BIN PXA_Blcny		"res/Stage/White/Blcny.pxa"
BIN PXM_e_Blcn		"res/Stage/White/e_Blcn.cpxm"
BIN PXM_Blcny1		"res/Stage/White/Blcny1.cpxm"
BIN PXM_Blcny2		"res/Stage/White/Blcny2.cpxm"

BIN PXA_Kings		"res/Stage/White/Kings.pxa"
BIN PXM_Kings		"res/Stage/White/Kings.cpxm"
BIN PXM_Ring1		"res/Stage/White/Ring1.cpxm"
BIN PXM_Ostep		"res/Stage/White/Ostep.cpxm"

BIN PXA_Ring2		"res/Stage/White/Ring2.pxa"
BIN PXM_Ring2		"res/Stage/White/Ring2.cpxm"

BIN PXA_Ring3		"res/Stage/White/Ring3.pxa"
BIN PXM_Ring3		"res/Stage/White/Ring3.cpxm"

BIN PXA_Hell		"res/Stage/Hell/Hell.pxa"
BIN PXM_Hell1		"res/Stage/Hell/Hell1.cpxm"
BIN PXM_Hell2		"res/Stage/Hell/Hell2.cpxm"
BIN PXM_Hell3		"res/Stage/Hell/Hell3.cpxm"
BIN PXM_Hell4		"res/Stage/Hell/Hell4.cpxm"
BIN PXM_Hell42		"res/Stage/Hell/Hell42.cpxm"
BIN PXM_Ballo1		"res/Stage/Hell/Ballo1.cpxm"
BIN PXM_Ballo2		"res/Stage/Hell/Ballo2.cpxm"

BIN PXA_Statue		"res/Stage/Hell/Statue.pxa"
BIN PXM_Statue		"res/Stage/Hell/Statue.cpxm"

# Stages (PXM)
BIN PXM_Almond	"res/Stage/Almond.cpxm"
BIN PXM_Cave	"res/Stage/Cave.cpxm"
BIN PXM_Cent	"res/Stage/Cent.cpxm"
BIN PXM_CentW	"res/Stage/CentW.cpxm"
BIN PXM_Chako	"res/Stage/Chako.cpxm"
BIN PXM_Clock	"res/Stage/Clock.cpxm"
BIN PXM_Comu	"res/Stage/Comu.cpxm"
BIN PXM_Cthu	"res/Stage/Cthu.cpxm"
BIN PXM_Cthu2	"res/Stage/Cthu2.cpxm"
BIN PXM_Curly	"res/Stage/Curly.cpxm"
BIN PXM_CurlyS	"res/Stage/CurlyS.cpxm"
BIN PXM_Dark	"res/Stage/Dark.cpxm"
BIN PXM_Drain	"res/Stage/Drain.cpxm"
BIN PXM_e_Jenk	"res/Stage/e_Jenk.cpxm"
BIN PXM_e_Labo	"res/Stage/e_Labo.cpxm"
BIN PXM_e_Malc	"res/Stage/e_Malc.cpxm"
BIN PXM_e_Sky	"res/Stage/e_Sky.cpxm"
BIN PXM_EgEnd1	"res/Stage/EgEnd1.cpxm"
BIN PXM_EgEnd2	"res/Stage/EgEnd2.cpxm"
BIN PXM_Egg1	"res/Stage/Egg1.cpxm"
BIN PXM_Egg6	"res/Stage/Egg6.cpxm"
BIN PXM_EggR	"res/Stage/EggR.cpxm"
BIN PXM_EggR2	"res/Stage/EggR2.cpxm"
BIN PXM_EggX	"res/Stage/EggX.cpxm"
BIN PXM_EggX2	"res/Stage/EggX2.cpxm"
BIN PXM_Fall	"res/Stage/Fall.cpxm"
BIN PXM_Frog	"res/Stage/Frog.cpxm"
BIN PXM_Gard	"res/Stage/Gard.cpxm"
BIN PXM_Itoh	"res/Stage/Itoh.cpxm"
BIN PXM_Island	"res/Stage/Island.cpxm"
BIN PXM_Jail1	"res/Stage/Jail1.cpxm"
BIN PXM_Jail2	"res/Stage/Jail2.cpxm"
BIN PXM_Jenka1	"res/Stage/Jenka1.cpxm"
BIN PXM_Jenka2	"res/Stage/Jenka2.cpxm"
BIN PXM_Little	"res/Stage/Little.cpxm"
BIN PXM_Lounge	"res/Stage/Lounge.cpxm"
BIN PXM_Malco	"res/Stage/Malco.cpxm"
BIN PXM_Mapi	"res/Stage/Mapi.cpxm"
BIN PXM_MazeA	"res/Stage/MazeA.cpxm"
BIN PXM_Momo	"res/Stage/Momo.cpxm"
BIN PXM_Oside	"res/Stage/Oside.cpxm"
BIN PXM_Pens1	"res/Stage/Pens1.cpxm"
BIN PXM_Pens2	"res/Stage/Pens2.cpxm"
BIN PXM_Pixel	"res/Stage/Pixel.cpxm"
BIN PXM_Pole	"res/Stage/Pole.cpxm"
BIN PXM_Prefa1	"res/Stage/Prefa1.cpxm"
BIN PXM_Prefa2	"res/Stage/Prefa2.cpxm"
BIN PXM_Priso1	"res/Stage/Priso1.cpxm"
BIN PXM_Priso2	"res/Stage/Priso2.cpxm"
BIN PXM_River	"res/Stage/River.cpxm"
BIN PXM_Sand	"res/Stage/Sand.cpxm"
BIN PXM_SandE	"res/Stage/SandE.cpxm"
BIN PXM_Santa	"res/Stage/Santa.cpxm"
BIN PXM_Shelt	"res/Stage/Shelt.cpxm"
BIN PXM_Start	"res/Stage/Start.cpxm"
BIN PXM_Stream	"res/Stage/Stream.cpxm"
BIN PXM_Weed	"res/Stage/Weed.cpxm"
BIN PXM_WeedB	"res/Stage/WeedB.cpxm"
BIN PXM_WeedD	"res/Stage/WeedD.cpxm"
BIN PXM_WeedS	"res/Stage/WeedS.cpxm"

# Entities (PXE)
BIN PXE_0       "res/Stage/0.pxe"
BIN PXE_Almond	"res/Stage/Almond.pxe"
BIN PXE_Ballo1	"res/Stage/Ballo1.pxe"
BIN PXE_Ballo2	"res/Stage/Ballo2.pxe"
BIN PXE_Barr	"res/Stage/Barr.pxe"
BIN PXE_Blcny1	"res/Stage/Blcny1.pxe"
BIN PXE_Blcny2	"res/Stage/Blcny2.pxe"
BIN PXE_Cave	"res/Stage/Cave.pxe"
BIN PXE_Cemet	"res/Stage/Cemet.pxe"
BIN PXE_Cent	"res/Stage/Cent.pxe"
BIN PXE_CentW	"res/Stage/CentW.pxe"
BIN PXE_Chako	"res/Stage/Chako.pxe"
BIN PXE_Clock	"res/Stage/Clock.pxe"
BIN PXE_Comu	"res/Stage/Comu.pxe"
BIN PXE_Cthu	"res/Stage/Cthu.pxe"
BIN PXE_Cthu2	"res/Stage/Cthu2.pxe"
BIN PXE_Curly	"res/Stage/Curly.pxe"
BIN PXE_CurlyS	"res/Stage/CurlyS.pxe"
BIN PXE_Dark	"res/Stage/Dark.pxe"
BIN PXE_Drain	"res/Stage/Drain.pxe"
BIN PXE_e_Blcn	"res/Stage/e_Blcn.pxe"
BIN PXE_e_Ceme	"res/Stage/e_Ceme.pxe"
BIN PXE_e_Jenk	"res/Stage/e_Jenk.pxe"
BIN PXE_e_Labo	"res/Stage/e_Labo.pxe"
BIN PXE_e_Malc	"res/Stage/e_Malc.pxe"
BIN PXE_e_Maze	"res/Stage/e_Maze.pxe"
BIN PXE_e_Sky	"res/Stage/e_Sky.pxe"
BIN PXE_EgEnd1	"res/Stage/EgEnd1.pxe"
BIN PXE_EgEnd2	"res/Stage/EgEnd2.pxe"
BIN PXE_Egg1	"res/Stage/Egg1.pxe"
BIN PXE_Egg6	"res/Stage/Egg6.pxe"
BIN PXE_EggR	"res/Stage/EggR.pxe"
BIN PXE_EggR2	"res/Stage/EggR2.pxe"
BIN PXE_Eggs	"res/Stage/Eggs.pxe"
BIN PXE_Eggs2	"res/Stage/Eggs2.pxe"
BIN PXE_EggX	"res/Stage/EggX.pxe"
BIN PXE_EggX2	"res/Stage/EggX2.pxe"
BIN PXE_Fall	"res/Stage/Fall.pxe"
BIN PXE_Frog	"res/Stage/Frog.pxe"
BIN PXE_Gard	"res/Stage/Gard.pxe"
BIN PXE_Hell1	"res/Stage/Hell1.pxe"
BIN PXE_Hell2	"res/Stage/Hell2.pxe"
BIN PXE_Hell3	"res/Stage/Hell3.pxe"
BIN PXE_Hell4	"res/Stage/Hell4.pxe"
BIN PXE_Hell42	"res/Stage/Hell42.pxe"
BIN PXE_Island	"res/Stage/Island.pxe"
BIN PXE_Itoh	"res/Stage/Itoh.pxe"
BIN PXE_Jail1	"res/Stage/Jail1.pxe"
BIN PXE_Jail2	"res/Stage/Jail2.pxe"
BIN PXE_Jenka1	"res/Stage/Jenka1.pxe"
BIN PXE_Jenka2	"res/Stage/Jenka2.pxe"
BIN PXE_Kings	"res/Stage/Kings.pxe"
BIN PXE_Little	"res/Stage/Little.pxe"
BIN PXE_Lounge	"res/Stage/Lounge.pxe"
BIN PXE_Malco	"res/Stage/Malco.pxe"
BIN PXE_Mapi	"res/Stage/Mapi.pxe"
BIN PXE_MazeA	"res/Stage/MazeA.pxe"
BIN PXE_MazeB	"res/Stage/MazeB.pxe"
BIN PXE_MazeD	"res/Stage/MazeD.pxe"
BIN PXE_MazeH	"res/Stage/MazeH.pxe"
BIN PXE_MazeI	"res/Stage/MazeI.pxe"
BIN PXE_MazeM	"res/Stage/MazeM.pxe"
BIN PXE_MazeO	"res/Stage/MazeO.pxe"
BIN PXE_MazeS	"res/Stage/MazeS.pxe"
BIN PXE_MazeW	"res/Stage/MazeW.pxe"
BIN PXE_MiBox	"res/Stage/MiBox.pxe"
BIN PXE_Mimi	"res/Stage/Mimi.pxe"
BIN PXE_Momo	"res/Stage/Momo.pxe"
BIN PXE_Oside	"res/Stage/Oside.pxe"
BIN PXE_Ostep	"res/Stage/Ostep.pxe"
BIN PXE_Pens1	"res/Stage/Pens1.pxe"
BIN PXE_Pens2	"res/Stage/Pens2.pxe"
BIN PXE_Pixel	"res/Stage/Pixel.pxe"
BIN PXE_Plant	"res/Stage/Plant.pxe"
BIN PXE_Pole	"res/Stage/Pole.pxe"
BIN PXE_Pool	"res/Stage/Pool.pxe"
BIN PXE_Prefa1	"res/Stage/Prefa1.pxe"
BIN PXE_Prefa2	"res/Stage/Prefa2.pxe"
BIN PXE_Priso1	"res/Stage/Priso1.pxe"
BIN PXE_Priso2	"res/Stage/Priso2.pxe"
BIN PXE_Ring1	"res/Stage/Ring1.pxe"
BIN PXE_Ring2	"res/Stage/Ring2.pxe"
BIN PXE_Ring3	"res/Stage/Ring3.pxe"
BIN PXE_River	"res/Stage/River.pxe"
BIN PXE_Sand	"res/Stage/Sand.pxe"
BIN PXE_SandE	"res/Stage/SandE.pxe"
BIN PXE_Santa	"res/Stage/Santa.pxe"
BIN PXE_Shelt	"res/Stage/Shelt.pxe"
BIN PXE_Start	"res/Stage/Start.pxe"
BIN PXE_Statue	"res/Stage/Statue.pxe"
BIN PXE_Stream	"res/Stage/Stream.pxe"
BIN PXE_Weed	"res/Stage/Weed.pxe"
BIN PXE_WeedB	"res/Stage/WeedB.pxe"
BIN PXE_WeedD	"res/Stage/WeedD.pxe"
BIN PXE_WeedS	"res/Stage/WeedS.pxe"

# NPC Table
BIN     NPC_TABLE		"res/npc.tbl"

/* Background tilemaps */
# Moon
BIN		PAT_MoonTop		"res/back/bkMoonTop.pat"
BIN		MAP_MoonTop		"res/back/bkMoonTop.map"
BIN		PAT_MoonBtm		"res/back/bkMoonBottom.pat"
BIN		MAP_MoonBtm		"res/back/bkMoonBottom.map"
# Fog
BIN		PAT_FogTop		"res/back/bkFogTop.pat"
BIN		MAP_FogTop		"res/back/bkFogTop.map"
BIN		PAT_FogBtm		"res/back/bkFogBottom.pat"
BIN		MAP_FogBtm		"res/back/bkFogBottom.map"
# Sound Test
BIN		PAT_SndTest	    "res/back/soundtest.pat"
BIN		MAP_SndTest	    "res/back/soundtest.map"

/* XGM Driver blob */
BIN z80_xgm, "src/xgm/z80_xgm.o80"
    .globl z80_xgm_end
z80_xgm_end:

BIN stop_xgm, "res/stop_xgm.bin"

/* Sound Effects */
PCM SFX_01  "res/sfx/01.pcm"
PCM SFX_02  "res/sfx/02.pcm"
PCM SFX_03  "res/sfx/03.pcm"
PCM SFX_04  "res/sfx/04.pcm"
PCM SFX_05  "res/sfx/05.pcm"
PCM SFX_06  "res/sfx/06.pcm"
/* 07 */
PCM SFX_08  "res/sfx/08.pcm"
/* 09 0A */
PCM SFX_0B  "res/sfx/0B.pcm"
PCM SFX_0C  "res/sfx/0C.pcm"
/* 0D */
PCM SFX_0E  "res/sfx/0E.pcm"
PCM SFX_0F  "res/sfx/0F.pcm"
PCM SFX_10  "res/sfx/10.pcm"
PCM SFX_11  "res/sfx/11.pcm"
PCM SFX_12  "res/sfx/12.pcm"
/* 13 */
PCM SFX_14  "res/sfx/14.pcm"
PCM SFX_15  "res/sfx/15.pcm"
PCM SFX_16  "res/sfx/16.pcm"
PCM SFX_17  "res/sfx/17.pcm"
PCM SFX_18  "res/sfx/18.pcm"
PCM SFX_19  "res/sfx/19.pcm"
PCM SFX_1A  "res/sfx/1A.pcm"
PCM SFX_1B  "res/sfx/1B.pcm"
PCM SFX_1C  "res/sfx/1C.pcm"
PCM SFX_1D  "res/sfx/1D.pcm"
PCM SFX_1E  "res/sfx/1E.pcm"
PCM SFX_1F  "res/sfx/1F.pcm"
PCM SFX_20  "res/sfx/20.pcm"
PCM SFX_21  "res/sfx/21.pcm"
PCM SFX_22  "res/sfx/22.pcm"

.section .rodata
/* 0x200000 */
    .align 0x80000

    .globl smp_null
    .globl TILE_BLANK
    .globl BLANK_DATA
smp_null:
TILE_BLANK:
BLANK_DATA:
.rept 16
    dc.l    0, 0, 0, 0
.endr

    .ascii  "SEGA MEGA DRIVE "
    .ascii  "                "
    .ascii  "https://www.youtube.com/watch?v=y-kIvItmQMc     "
    .ascii  "https://www.youtube.com/watch?v=HnyGSl3K-IE     "
    .ascii  "GM 00001009-00"
    dc.w    0

/* More sound effects */

PCM SFX_23  "res/sfx/23.pcm"
/* 24 */
PCM SFX_25  "res/sfx/25.pcm"
PCM SFX_26  "res/sfx/26.pcm"
PCM SFX_27  "res/sfx/27.pcm"
PCM SFX_28  "res/sfx/28.pcm"
/* 29 */
PCM SFX_2A  "res/sfx/2A.pcm"
PCM SFX_2B  "res/sfx/2B.pcm"
PCM SFX_2C  "res/sfx/2C.pcm"
PCM SFX_2D  "res/sfx/2D.pcm"
PCM SFX_2E  "res/sfx/2E.pcm"
PCM SFX_2F  "res/sfx/2F.pcm"
PCM SFX_30  "res/sfx/30.pcm"
PCM SFX_31  "res/sfx/31.pcm"
PCM SFX_32  "res/sfx/32.pcm"
PCM SFX_33  "res/sfx/33.pcm"
PCM SFX_34  "res/sfx/34.pcm"
PCM SFX_35  "res/sfx/35.pcm"
PCM SFX_36  "res/sfx/36.pcm"
PCM SFX_37  "res/sfx/37.pcm"
PCM SFX_38  "res/sfx/38.pcm"
PCM SFX_39  "res/sfx/39.pcm"
PCM SFX_3A  "res/sfx/3A.pcm"
PCM SFX_3B  "res/sfx/3B.pcm"
PCM SFX_3C  "res/sfx/3C.pcm"
PCM SFX_3D  "res/sfx/3D.pcm"
PCM SFX_3E  "res/sfx/3E.pcm"
PCM SFX_3F  "res/sfx/3F.pcm"
PCM SFX_40  "res/sfx/40.pcm"
PCM SFX_41  "res/sfx/41.pcm"
/* 42 43 44 45 */
PCM SFX_46  "res/sfx/46.pcm"
PCM SFX_47  "res/sfx/47.pcm"
PCM SFX_48  "res/sfx/48.pcm"
/* 49 4A 4B 4C 4D 4E 4F */

#PCM SFX_96  "res/sfx/96.pcm"
#PCM SFX_97  "res/sfx/97.pcm"
#PCM SFX_98  "res/sfx/98.pcm"
#PCM SFX_99  "res/sfx/99.pcm"
#PCM SFX_9A  "res/sfx/9A.pcm"
#PCM SFX_9B  "res/sfx/9B.pcm"

/* 60 61 62 63 */
PCM SFX_64  "res/sfx/64.pcm"
PCM SFX_65  "res/sfx/65.pcm"
PCM SFX_66  "res/sfx/66.pcm"
PCM SFX_67  "res/sfx/67.pcm"
PCM SFX_68  "res/sfx/68.pcm"
PCM SFX_69  "res/sfx/69.pcm"
PCM SFX_6A  "res/sfx/6A.pcm"
PCM SFX_6B  "res/sfx/6B.pcm"
PCM SFX_6C  "res/sfx/6C.pcm"
PCM SFX_6D  "res/sfx/6D.pcm"
PCM SFX_6E  "res/sfx/6E.pcm"
PCM SFX_6F  "res/sfx/6F.pcm"
PCM SFX_70  "res/sfx/70.pcm"
PCM SFX_71  "res/sfx/71.pcm"
PCM SFX_72  "res/sfx/72.pcm"
PCM SFX_73  "res/sfx/73.pcm"
PCM SFX_74  "res/sfx/74.pcm"
PCM SFX_75  "res/sfx/75.pcm"
/* 76 77 78 79 7A 7B 7C 7D 7E 7F */

BIN BGM_Mischievous, "res/bgm/mischievous.xgc"
BIN BGM_Safety,      "res/bgm/safety.xgc"
BIN BGM_GameOver,    "res/bgm/gameover.xgc"
BIN BGM_Gravity,     "res/bgm/gravity.xgc"
BIN BGM_Grasstown,   "res/bgm/grasstown.xgc"
BIN BGM_Meltdown2,   "res/bgm/meltdown2.xgc"
BIN BGM_EyesOfFlame, "res/bgm/eyesofflame.xgc"
BIN BGM_Gestation,   "res/bgm/gestation.xgc"
BIN BGM_Mimiga,      "res/bgm/mimiga.xgc"
BIN BGM_Fanfare1,    "res/bgm/fanfare1.xgc"
BIN BGM_Balrog,      "res/bgm/balrog.xgc"
BIN BGM_Cemetery,    "res/bgm/cemetery.xgc"
BIN BGM_Plant,       "res/bgm/plant.xgc"
BIN BGM_Pulse,       "res/bgm/pulse.xgc"
BIN BGM_Fanfare2,    "res/bgm/fanfare2.xgc"
BIN BGM_Fanfare3,    "res/bgm/fanfare3.xgc"
BIN BGM_Tyrant,      "res/bgm/tyrant.xgc"
BIN BGM_Run,         "res/bgm/run.xgc"
BIN BGM_Jenka1,      "res/bgm/jenka1.xgc"
BIN BGM_LabFight,    "res/bgm/labfight.xgc"
BIN BGM_Access,      "res/bgm/access.xgc"
BIN BGM_Oppression,  "res/bgm/oppression.xgc"
BIN BGM_Geothermal,  "res/bgm/geothermal.xgc"
BIN BGM_CaveStory,   "res/bgm/cavestory.xgc"
BIN BGM_MoonSong,    "res/bgm/moonsong.xgc"
BIN BGM_HerosEnd,    "res/bgm/herosend.xgc"
BIN BGM_Scorching,   "res/bgm/scorching.xgc"
BIN BGM_Quiet,       "res/bgm/quiet.xgc"
BIN BGM_LastCave,    "res/bgm/lastcave.xgc"
BIN BGM_Balcony,     "res/bgm/balcony.xgc"
BIN BGM_Charge,      "res/bgm/charge.xgc"
BIN BGM_LastBoss,    "res/bgm/lastboss.xgc"
BIN BGM_WayBackHome, "res/bgm/credits.xgc"
BIN BGM_Zombie,      "res/bgm/zombie.xgc"
BIN BGM_Breakdown,   "res/bgm/breakdown.xgc"
BIN BGM_RunningHell, "res/bgm/running_hell.xgc"
BIN BGM_Jenka2,      "res/bgm/jenka2.xgc"
BIN BGM_Waterway,    "res/bgm/waterway.xgc"
BIN BGM_SealChamber, "res/bgm/seal_chamber.xgc"
BIN BGM_Toroko,      "res/bgm/toroko.xgc"
BIN BGM_White,       "res/bgm/white.xgc"

/* Bonus Tracks */
BIN BGM_TimeTable,   "res/bgm/timetable.xgc"

/* 0x380000 */
#    .align 0x80000

# Japanese Font - 1bpp bitmap data
#BIN     BMP_Ascii		"res/ja_ascii.dat"
#BIN     BMP_Kanji		"res/ja_kanji.dat"

# Japanese stage names and credits text
#BIN     JStageName		"res/ja_stagename.dat"
#BIN     JCreditStr		"res/ja_credits.dat"
#BIN     JConfigText		"res/ja_config.dat"

/* Credits Illustrations */
BIN		PAT_Ill01	"res/credits/ill01.pat"
BIN		MAP_Ill01	"res/credits/ill01.map"
BIN		PAT_Ill02	"res/credits/ill02.pat"
BIN		MAP_Ill02	"res/credits/ill02.map"
BIN		PAT_Ill03	"res/credits/ill03.pat"
BIN		MAP_Ill03	"res/credits/ill03.map"
BIN		PAT_Ill04	"res/credits/ill04.pat"
BIN		MAP_Ill04	"res/credits/ill04.map"
BIN		PAT_Ill05	"res/credits/ill05.pat"
BIN		MAP_Ill05	"res/credits/ill05.map"
BIN		PAT_Ill06	"res/credits/ill06.pat"
BIN		MAP_Ill06	"res/credits/ill06.map"
BIN		PAT_Ill07	"res/credits/ill07.pat"
BIN		MAP_Ill07	"res/credits/ill07.map"
BIN		PAT_Ill08	"res/credits/ill08.pat"
BIN		MAP_Ill08	"res/credits/ill08.map"
BIN		PAT_Ill09	"res/credits/ill09.pat"
BIN		MAP_Ill09	"res/credits/ill09.map"
BIN		PAT_Ill10	"res/credits/ill10.pat"
BIN		MAP_Ill10	"res/credits/ill10.map"
BIN		PAT_Ill11	"res/credits/ill11.pat"
BIN		MAP_Ill11	"res/credits/ill11.map"
BIN		PAT_Ill12	"res/credits/ill12.pat"
BIN		MAP_Ill12	"res/credits/ill12.map"

BIN		PAT_Ill14	"res/credits/ill14.pat"
BIN		MAP_Ill14	"res/credits/ill14.map"
BIN		PAT_Ill15	"res/credits/ill15.pat"
BIN		MAP_Ill15	"res/credits/ill15.map"
BIN		PAT_Ill16	"res/credits/ill16.pat"
BIN		MAP_Ill16	"res/credits/ill16.map"
BIN		PAT_Ill17	"res/credits/ill17.pat"
BIN		MAP_Ill17	"res/credits/ill17.map"
BIN		PAT_Ill18	"res/credits/ill18.pat"
BIN		MAP_Ill18	"res/credits/ill18.map"

/* Level Select data */
BIN LS_00		"res/save/00_firstcave.sram-trim"
BIN LS_01		"res/save/01_mimigavillage.sram-trim"
BIN LS_02		"res/save/02_eggcorridor.sram-trim"
BIN LS_03		"res/save/03_grasstown.sram-trim"
BIN LS_04		"res/save/04_malco.sram-trim"
BIN LS_05		"res/save/05_balfrog.sram-trim"
BIN LS_06		"res/save/06_sandzone.sram-trim"
BIN LS_07		"res/save/07_omega.sram-trim"
BIN LS_08		"res/save/08_storehouse.sram-trim"
BIN LS_09		"res/save/09_labyrinth.sram-trim"
BIN LS_10		"res/save/10_monsterx.sram-trim"
BIN LS_11		"res/save/11_labyrinthm.sram-trim"
BIN LS_12		"res/save/12_core.sram-trim"
BIN LS_13		"res/save/13_waterway.sram-trim"
BIN LS_14		"res/save/14_eggcorridor2.sram-trim"
BIN LS_15		"res/save/15_outerwall.sram-trim"
BIN LS_16		"res/save/16_plantation.sram-trim"
BIN LS_17		"res/save/17_lastcave.sram-trim"
BIN LS_18		"res/save/18_lastcave2.sram-trim"
BIN LS_19		"res/save/19_balcony.sram-trim"
BIN LS_20		"res/save/20_sacredground.sram"
BIN LS_21		"res/save/21_sealchamber.sram"


/* Start of localizable data */
.ascii "LANGDAT\0"

    .globl LANGUAGE
LANGUAGE:
.ascii "EN\0\0"

    .globl BMP_ASCII
BMP_ASCII:
dc.l 0
    .globl BMP_KANJI
BMP_KANJI:
dc.l 0

    .globl STAGE_NAMES
STAGE_NAMES:
dc.l 0
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

# Scripts (TSC) - English
# Global
BIN TSC_ArmsItem	"res/tsc/en/ArmsItem.tsb"
BIN TSC_Head		"res/tsc/en/Head.tsb"
BIN TSC_StageSelect	"res/tsc/en/StageSelect.tsb"
BIN TSC_Credits		"res/tsc/en/Stage/0.tsb"
# Stage Specific
BIN TSC_Almond	"res/tsc/en/Stage/Almond.tsb"
BIN TSC_Ballo1	"res/tsc/en/Stage/Ballo1.tsb"
BIN TSC_Ballo2	"res/tsc/en/Stage/Ballo2.tsb"
BIN TSC_Barr	"res/tsc/en/Stage/Barr.tsb"
BIN TSC_Blcny1	"res/tsc/en/Stage/Blcny1.tsb"
BIN TSC_Blcny2	"res/tsc/en/Stage/Blcny2.tsb"
BIN TSC_Cave	"res/tsc/en/Stage/Cave.tsb"
BIN TSC_Cemet	"res/tsc/en/Stage/Cemet.tsb"
BIN TSC_Cent	"res/tsc/en/Stage/Cent.tsb"
BIN TSC_CentW	"res/tsc/en/Stage/CentW.tsb"
BIN TSC_Chako	"res/tsc/en/Stage/Chako.tsb"
BIN TSC_Clock	"res/tsc/en/Stage/Clock.tsb"
BIN TSC_Comu	"res/tsc/en/Stage/Comu.tsb"
BIN TSC_Cthu	"res/tsc/en/Stage/Cthu.tsb"
BIN TSC_Cthu2	"res/tsc/en/Stage/Cthu2.tsb"
BIN TSC_Curly	"res/tsc/en/Stage/Curly.tsb"
BIN TSC_CurlyS	"res/tsc/en/Stage/CurlyS.tsb"
BIN TSC_Dark	"res/tsc/en/Stage/Dark.tsb"
BIN TSC_Drain	"res/tsc/en/Stage/Drain.tsb"
BIN TSC_e_Blcn	"res/tsc/en/Stage/e_Blcn.tsb"
BIN TSC_e_Ceme	"res/tsc/en/Stage/e_Ceme.tsb"
BIN TSC_e_Jenk	"res/tsc/en/Stage/e_Jenk.tsb"
BIN TSC_e_Labo	"res/tsc/en/Stage/e_Labo.tsb"
BIN TSC_e_Malc	"res/tsc/en/Stage/e_Malc.tsb"
BIN TSC_e_Maze	"res/tsc/en/Stage/e_Maze.tsb"
BIN TSC_e_Sky	"res/tsc/en/Stage/e_Sky.tsb"
BIN TSC_EgEnd1	"res/tsc/en/Stage/EgEnd1.tsb"
BIN TSC_EgEnd2	"res/tsc/en/Stage/EgEnd2.tsb"
BIN TSC_Egg1	"res/tsc/en/Stage/Egg1.tsb"
BIN TSC_Egg6	"res/tsc/en/Stage/Egg6.tsb"
BIN TSC_EggR	"res/tsc/en/Stage/EggR.tsb"
BIN TSC_EggR2	"res/tsc/en/Stage/EggR2.tsb"
BIN TSC_Eggs	"res/tsc/en/Stage/Eggs.tsb"
BIN TSC_Eggs2	"res/tsc/en/Stage/Eggs2.tsb"
BIN TSC_EggX	"res/tsc/en/Stage/EggX.tsb"
BIN TSC_EggX2	"res/tsc/en/Stage/EggX2.tsb"
BIN TSC_Fall	"res/tsc/en/Stage/Fall.tsb"
BIN TSC_Frog	"res/tsc/en/Stage/Frog.tsb"
BIN TSC_Gard	"res/tsc/en/Stage/Gard.tsb"
BIN TSC_Hell1	"res/tsc/en/Stage/Hell1.tsb"
BIN TSC_Hell2	"res/tsc/en/Stage/Hell2.tsb"
BIN TSC_Hell3	"res/tsc/en/Stage/Hell3.tsb"
BIN TSC_Hell4	"res/tsc/en/Stage/Hell4.tsb"
BIN TSC_Hell42	"res/tsc/en/Stage/Hell42.tsb"
BIN TSC_Island	"res/tsc/en/Stage/Island.tsb"
BIN TSC_Itoh	"res/tsc/en/Stage/Itoh.tsb"
BIN TSC_Jail1	"res/tsc/en/Stage/Jail1.tsb"
BIN TSC_Jail2	"res/tsc/en/Stage/Jail2.tsb"
BIN TSC_Jenka1	"res/tsc/en/Stage/Jenka1.tsb"
BIN TSC_Jenka2	"res/tsc/en/Stage/Jenka2.tsb"
BIN TSC_Kings	"res/tsc/en/Stage/Kings.tsb"
BIN TSC_Little	"res/tsc/en/Stage/Little.tsb"
BIN TSC_Lounge	"res/tsc/en/Stage/Lounge.tsb"
BIN TSC_Malco	"res/tsc/en/Stage/Malco.tsb"
BIN TSC_Mapi	"res/tsc/en/Stage/Mapi.tsb"
BIN TSC_MazeA	"res/tsc/en/Stage/MazeA.tsb"
BIN TSC_MazeB	"res/tsc/en/Stage/MazeB.tsb"
BIN TSC_MazeD	"res/tsc/en/Stage/MazeD.tsb"
BIN TSC_MazeH	"res/tsc/en/Stage/MazeH.tsb"
BIN TSC_MazeI	"res/tsc/en/Stage/MazeI.tsb"
BIN TSC_MazeM	"res/tsc/en/Stage/MazeM.tsb"
BIN TSC_MazeO	"res/tsc/en/Stage/MazeO.tsb"
BIN TSC_MazeS	"res/tsc/en/Stage/MazeS.tsb"
BIN TSC_MazeW	"res/tsc/en/Stage/MazeW.tsb"
BIN TSC_MiBox	"res/tsc/en/Stage/MiBox.tsb"
BIN TSC_Mimi	"res/tsc/en/Stage/Mimi.tsb"
BIN TSC_Momo	"res/tsc/en/Stage/Momo.tsb"
BIN TSC_Oside	"res/tsc/en/Stage/Oside.tsb"
BIN TSC_Ostep	"res/tsc/en/Stage/Ostep.tsb"
BIN TSC_Pens1	"res/tsc/en/Stage/Pens1.tsb"
BIN TSC_Pens2	"res/tsc/en/Stage/Pens2.tsb"
BIN TSC_Pixel	"res/tsc/en/Stage/Pixel.tsb"
BIN TSC_Plant	"res/tsc/en/Stage/Plant.tsb"
BIN TSC_Pole	"res/tsc/en/Stage/Pole.tsb"
BIN TSC_Pool	"res/tsc/en/Stage/Pool.tsb"
BIN TSC_Prefa1	"res/tsc/en/Stage/Prefa1.tsb"
BIN TSC_Prefa2	"res/tsc/en/Stage/Prefa2.tsb"
BIN TSC_Priso1	"res/tsc/en/Stage/Priso1.tsb"
BIN TSC_Priso2	"res/tsc/en/Stage/Priso2.tsb"
BIN TSC_Ring1	"res/tsc/en/Stage/Ring1.tsb"
BIN TSC_Ring2	"res/tsc/en/Stage/Ring2.tsb"
BIN TSC_Ring3	"res/tsc/en/Stage/Ring3.tsb"
BIN TSC_River	"res/tsc/en/Stage/River.tsb"
BIN TSC_Sand	"res/tsc/en/Stage/Sand.tsb"
BIN TSC_SandE	"res/tsc/en/Stage/SandE.tsb"
BIN TSC_Santa	"res/tsc/en/Stage/Santa.tsb"
BIN TSC_Shelt	"res/tsc/en/Stage/Shelt.tsb"
BIN TSC_Start	"res/tsc/en/Stage/Start.tsb"
BIN TSC_Statue	"res/tsc/en/Stage/Statue.tsb"
BIN TSC_Stream	"res/tsc/en/Stage/Stream.tsb"
BIN TSC_Weed	"res/tsc/en/Stage/Weed.tsb"
BIN TSC_WeedB	"res/tsc/en/Stage/WeedB.tsb"
BIN TSC_WeedD	"res/tsc/en/Stage/WeedD.tsb"
BIN TSC_WeedS	"res/tsc/en/Stage/WeedS.tsb"
