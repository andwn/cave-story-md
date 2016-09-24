#include "tables.h"

#include "ai.h"
#include "resources.h"
#include "sheet.h"

const npc_info_def npc_info[NPC_COUNT + 9 + 3] = {
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_op2snap, NULL, NULL },
	{ NULL, SHEET_ENERGY, 	PAL1, 1, &onspawn_energy, &ai_energy, NULL }, // Weapon Energy
	{ NULL, SHEET_BEHEM, 	PAL1, 1, NULL, &ai_behemoth, &ondeath_default }, // Behemoth
	{ NULL, NOSHEET, 		PAL0, 0, NULL, NULL, NULL },
	{ NULL, NOSHEET, 		PAL0, 0, NULL, NULL, NULL },
	{ NULL, SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ NULL, SHEET_BEETLE, 	PAL3, 1, NULL, &ai_beetle, &ondeath_default }, // Beetle
	{ NULL, SHEET_BASIL, 	PAL1, 1, &onspawn_basil, &ai_basil, NULL }, // Basil
	{ NULL, SHEET_BEETLE, 	PAL3, 1, &onspawn_beetleFollow, &ai_beetleFollow, &ondeath_default },
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ai_balrog_drop_in, NULL }, // Balrog
	{ NULL, NOSHEET, 		PAL0, 0, NULL, NULL, NULL },
	{ NULL, SHEET_IGORSHOT, PAL1, 1, NULL, &ai_genericproj, NULL }, // Igor Shot
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ai_balrog, NULL }, // Balrog
	{ NULL, SHEET_FFIELD, 	PAL1, 1, NULL, NULL, NULL }, // Forcefield
	{ &SPR_Key, NOSHEET, 	PAL1, 1, NULL, NULL, NULL }, // Santa's Key
	{ &SPR_Chest, NOSHEET, 	PAL1, 1, NULL, &ai_grav, NULL }, // Chest
	/* 0x010 (16) */
	{ &SPR_Save, NOSHEET, 	PAL1, 1, NULL, &ai_savepoint, NULL }, // Save Point
	{ &SPR_Refill, NOSHEET, PAL1, 1, NULL, &ai_refill, NULL }, // Refill
	{ &SPR_Door, NOSHEET, 	PAL1, 1, &onspawn_door, &ai_door, NULL }, // Door
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ai_balrog_bust_in, NULL }, // Balrog
	{ &SPR_Computer,NOSHEET,PAL1, 1, NULL, &ai_computer, NULL }, // Computer
	{ &SPR_Chest2, NOSHEET, PAL1, 1, &onspawn_snap, NULL, NULL }, // Open Chest
	{ &SPR_Tele, NOSHEET, 	PAL1, 1, NULL, NULL, NULL }, // Teleporter
	{ &SPR_TeleLight,NOSHEET,PAL1,1, &onspawn_teleLight, &ai_teleLight, NULL }, // Tele Light
	{ NULL, SHEET_PCRITTER, PAL0, 1, &onspawn_snap, &ai_critter, &ondeath_default },// Power Critter
	{ &SPR_Platform,NOSHEET,PAL1, 1, &onspawn_lift, &ai_lift, NULL }, // Lift Platform
	{ NULL, SHEET_BAT, 		PAL0, 1, NULL, &ai_batCircle, &ondeath_default }, // Bat
	{ NULL, SHEET_TRAP, 	PAL1, 1, NULL, NULL, NULL }, // Death Trap
	{ NULL, SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ &SPR_Cthu, NOSHEET, 	PAL3, 1, NULL, NULL, NULL }, // Cthulhu
	{ &SPR_Gunsmith,NOSHEET,PAL3, 1, &onspawn_snap, &ai_gunsmith, NULL }, // Gunsmith
	{ NULL, SHEET_BAT, 		PAL0, 1, &onspawn_batHang, &ai_batHang, &ondeath_default }, // Bat
	/* 0x020 (32) */
	{ &SPR_LifeUp, NOSHEET, PAL1, 1, NULL, &ai_lifeup, NULL }, // Life Capsule
	{ NULL, SHEET_IGORSHOT, PAL1, 1, NULL, &ai_balrogShot, NULL }, // Balrog Shot
	{ &SPR_Bed, NOSHEET, 	PAL1, 1, NULL, NULL, NULL }, // Bed
	{ NULL, SHEET_MANNAN, 	PAL3, 1, NULL, &ai_mannan, NULL }, // Mannan
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ondeath_balrogFlying, &ai_balrogFlying },
	{ &SPR_Sign, NOSHEET, 	PAL1, 1, NULL, NULL, NULL }, // Sign Post
	{ &SPR_Fire, NOSHEET, 	PAL1, 1, NULL, &ai_fireplace, NULL }, // Fireplace
	{ &SPR_SaveSign,NOSHEET,PAL1, 1, &onspawn_op2anim, NULL, NULL }, // Save Sign
	{ &SPR_Santa, NOSHEET, 	PAL3, 1, NULL, &ai_santa, NULL }, // Santa
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_door, NULL, NULL }, // Busted Doorway
	{ &SPR_Sue, NOSHEET, 	PAL3, 1, &onspawn_snapflip, &ai_sue, NULL }, // Sue
	{ &SPR_Board, NOSHEET, 	PAL1, 2, &onspawn_blackboard, NULL, NULL }, // Blackboard
	{ NULL, SHEET_POLISH, 	PAL3, 1, NULL, &ai_polish, &ondeath_default }, // Polish
	{ NULL, SHEET_BABY, 	PAL3, 1, NULL, &ai_baby, &ondeath_default }, // Baby
	{ NULL, NOSHEET, 		PAL0, 0, NULL, &ai_trigger, NULL }, // Trigger
	{ NULL, SHEET_CROC, 	PAL3, 2, NULL, &ai_sandcroc, &ondeath_default }, // Sandcroc
	/* 0x030 (48) */
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // Omega Shot
	{ NULL, SHEET_SKULLH, 	PAL1, 1, NULL, &ai_skullhead, &ondeath_default }, // Skullhead
	{ &SPR_Skullhead,NOSHEET,PAL1,1, NULL, &ai_skullhead, &ondeath_default }, // Skeleton
	{ NULL, SHEET_CROW, 	PAL3, 1, NULL, &ai_crow, &ondeath_default }, // Crow & Skullhead
	{ &SPR_Robot, NOSHEET, 	PAL3, 1, NULL, NULL, NULL }, // Blue Robot
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // ???UNKNOWN???
	{ NULL, SHEET_SKULLH, 	PAL1, 1, NULL, &ai_skullhead, &ondeath_default }, // Skullstep
	{ &SPR_Kazuma, NOSHEET, PAL3, 1, &onspawn_snap, &ai_kazuma, NULL }, // Kazuma
	{ NULL, SHEET_BEETLE, 	PAL3, 1, NULL, &ai_beetle, &ondeath_default }, // Beetle
	{ NULL, SHEET_CROW, 	PAL3, 1, NULL, &ai_crow, &ondeath_default }, // Crow
	{ NULL, SHEET_BASU, 	PAL3, 1, &onspawn_basu, &ai_basu, &ondeath_default }, // Basu
	{ &SPR_DoorE, NOSHEET,  PAL1, 1, NULL, &ai_theDoor, &ondeath_default }, // The Door
	{ &SPR_Toroko, NOSHEET, PAL3, 1, &onspawn_op2flip, &ai_toroko, NULL }, // Toroko
	{ &SPR_King, NOSHEET, 	PAL3, 1, &onspawn_op2flip, &ai_king, NULL }, // King
	{ &SPR_KazuCom,NOSHEET, PAL3, 1, &onspawn_snap, &ai_npc_at_computer, NULL }, // Kazuma Typing
	{ &SPR_Toroko, NOSHEET, PAL3, 1, &onspawn_torokoAtk, &ai_torokoAtk, NULL }, // Toroko
	/* 0x040 (64) */
	{ NULL, SHEET_CRITTER, 	PAL2, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ NULL, SHEET_BAT, 		PAL0, 1, &onspawn_batVertical, &ai_batVertical, &ondeath_default }, // Bat
	{ &SPR_Bubble, NOSHEET, PAL0, 1, NULL, &ai_misery_bubble, NULL }, // Misery's Bubble
	{ &SPR_Misery, NOSHEET, PAL0, 1, NULL, &ai_misery_float, NULL }, // Misery
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ai_balrogRunning, &ondeath_balrogRunning },
	{ NULL, SHEET_PIGNON, 	PAL1, 1, NULL, &ai_pignon, &ondeath_default }, // Pignon
	{ &SPR_Sparkle,NOSHEET, PAL1, 1, NULL, NULL, NULL }, // Sparkle
	{ &SPR_Fish, NOSHEET, 	PAL0, 1, NULL, &ai_chinfish, &ondeath_default }, // Chinfish
	{ &SPR_Sprinkler,NOSHEET,PAL1,1, NULL, &ai_sprinkler, NULL }, // Sprinkler
	{ NULL, SHEET_DROP, 	PAL1, 1, &onspawn_persistent, &ai_water_droplet, NULL }, // Water Drop
	{ &SPR_Jack, NOSHEET, 	PAL3, 1, &onspawn_snap, &ai_jack, NULL }, // Jack
	{ &SPR_Kanpachi,NOSHEET,PAL3, 1, NULL, &ai_kanpachi_fishing, NULL }, // Kanpachi
	{ NULL, SHEET_FLOWER, 	PAL3, 1, &onspawn_flower, NULL, NULL }, // Flowers
	{ &SPR_Sanda, NOSHEET, 	PAL3, 2, NULL, NULL, NULL }, // Sandame
	{ &SPR_Pot, NOSHEET, 	PAL1, 1, &onspawn_op2anim, NULL, NULL }, // Pot
	{ &SPR_Mahin, NOSHEET, 	PAL3, 1, &onspawn_snap, NULL, NULL }, // Mahin
	/* 0x050 (80) */
	{ &SPR_Keeper, NOSHEET, PAL1, 1, &onspawn_gkeeper, &ai_gkeeper, &ondeath_default }, // Gravekeeper
	{ &SPR_PignonB,NOSHEET, PAL1, 1, NULL, &ai_pignon, &ondeath_default }, // Big Pignon
	{ &SPR_Misery, NOSHEET, PAL0, 1, NULL, NULL, NULL }, // Misery
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igorscene, NULL }, // Igor
	{ NULL, SHEET_BASUSHOT, PAL0, 1, NULL, NULL, NULL }, // Basu Shot
	{ NULL, SHEET_TERM, 	PAL1, 1, NULL, &ai_terminal, NULL }, // Terminal
	{ NULL, SHEET_MISSILE, 	PAL1, 1, &onspawn_op2anim, &ai_missile, NULL }, // Missile
	{ NULL, SHEET_HEART, 	PAL1, 1, &onspawn_op2anim, &ai_heart, NULL }, // Heart
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igor, &ondeath_igor }, // Igor
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igordead, NULL }, // Igor
	{ NULL, NOSHEET, 		PAL0, 0, NULL, NULL, NULL },
	{ &SPR_Cage, NOSHEET, 	PAL1, 1, &onspawn_snap, NULL, NULL }, // Cage
	{ &SPR_SueCom, NOSHEET, PAL3, 1, &onspawn_snap, &ai_npc_at_computer, NULL }, // Sue Typing
	{ &SPR_Chaco, NOSHEET, 	PAL3, 1, NULL, &ai_chaco, NULL }, // Chaco
	{ &SPR_Kulala, NOSHEET, PAL0, 2, NULL, NULL, &ondeath_default }, // Kulala
	{ NULL,	SHEET_JELLY, 	PAL0, 1, NULL, &ai_jelly, &ondeath_default }, // Jelly
	/* 0x060 (96) */
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, NULL }, // Fans (L,U,R,D)
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, NULL },
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, NULL },
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, NULL },
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // Grate
	{ NULL, SHEET_POWERS, 	PAL1, 1, NULL, NULL, NULL }, // Power Screen
	{ NULL, SHEET_POWERF, 	PAL1, 1, &onspawn_powerc, NULL, NULL }, // Power Flow
	{ &SPR_ManShot,NOSHEET, PAL1, 1, NULL, &ai_mannanShot, NULL }, // Mannan Shot
	{ &SPR_Frog, NOSHEET, 	PAL3, 1, NULL, &ai_frog, &ondeath_default }, // Frog
	{ &SPR_Hey, NOSHEET, 	PAL1, 1, NULL, &ai_hey, NULL }, // "Hey!"
	{ &SPR_Hey, NOSHEET, 	PAL1, 1, NULL, &ai_hey, NULL }, // "Hey!" Emitter
	{ &SPR_Malco, NOSHEET, 	PAL1, 1, NULL, &ai_malco, NULL }, // Malco
	{ NULL, SHEET_REDSHOT, 	PAL1, 1, &onspawn_persistent, &ai_genericproj, NULL }, // Balfrog Shot
	{ &SPR_Malco, NOSHEET, 	PAL1, 1, &onspawn_malcoBroken, NULL, NULL }, // Malco
	{ NULL, SHEET_PUCHI, 	PAL3, 1, NULL, &ai_frog, &ondeath_default }, // Puchi
	{ &SPR_Quotele,NOSHEET, PAL0, 1, &onspawn_teleOut, &ai_teleOut, NULL }, // Tele Out
	/* 0x070 (112) */
	{ &SPR_Quotele,NOSHEET, PAL0, 1, &onspawn_teleIn, &ai_teleIn, NULL }, // Tele In
	{ &SPR_Booster,NOSHEET, PAL3, 1, NULL, &ai_booster, NULL }, // Prof. Booster
	{ &SPR_Press, NOSHEET, 	PAL1, 1, NULL, &ai_press, &ondeath_default }, // Press
	{ NULL, NOSHEET, 		PAL0, 2, NULL, NULL, &ondeath_default }, // Ravil
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // Red Flower Petals
	{ &SPR_Curly, NOSHEET, 	PAL3, 1, &onspawn_snap, &ai_curly, NULL }, // Curly
	{ &SPR_CurlyB, NOSHEET, PAL3, 1, &onspawn_snap, &ai_curlyBoss, &ondeath_curlyBoss }, // Curly
	{ &SPR_Table, NOSHEET, 	PAL1, 1, NULL, NULL, NULL }, // Table & Chair
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, NULL, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, NULL, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, NULL, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_MGUN, 	PAL0, 1, &onspawn_persistent, &ai_curlyBossShot, NULL }, // Curly Shot
	{ &SPR_Sunstone,NOSHEET,PAL2, 1, &onspawn_sunstone, &ai_sunstone, NULL }, // Sunstone
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_hiddenPowerup, &ai_hiddenPowerup, NULL },
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, NULL, NULL }, // Puppy (Running)
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // ???UNKNOWN???
	/* 0x080 (128) */
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // ???UNKNOWN???
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // ???UNKNOWN???
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, NULL, NULL }, // Puppy (Tail Wag)
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, NULL, NULL }, // Puppy (Sleeping)
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, NULL, NULL }, // Puppy (Barking)
	{ &SPR_Jenka, NOSHEET, 	PAL3, 1, NULL, &ai_jenka, NULL }, // Jenka
	{ &SPR_Armadl, NOSHEET, PAL3, 1, NULL, &ai_armadillo, &ondeath_default }, // Armadillo
	{ &SPR_Skullhead,NOSHEET,PAL3,1, NULL, &ai_skullhead, &ondeath_default }, // Skeleton
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppyCarry, &ai_puppyCarry, NULL }, // Puppy (Carried)
	{ &SPR_BigDoorFrame,NOSHEET,PAL1,2, NULL, NULL, NULL }, // Large Door (Frame)
	{ &SPR_BigDoor,NOSHEET, PAL1, 1, &onspawn_doorway, &ai_doorway, NULL }, // Large Door
	{ &SPR_Doctor, NOSHEET, PAL3, 1, &onspawn_snap, &ai_doctor, NULL }, // Doctor
	{ &SPR_ToroBoss,NOSHEET,PAL3, 2, &onspawn_torokoBoss, &ai_torokoBoss, &ondeath_torokoBoss },
	{ &SPR_ToroBlock,NOSHEET,PAL1,1, &onspawn_persistent, &ai_torokoBlock, NULL },
	{ &SPR_ToroFlower,NOSHEET,PAL3,1,&onspawn_persistent, &ai_torokoFlower, &ondeath_default },
	{ &SPR_Jenka, NOSHEET, 	PAL3, 1, &onspawn_jenka, NULL, NULL }, // Jenka
	/* 0x090 (144) */
	{ &SPR_Toroko, NOSHEET, PAL3, 1, NULL, &ai_toroko_teleport_in, NULL },
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL },
	{ NULL, NOSHEET, 		PAL0, 4, NULL, NULL, NULL }, // Lightning
	{ NULL, NOSHEET, 		PAL2, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Purple Critter
	{ &SPR_LabShot,NOSHEET, PAL2, 1, &onspawn_persistent, NULL, NULL }, // Critter Shot
	{ &SPR_MazeBlock,NOSHEET,PAL2,1, &onspawn_block, &ai_blockh, NULL }, // Moving Block (H)
	{ &SPR_Quote, NOSHEET, 	PAL0, 1, &onspawn_persistent, &ondeath_player, &ai_player }, // Quote
	{ &SPR_Robot, NOSHEET, 	PAL3, 1, NULL, &ai_blue_robot, NULL }, // Blue Robot
	{ NULL, NOSHEET, 		PAL2, 1, NULL, &ai_shutter_stuck, &ondeath_default }, // Shutter (Stuck)
	{ NULL, SHEET_GAUDI, 	PAL2, 1, NULL, &ai_gaudi, &ondeath_default }, // Gaudi
	{ NULL, SHEET_GAUDID, 	PAL2, 1, NULL, &ai_gaudiDying, &ondeath_default }, // Gaudi (Dying)
	{ NULL, SHEET_GAUDI, 	PAL2, 1, NULL, &ai_gaudiFlying, &ondeath_default }, // Gaudi (Flying)
	{ &SPR_LabShot,NOSHEET, PAL2, 1, &onspawn_persistent, &ai_gaudiArmoredShot, NULL },
	{ &SPR_MazeBlock,NOSHEET,PAL2,1, &onspawn_block, &ai_blockv, NULL }, // Moving Block (V)
	{ NULL, NOSHEET, 		PAL0, 1, NULL, NULL, NULL }, // Monster X Shot
	{ NULL, NOSHEET, 		PAL0, 4, NULL, NULL, NULL }, // Monster X Cat
	/* 0x0A0 (160) */
	{ NULL, SHEET_DARK, 	PAL3, 2, NULL, &ai_pooh_black, &ondeath_default }, // Pooh Black
	{ NULL, SHEET_DARKBUB, 	PAL3, 1, NULL, &ai_pooh_black_bubble, NULL }, // Pooh Black Bubble
	{ NULL, SHEET_DARK, 	PAL3, 2, NULL, &ai_pooh_black_dying, NULL }, // Pooh Black (Dying)
	{ &SPR_DrGero, NOSHEET, PAL3, 1, NULL, &generic_npc_states, NULL }, // Dr. Gero
	{ &SPR_Nurse, NOSHEET, 	PAL3, 1, NULL, &generic_npc_states, NULL }, // Nurse Hasumi
	{ &SPR_Curly, NOSHEET, 	PAL3, 1, NULL, &ai_curly, NULL }, // Curly (Collapsed)
	{ &SPR_GaudiMerch,NOSHEET,PAL3,1,NULL, NULL, NULL }, // Chaba
	{ &SPR_Booster,NOSHEET, PAL3, 1, &onspawn_persistent, &ai_booster_falling, NULL }, // Booster
	{ &SPR_Boulder,NOSHEET, PAL3, 4, NULL, &ai_boulder, NULL }, // Boulder
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_balrog, &ai_balrog_boss_missiles, &ondeath_default },
	{ NULL, NOSHEET, 		PAL0, 1, NULL, &ai_balrog_missile, NULL }, // Balrog Missile
	{ &SPR_CoreThing,NOSHEET,PAL2,1, NULL, &ai_firewhirr, &ondeath_default }, // Fire Whirr
	{ &SPR_CoreThingShot,NOSHEET,PAL2,1, &onspawn_persistent, &ai_firewhirr_shot, NULL },
	{ &SPR_GaudiArmor,NOSHEET,PAL2,1,NULL, &ai_gaudiArmored, NULL },
	{ NULL, NOSHEET, 		PAL2, 1, &onspawn_persistent, NULL, NULL }, // Gaudi Shot
	{ &SPR_GaudiEgg,NOSHEET,PAL2, 1, NULL, &ai_gaudi_egg, NULL }, // Gaudi Egg
	/* 0x0B0 (176) */
	{ &SPR_BuyoBase, NOSHEET, PAL2, 1, // Buyobuyo Base
		NULL, &ai_buyobuyo_base, NULL },
	{ &SPR_Buyo, 	NOSHEET, PAL2, 1, // Buyobuyo
		NULL, &ai_buyobuyo, &ondeath_default },
	{ &SPR_CoreShot1, NOSHEET, PAL3, 1, // Core Spinning Projectile
		&onspawn_persistent, &ai_minicore_shot, NULL },
	{ &SPR_CoreShot2, NOSHEET, PAL3, 1, // Core Wisp Projectile
		&onspawn_persistent, &ai_core_ghostie, NULL },
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, // Curly (AI)
		&onspawn_persistent, &ai_curly_ai, NULL },
	{ &SPR_Polar, 	NOSHEET, PAL0, 1, // Curly Polar Star
		&onspawn_persistent, &ai_cai_gun, NULL },
	{ &SPR_MGun, 	NOSHEET, PAL1, 1, // Curly Machine Gun
		&onspawn_persistent, &ai_cai_gun, NULL },
	{ &SPR_Bubble, 	NOSHEET, PAL0, 1, // Curly Bubble Shield
		&onspawn_persistent, &ai_cai_watershield, NULL },
	{ NULL, 		NOSHEET, PAL2, 1, // Shutter (Large)
		NULL, &ai_shutter, NULL },
	{ NULL, 		NOSHEET, PAL2, 1, // Shutter (Small)
		NULL, &ai_shutter, NULL },
	{ &SPR_Baby, 		NOSHEET, PAL2, 1, // Lift Block
		NULL, &ai_shutter, NULL },
	{ &SPR_FuzzCore, NOSHEET, PAL2, 1, // Fuzz Core
		NULL, &ai_fuzz_core, &ondeath_default },
	{ &SPR_Fuzz, 	NOSHEET, PAL2, 1, // Fuzz
		NULL, &ai_fuzz, &ondeath_default },
	{ &SPR_CoreShot4, NOSHEET, PAL3, 1, // Homing Flame Projectile
		&onspawn_persistent, &ai_core_blast, NULL },
	{ NULL, 		NOSHEET, PAL0, 1, // Surface Robot
		NULL, &ai_almond_robot, NULL },
	{ NULL, 		NOSHEET, PAL0, 2, // Water Level
		&onspawn_persistent, &ai_waterlevel, NULL },
	/* 0x0C0 (192) */
	{ &SPR_Buggy, 	NOSHEET, PAL1, 1, // Scooter
		NULL, &ai_motorbike, NULL },
	{ &SPR_Buggy2, 	NOSHEET, PAL1, 2, // Scooter (Pieces)
		NULL, NULL, NULL },
	{ &SPR_Robot, 	NOSHEET, PAL3, 1, // Blue Robot (Pieces)
		NULL, NULL, NULL },
	{ NULL, 		NOSHEET, PAL0, 1, // Grate Mouth
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Motion Wall
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Porcupine Fish
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ironhead Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Underwater Current
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Dragon Zombie
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Dragon Zombie (Dead)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Dragon Zombie Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Blue Hopping Critter
		&onspawn_snap, &ai_critter, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling Spike (Small)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling Spike (Large)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Counter Bomb
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Countdown Balloon
		NULL, NULL, &ondeath_default },
	// 0x0D0 (208) */
	{ NULL, 		NOSHEET, PAL0, 1, // Basu (2)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Basu Projectile (2)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Green Beetle (Follow 2)
		NULL, NULL, &ondeath_default },
	{ &SPR_Spikes, 	NOSHEET, PAL1, 1, // Spikes
		&onspawn_spike, NULL, NULL },
	{ NULL, 		NOSHEET, PAL0, 1, // Sky Dragon
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Night Spirit
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Night Spirit Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // White Sandcroc
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Debug Cat
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Itoh
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Smoke/Current Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Shovel Brigade
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Shovel Brigade (Walking)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Prison Bars
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Momorin
		NULL, NULL, &ondeath_default },
	/* 0x0E0 (224) */
	{ NULL, 		NOSHEET, PAL0, 1, // Chie
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Megane
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Kanpachi
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bucket
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Droll (Guard)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Flower Sprouts
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Blooming Red Flowers
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Rocket
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orangebell
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Picked Red Flowers
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Midorin
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Gunfish
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Gunfish Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Lethal Press
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Cage Bars
		NULL, NULL, &ondeath_default },
	/* 0x0F0 (240) */
	{ NULL, 		NOSHEET, PAL0, 1, // Jailed Mimiga
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Critter
		&onspawn_snap, &ai_critter, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bat
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bat Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Acid Drop
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Acid Drop Generator
		NULL, NULL, &ondeath_default },
	{ &SPR_Press, 	NOSHEET, PAL1, 1, // Press (Proximity)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Boss)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Vanish)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery Energy Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery Lightning Ball
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Misery Lightning
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Energy Capsule
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Helicopter
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	/* 0x100 (256) */
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Before Fight)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Crystal
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sleeping Mimiga
		NULL, NULL, &ondeath_default },
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, // Curly (Carried)
		&onspawn_persistent, &ai_curly_carried, NULL },
	{ NULL, 		NOSHEET, PAL0, 1, // Shovel Brigade (Caged)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Chie (Caged)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Chaco (Caged)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Boss)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Wave
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Ball (Fast)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Ball (Slow)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Muscle Doctor (Boss)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Igor
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Energy Bat
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Energy
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Underwater Block
		NULL, NULL, &ondeath_default },
	/* 0x110 (272) */
	{ NULL, 		NOSHEET, PAL0, 0, // Water Block Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Droll Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Droll
		NULL, NULL, &ondeath_default },
	{ &SPR_Puppy, 	NOSHEET, PAL1, 1, // Puppy (With Item)
		&onspawn_puppy, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Demon
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Demon Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Little Family
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling Block (Large)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sue (Teleport In)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Red Energy)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Mini Undead Core
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Transformed)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sue (Transformed)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Spinning Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Dot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Smoke
		NULL, NULL, &ondeath_default },
	/* 0x120 (288) */
	{ NULL, 		NOSHEET, PAL0, 1, // Glowing Rock Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Critter
		&onspawn_snap, &ai_critter, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Bat
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Mini Core (Before Fight)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Quake
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Huge Energy Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Falling Block Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Cloud
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Cloud Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Uncrowned)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Balrog/Misery (Bubble)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Demon Crown
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Fish Missile (Orange)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	/* 0x130 (304) */
	{ NULL, 		NOSHEET, PAL0, 1, // Gaudi (Sitting)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Puppy (Small)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Balrog (Nurse)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Santa (Caged)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Stumpy
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Sword)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Archer)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ma Pignon
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling (?)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Hopping Enemy (?)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Defeated)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Mesa
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Mesa (Defeated)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	/* 0x140 (320) */
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, // Curly (Hell)
		NULL, NULL, NULL },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Deleet
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Generated)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Bute Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Heavy Press Projectile
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Itoh/Sue (Ending)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Transmogrifier
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Building Fan
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Rolling
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Bone Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Shockwave
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Ballos Lightning
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sweat
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ika-chan
		NULL, NULL, &ondeath_default },
	/* 0x150 (336) */
	{ NULL, 		NOSHEET, PAL0, 0, // Ika-chan Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Numahachi
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Green Devil
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Green Devil Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos (Boss)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Skull Shot
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Hoppy
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Spikes
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Statue
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bute (Archer)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Statue (Breakable)
		NULL, NULL, &ondeath_default },
	/* 0x160 (352) */
	{ NULL, 		NOSHEET, PAL0, 1, // King (Sword)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bute (Sword)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Kill Zone
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Balrog (Rescue)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Puppy (Ghost)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Wind)
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Water Drop Generator
		NULL, NULL, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Thank You
		NULL, NULL, &ondeath_default },
	/* ^ 0x168 (360) ^ */
	/* MAJOR BOSSES - Don't exist in npc.tbl, but need AI methods */
	/* 0x169 (361) */
	{ &SPR_Omega, 	NOSHEET, PAL3, 3, // Omega
		&onspawn_omega, &ai_omega, &ondeath_omega },
	{ &SPR_Balfrog1, NOSHEET, PAL3, 3, // Balfrog
		&onspawn_balfrog, &ai_balfrog, &ondeath_balfrog },
	{ NULL, NOSHEET, PAL3, 3, // Monster X
		NULL, NULL, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Core (Controller)
		&onspawn_core, &ai_core, &ondeath_core },
	{ NULL, NOSHEET, PAL3, 3, // Ironhead
		NULL, NULL, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Sisters
		NULL, NULL, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Undead Core
		NULL, NULL, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Heavy Press
		NULL, NULL, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Ballos
		NULL, NULL, NULL },
	/* ^ 0x171 (369) ^ */
	/* BOSS PARTS - Separate entities belonging to the bosses which don't
	 * already exist in the NPC table, for whatever reason */
	/* 0x172 (370) */
	{ NULL, NOSHEET, PAL3, 3, // Core (Front)
		&onspawn_persistent, &ai_core_front, NULL },
	{ NULL, NOSHEET, PAL3, 3, // Core (Back)
		&onspawn_persistent, &ai_core_back, NULL },
	{ &SPR_MiniCore, NOSHEET, PAL3, 2, // Mini Core
		&onspawn_persistent, &ai_minicore, NULL },
};
