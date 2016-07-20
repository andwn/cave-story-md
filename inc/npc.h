#ifndef INC_NPC_H_
#define INC_NPC_H_

#include "common.h"

/*
 * Cave Story has a file named npc.tbl that contains a bunch of information about each
 * NPC Type (361 of them). This file is included into the ROM without modification,
 * and this module is used to grab the values needed from it
 */

#define NPC_COUNT 361

// NPC Flags
// Player can't pass through the NPC
#define NPC_SOLID			0x0001
// Ignore tile type 0x44. Normally NPCs cannot pass them
#define NPC_IGNORE44		0x0002
// Can't be damaged and makes a *clink* sound
#define NPC_INVINCIBLE		0x0004
// Goes through walls, used on flying enemies
#define NPC_IGNORESOLID		0x0008
// I think only the Behemoth enemy uses this
#define NPC_BOUNCYTOP		0x0010
// Can be damaged with player bullets
#define NPC_SHOOTABLE		0x0020
// This seems to be like a normal solid but allows penetrating just 1 pixel into the hitbox
#define NPC_SPECIALSOLID	0x0040
// Touching the bottom, top, and back does not hurt
#define NPC_FRONTATKONLY	0x0080
// Custom, depends on the NPC type
#define NPC_OPTION1			0x0100
// Trigger event [Entity.event] on death
#define NPC_EVENTONDEATH	0x0200
// Drops health/energy/missiles
#define NPC_DROPPOWERUP		0x0400
// Only appears if the flag [Entity.id] is set
#define NPC_ENABLEONFLAG	0x0800
// Custom, depends on the NPC type
#define NPC_OPTION2			0x1000
// Calls event [Entity.event] when player presses down
#define NPC_INTERACTIVE		0x2000
// Only appears if the flag [Entity.id] is unset
#define NPC_DISABLEONFLAG	0x4000
// Pops up red numbers when damaged
#define NPC_SHOWDAMAGE		0x8000

// Enumeration of all object types, copied from NXEngine and used by AI
#define OBJ_NULL					0

#define OBJ_XP						1
#define OBJ_BEHEMOTH				2
#define OBJ_SMOKE_CLOUD				4
#define OBJ_CRITTER_HOPPING_GREEN	5
#define OBJ_BEETLE_GREEN			6
#define OBJ_BASIL					7
#define OBJ_BEETLE_FREEFLY			8
#define OBJ_BALROG_DROP_IN			9
#define OBJ_IGOR_SHOT				11
#define OBJ_BALROG					12
#define OBJ_FORCEFIELD				13
#define OBJ_SANTAS_KEY				14
#define OBJ_CHEST_CLOSED			15
#define OBJ_SAVE_POINT				16
#define OBJ_RECHARGE				17
#define OBJ_DOOR					18
#define OBJ_BALROG_BUST_IN			19
#define OBJ_COMPUTER				20
#define OBJ_CHEST_OPEN				21
#define OBJ_TELEPORTER				22
#define OBJ_TELEPORTER_LIGHTS		23
#define OBJ_POWER_CRITTER			24
#define OBJ_EGG_ELEVATOR			25		// lift platform in Egg Observation Room
#define OBJ_BAT_CIRCLE				26
#define OBJ_BIG_SPIKE				27
#define OBJ_CRITTER_FLYING			28		// cyan critters from grasstown
#define OBJ_CHTHULU					29
#define OBJ_HERMIT_GUNSMITH			30
#define OBJ_BAT_HANG				31
#define OBJ_LIFE_CAPSULE			32
#define OBJ_BALROG_SHOT_BOUNCE		33
#define OBJ_BED						34
#define OBJ_MANNAN					35
#define OBJ_BALROG_BOSS_FLYING		36
#define OBJ_SIGNPOST				37
#define OBJ_FIREPLACE				38
#define OBJ_SAVE_SIGN				39
#define OBJ_SANTA					40		// NPC Santa
#define OBJ_DOOR_BUSTED				41		// door of shack after balrog breaks it
#define OBJ_SUE						42
#define OBJ_CHALKBOARD				43
#define OBJ_POLISH					44
#define OBJ_POLISHBABY				45
#define OBJ_HVTRIGGER				46
#define OBJ_SANDCROC				47
#define OBJ_SKULLHEAD				49
#define OBJ_SKELETON_SHOT			50		// bones fired by skullhead_carried & skeleton
#define OBJ_CROWWITHSKULL			51
#define OBJ_BLUE_ROBOT_SITTING		52
#define OBJ_SKULLSTEP_FOOT			53
#define OBJ_SKULLSTEP				54
#define OBJ_KAZUMA					55
#define OBJ_BEETLE_BROWN			56
#define OBJ_CROW					57
#define OBJ_GIANT_BEETLE			58		// Basu
#define OBJ_DOOR_ENEMY				59		// attacking exit door from First Cave
#define OBJ_TOROKO					60
#define OBJ_KING					61
#define OBJ_KAZUMA_AT_COMPUTER		62
#define OBJ_TOROKO_SHACK			63		// scared toroko attacking in Shack
#define OBJ_CRITTER_HOPPING_BLUE	64		// from First Cave
#define OBJ_BAT_BLUE				65		// from First Cave
#define OBJ_MISERYS_BUBBLE			66		// misery's bubble that takes Toroko away in Shack
#define OBJ_MISERY_FLOAT			67
#define OBJ_BALROG_BOSS_RUNNING		68
#define OBJ_MUSHROOM_ENEMY			69
#define OBJ_HIDDEN_SPARKLE			70
#define OBJ_CHINFISH				71
#define OBJ_SPRINKLER				72
#define OBJ_WATER_DROPLET			73
#define OBJ_JACK					74		// guards cemetary door in Village
#define OBJ_KANPACHI_FISHING		75
#define OBJ_YAMASHITA_FLOWERS		76		// flowers grown in Yamashita Farm
#define OBJ_YAMASHITA_PAVILION		77		// Sandaime's Pavilion from Yamashita Farm
#define OBJ_POT						78
#define OBJ_MAHIN					79		// from village
#define OBJ_GRAVEKEEPER				80		// man with knife in Mimiga Graveyard
#define OBJ_GIANT_MUSHROOM_ENEMY	81
#define OBJ_MISERY_STAND			82
#define OBJ_NPC_IGOR				83
#define OBJ_GIANT_BEETLE_SHOT		84
#define OBJ_TERMINAL				85
#define OBJ_MISSILE					86
#define OBJ_HEART					87
#define OBJ_BOSS_IGOR				88
#define OBJ_BOSS_IGOR_DEFEATED		89
#define OBJ_CAGE					91
#define OBJ_SUE_AT_COMPUTER			92
#define OBJ_CHACO					93
#define OBJ_GIANT_JELLY				94
#define OBJ_JELLY					95
#define OBJ_FAN_LEFT				96
#define OBJ_FAN_UP					97
#define OBJ_FAN_RIGHT				98
#define OBJ_FAN_DOWN				99
#define OBJ_GRATE					100
#define OBJ_POWERCOMP				101
#define OBJ_POWERSINE				102
#define OBJ_MANNAN_SHOT				103
#define OBJ_FROG					104
#define OBJ_HEY						105		// "Hey!" balloon
#define OBJ_HEY_SPAWNER				106		// creates "Hey!" ballons
#define OBJ_MALCO					107
#define OBJ_BALFROG_SHOT			108
#define OBJ_MALCO_BROKEN			109
#define OBJ_MINIFROG				110
#define OBJ_PTELOUT					111		// player teleporting out (using teleporter)
#define OBJ_PTELIN					112		// player teleporting in (using teleporter)
#define OBJ_PROFESSOR_BOOSTER		113
#define OBJ_PRESS					114
#define OBJ_FRENZIED_MIMIGA			115
#define OBJ_RED_PETALS				116		// scattered around beds
#define OBJ_CURLY					117		// curly (npc)
#define OBJ_CURLY_BOSS				118		// curly (boss)
#define OBJ_TABLECHAIRS				119		// table & chairs
#define OBJ_MIMIGAC1				120		// curly's mimigas
#define OBJ_MIMIGAC2				121		// curly's mimigas
#define OBJ_MIMIGAC_ENEMY			122		// fighting mimigas during 1st boss fight in sand zone
#define OBJ_CURLYBOSS_SHOT			123
#define OBJ_SUNSTONE				124
#define OBJ_HIDDEN_POWERUP			125
#define OBJ_PUPPY_RUN				126
#define OBJ_PUPPY_WAG				130
#define OBJ_PUPPY_SLEEP				131
#define OBJ_PUPPY_BARK				132
#define OBJ_JENKA					133
#define OBJ_ARMADILLO				134
#define OBJ_SKELETON				135
#define OBJ_PUPPY_CARRY				136
#define OBJ_LARGEDOOR_FRAME			137
#define OBJ_LARGEDOOR				138
#define OBJ_DOCTOR					139
#define OBJ_TOROKO_FRENZIED			140
#define OBJ_TOROKO_BLOCK			141
#define OBJ_TOROKO_FLOWER			142
#define OBJ_JENKA_COLLAPSED			143
#define OBJ_TOROKO_TELEPORT_IN		144
#define OBJ_KINGS_SWORD				145		// sticks to linkedobject
#define OBJ_LIGHTNING				146
#define OBJ_CRITTER_SHOOTING_PURPLE	147
#define OBJ_CRITTER_SHOT			148
#define OBJ_BLOCK_MOVEH				149
#define OBJ_NPC_PLAYER				150		// the player as an NPC in cutscenes
#define OBJ_BLUE_ROBOT				151
#define OBJ_SHUTTER_STUCK			152
#define OBJ_GAUDI					153
#define OBJ_GAUDI_DYING				154
#define OBJ_GAUDI_FLYING			155
#define OBJ_GAUDI_FLYING_SHOT		156
#define OBJ_BLOCK_MOVEV				157
#define OBJ_X_FISHY_MISSILE			158		// homing fish projectiles fired by Monster X
#define OBJ_X_DEFEATED				159		// cat that falls out after defeating X
#define OBJ_POOH_BLACK				160		// clinic ghost
#define OBJ_POOH_BLACK_BUBBLE		161
#define OBJ_POOH_BLACK_DYING		162
#define OBJ_DR_GERO					163		// from labyrinth clinic
#define OBJ_NURSE_HASUMI			164		// from labyrinth clinic
#define OBJ_CURLY_COLLAPSED			165
#define OBJ_GAUDI_SHOPKEEP			166		// aka Chaba, at table in labyrinth shop
#define OBJ_BOOSTER_FALLING			167		// professor booster falling after Labyrinth
#define OBJ_BOULDER					168		// large boulder in the Boulder Chamber
#define OBJ_BALROG_BOSS_MISSILES	169		// balrog boss in Boulder Chamber
#define OBJ_BALROG_MISSILE			170
#define OBJ_FIREWHIRR				171
#define OBJ_FIREWHIRR_SHOT			172
#define OBJ_GAUDI_ARMORED			173
#define OBJ_GAUDI_ARMORED_SHOT		174
#define OBJ_GAUDI_EGG				175
#define OBJ_BUYOBUYO_BASE			176
#define OBJ_BUYOBUYO				177
#define OBJ_MINICORE_SHOT			178
#define OBJ_CORE_GHOSTIE			179
#define OBJ_CURLY_AI				180
#define OBJ_CAI_GUN					181
#define OBJ_CAI_MGUN				182
#define OBJ_CAI_WATERSHIELD			183
#define OBJ_SHUTTER_BIG				184
#define OBJ_SHUTTER					185
#define OBJ_ALMOND_LIFT				186
#define OBJ_FUZZ_CORE				187
#define OBJ_FUZZ					188
#define OBJ_ALMOND_ROBOT			190
#define OBJ_WATERLEVEL				191		// controls water level in Almond
#define OBJ_MOTORBIKE				192
#define OBJ_MOTORBIKE_BROKEN		193
#define OBJ_BLUE_ROBOT_REMAINS		194
#define OBJ_GRATING					195
#define OBJ_MOTION_WALL				196		// top & bottom wall during Ironhead battle
#define OBJ_IRONH_FISHY				197		// fishies in IronH battle
#define OBJ_IRONH_SHOT				198
#define OBJ_FAN_DROPLET				199		// air or water current (from fans and Waterway)
#define OBJ_DRAGON_ZOMBIE			200
#define OBJ_DRAGON_ZOMBIE_DEAD		201
#define OBJ_DRAGON_ZOMBIE_SHOT		202
#define OBJ_CRITTER_HOPPING_AQUA	203
#define OBJ_FALLING_SPIKE_SMALL		204
#define OBJ_FALLING_SPIKE_LARGE		205
#define OBJ_COUNTER_BOMB			206
#define OBJ_COUNTER_BOMB_NUMBER		207
#define OBJ_GIANT_BEETLE_2			208
#define OBJ_BEETLE_FREEFLY_2		210
#define OBJ_SPIKE_SMALL				211
#define OBJ_SKY_DRAGON				212		// kazuma's nice dragon (npc)
#define OBJ_NIGHT_SPIRIT			213
#define OBJ_NIGHT_SPIRIT_SHOT		214
#define OBJ_SANDCROC_OSIDE			215		// from outer wall
#define OBJ_PIXEL_CAT				216		// hidden pixel the cat from oside aka "debug kitty"
#define OBJ_ITOH					217
#define OBJ_CORE_BLAST				218
#define OBJ_BUBBLE_SPAWNER			219
#define OBJ_MIMIGA_FARMER_STANDING	220		// from plantation, doesn't move
#define OBJ_MIMIGA_FARMER_WALKING	221		// from plantation, walks back and forth
#define OBJ_JAIL_GRATING			222
#define OBJ_MOMORIN					223
#define OBJ_CHIE					224		// plantation lounge
#define OBJ_MEGANE					225		// plantation lounge (you give him the sprinkler)
#define OBJ_KANPACHI_STANDING		226		// standing version, plantation lounge
#define OBJ_BUCKET					227		// kanpachi's fishing bucket
#define OBJ_DROLL_GUARD				228		// droll that stomps you in Teleporter Room
#define OBJ_RED_FLOWERS_SPROUTS		229
#define OBJ_RED_FLOWERS_BLOOMING	230
#define OBJ_ROCKET					231		// ...that you ride up to Last Cave
#define OBJ_ORANGEBELL				232		// bat swarm from Plantation
#define OBJ_ORANGEBELL_BABY			233
#define OBJ_RED_FLOWERS_PICKED		234
#define OBJ_FLOWERS_PENS1			234		// picked red flowers in lower-right corner of Arthur's House
#define OBJ_MIDORIN					235
#define OBJ_GUNFISH					236
#define OBJ_GUNFISH_SHOT			237
#define OBJ_PROXIMITY_PRESS_HOZ		238		// horizontal-moving presses along rocket path
#define OBJ_MIMIGA_CAGE				239
#define OBJ_MIMIGA_JAILED			240		// mimigas in jail2
#define OBJ_CRITTER_HOPPING_RED		241
#define OBJ_RED_BAT					242
#define OBJ_RED_BAT_SPAWNER			243
#define OBJ_LAVA_DRIP				244
#define OBJ_LAVA_DRIP_SPAWNER		245		// for example at entrance to last cave (hidden)
#define OBJ_PROXIMITY_PRESS_VERT	246
#define OBJ_BOSS_MISERY				247
#define OBJ_MISERY_SHOT				248
#define OBJ_MISERY_PHASE			249		// 2 used in teleport effect
#define OBJ_MISERY_BALL				250
#define OBJ_BLACK_LIGHTNING			251
#define OBJ_MISERY_RING				252
#define OBJ_XP_CAPSULE				253
#define OBJ_HELICOPTER				254		// Balcony helicopter
#define OBJ_HELICOPTER_BLADE		255
#define OBJ_DOCTOR_CROWNED			256
#define OBJ_RED_CRYSTAL				257
#define OBJ_MIMIGA_SLEEPING			258		// plantation lounge
#define OBJ_CURLY_CARRIED			259		// curly being carried via Tow Rope
#define OBJ_MIMIGA_CAGED			260		// from Kings Table
#define OBJ_CHIE_CAGED				261		// from Kings Table
#define OBJ_CHACO_CAGED				262		// from Kings Table
#define OBJ_BOSS_DOCTOR				263
#define OBJ_DOCTOR_SHOT				264		// wave shot
#define OBJ_DOCTOR_SHOT_TRAIL		265
#define OBJ_DOCTOR_BLAST			266		// his explosion of red bouncy shots
#define OBJ_BOSS_DOCTOR_FRENZIED	267		// Muscle Doctor
#define OBJ_IGOR_BALCONY			268		// "igor"-like big Ravil on Balcony
#define OBJ_DOCTOR_BAT				269		// orange bats spawned by Doctor 2
#define OBJ_RED_ENERGY				270		// used by Doctor 2 and Undead Core cutscene
#define OBJ_IRONH_BRICK				271
#define OBJ_BRICK_SPAWNER			272
#define OBJ_DROLL_SHOT				273
#define OBJ_DROLL					274
#define OBJ_PUPPY_ITEMS				275		// wagging puppy such as that gives you life capsule
#define OBJ_RED_DEMON				276		// middle-boss in Last Cave Hidden
#define OBJ_RED_DEMON_SHOT			277
#define OBJ_LITTLE_FAMILY			278		// Little Man, and also used for his family
#define OBJ_FALLING_BLOCK			279		// from Misery and Hell B1
#define OBJ_SUE_TELEPORT_IN			280
#define OBJ_DOCTOR_GHOST			281		// doctor as red energy
#define OBJ_UDMINI_PLATFORM			282		// undead minicore (platforms)
#define OBJ_MISERY_FRENZIED			283		// for Final Battle transformed by DOCTOR_GHOST
#define OBJ_SUE_FRENZIED			284		// for Final Battle transformed by DOCTOR_GHOST
#define OBJ_UD_SPINNER				285
#define OBJ_UD_SPINNER_TRAIL		286
#define OBJ_UD_SMOKE				287
#define OBJ_UD_PELLET				288		// undead core pellet/rock shot
#define OBJ_MISERY_CRITTER			289
#define OBJ_MISERY_BAT				290
#define OBJ_UD_MINICORE_IDLE		291
#define OBJ_QUAKE					292
#define OBJ_UD_BLAST				293
#define OBJ_FALLING_BLOCK_SPAWNER	294
#define OBJ_CLOUD					295
#define OBJ_CLOUD_SPAWNER			296		// clouds from prtFall (ending sequence)
#define OBJ_INTRO_DOCTOR			298		// from intro
#define OBJ_INTRO_KINGS				299		// balrog/misery in bubble (from intro)
#define OBJ_INTRO_CROWN				300		// Demon Crown (from intro)
#define OBJ_MISERY_MISSILE			301
#define OBJ_SCROLL_CONTROLLER		302		// <FON on it and you can do various tricks with controlling the scrolling
#define OBJ_SANTA_CAGED				307		// from Kings Table
#define OBJ_GAUDI_PATIENT			304		// credits
#define OBJ_BABY_PUPPY				305		// credits
#define OBJ_BALROG_MEDIC			306		// credits
#define OBJ_STUMPY					308
#define OBJ_BUTE_FLYING				309
#define OBJ_BUTE_SWORD				310
#define OBJ_BUTE_ARCHER				311
#define OBJ_BUTE_ARROW				312
#define OBJ_MA_PIGNON				313
#define OBJ_MA_PIGNON_ROCK			314
#define OBJ_MA_PIGNON_CLONE			315
#define OBJ_BUTE_DYING				316
#define OBJ_MESA					317
#define OBJ_MESA_DYING				318
#define OBJ_MESA_BLOCK				319
#define OBJ_CURLY_CARRIED_SHOOTING	320
#define OBJ_CCS_GUN					321		// OBJ_CURLY_CARRIED_SHOOTING: her gun
#define OBJ_DELEET					322
#define OBJ_BUTE_FALLING			323
#define OBJ_BUTE_SPAWNER			324
#define OBJ_HP_LIGHTNING			325		// lightning shot & charging from Heavy Press
#define OBJ_TURNING_HUMAN			326		// Itoh/Sue turning human (credits)
#define OBJ_AHCHOO					327
#define OBJ_TRANSMOGRIFIER			328		// credits
#define OBJ_BUILDING_FAN			329		// credits
#define OBJ_ROLLING					330
#define OBJ_BALLOS_BONE				331
#define OBJ_BALLOS_BONE_SPAWNER		332
#define OBJ_BALLOS_TARGET			333
#define OBJ_STRAINING				334		// "straining" effect used in Boulder Chamber cutscene
#define OBJ_IKACHAN					335
#define OBJ_IKACHAN_SPAWNER			336
#define OBJ_NUMAHACHI				337		// in Plantation version of Statue Room
#define OBJ_GREEN_DEVIL				338
#define OBJ_GREEN_DEVIL_SPAWNER		339
#define OBJ_BALLOS_PRIEST			340		// form #1 in Seal Chamber
#define OBJ_BALLOS_SMILE			341		// his closed face that smiles at start of battle
#define OBJ_BALLOS_ROTATOR			342		// 3rd/4th form spiky rotators
#define OBJ_BALLOS_BODY_2			343		// time-limited body?
#define OBJ_BALLOS_EYE_2			344		// time-limited closed eyes?
#define OBJ_BALLOS_SKULL			345		// falling skulls during 3rd form
#define OBJ_BALLOS_PLATFORM			346		// platforms for 4th form
#define OBJ_HOPPY					347
#define OBJ_BALLOS_SPIKES			348
#define OBJ_STATUE_BASE				349		// statues in Statue Room (actionable base)
#define OBJ_BUTE_ARCHER_RED			350
#define OBJ_STATUE					351		// statues in Statue Room
#define OBJ_THE_CAST				352		// friends that surround player at end of credits
#define OBJ_BUTE_SWORD_RED			353
#define OBJ_WALL_COLLAPSER			354		// post-Ballos cutscene
#define OBJ_BALROG_PASSENGER		355		// Player/Curly when rescued from Seal Chamber by Balrog
#define OBJ_BALROG_FLYING			356		// best-ending flying-in-clouds cutscene
#define OBJ_PUPPY_GHOST				357		// Ballos's dog in Corridor/ostep
#define OBJ_MISERY_WIND				358		// seen in best-ending credits
#define OBJ_DROPLET_SPAWNER			359		// spawns small falling water drips from e.g. resevoir
#define OBJ_THANK_YOU				360		// credits

// NPC Flags
u16 npc_flags(u16 type);
// Starting health
u16 npc_health(u16 type);
// This is actually an index of which file contains the sprites.. don't use it
u8 npc_palette(u16 type);
// Sound to make when the NPC is damaged by player's weapon
u8 npc_hurtSound(u16 type);
// Sound to play when defeated
u8 npc_deathSound(u16 type);
// Type of smoke effect to create when defeated (small, mid, large)
u8 npc_deathSmoke(u16 type);
// Amount of weapon energy that is dropped
u16 npc_experience(u16 type);
// Damage to player when colliding
u16 npc_attack(u16 type);
// Area relative to the center where collision may take place
bounding_box npc_hitBox(u16 type);
// Area relative to the center where the sprite is displayed
bounding_box npc_displayBox(u16 type);

#endif /* INC_NPC_H_ */
