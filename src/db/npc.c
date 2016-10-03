#include "tables.h"

#include "ai_gen.h"
#include "resources.h"
#include "sheet.h"

const npc_info_def npc_info[NPC_COUNT + 9 + 8] = {
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_op2snap, &ai_null, &ai_null },
	{ NULL, SHEET_ENERGY, 	PAL1, 1, &onspawn_energy, &ai_energy, &ai_null }, // Weapon Energy
	{ NULL, SHEET_BEHEM, 	PAL1, 1, &ai_null, &ai_behemoth, &ondeath_default }, // Behemoth
	{ NULL, NOSHEET, 		PAL0, 0, &ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, 		PAL0, 0, &ai_null, &ai_null, &ai_null },
	{ NULL, SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ NULL, SHEET_BEETLE, 	PAL3, 1, &ai_null, &ai_beetle, &ondeath_default }, // Beetle
	{ NULL, SHEET_BASIL, 	PAL1, 1, &onspawn_basil, &ai_basil, &ai_null }, // Basil
	{ NULL, SHEET_BEETLE, 	PAL3, 1, &onspawn_beetleFollow, &ai_beetleFollow, &ondeath_default },
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrog_drop_in, &ai_null }, // Balrog
	{ NULL, NOSHEET, 		PAL0, 0, &ai_null, &ai_null, &ai_null },
	{ NULL, SHEET_IGORSHOT, PAL1, 1, &onspawn_persistent, &ai_genericproj, &ai_null }, // Igor Shot
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrog, &ai_null }, // Balrog
	{ NULL, SHEET_FFIELD, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Forcefield
	{ &SPR_Key, NOSHEET, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Santa's Key
	{ &SPR_Chest, NOSHEET, 	PAL1, 1, &ai_null, &ai_chest, &ai_null }, // Chest
	/* 0x010 (16) */
	{ &SPR_Save, NOSHEET, 	PAL1, 1, &ai_null, &ai_savepoint, &ai_null }, // Save Point
	{ &SPR_Refill, NOSHEET, PAL1, 1, &ai_null, &ai_refill, &ai_null }, // Refill
	{ &SPR_Door, NOSHEET, 	PAL1, 1, &onspawn_door, &ai_door, &ai_null }, // Door
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrog_bust_in, &ai_null }, // Balrog
	{ &SPR_Computer,NOSHEET,PAL1, 1, &ai_null, &ai_computer, &ai_null }, // Computer
	{ &SPR_Chest2, NOSHEET, PAL1, 1, &onspawn_snap, &ai_null, &ai_null }, // Open Chest
	{ &SPR_Tele, NOSHEET, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Teleporter
	{ &SPR_TeleLight,NOSHEET,PAL1,1, &onspawn_teleLight, &ai_teleLight, &ai_null }, // Tele Light
	{ NULL, SHEET_PCRITTER, PAL0, 1, &onspawn_snap, &ai_critter, &ondeath_default },// Power Critter
	{ &SPR_Platform,NOSHEET,PAL1, 1, &onspawn_lift, &ai_lift, &ai_null }, // Lift Platform
	{ NULL, SHEET_BAT, 		PAL0, 1, &ai_null, &ai_batCircle, &ondeath_default }, // Bat
	{ NULL, SHEET_TRAP, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Death Trap
	{ NULL, SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ &SPR_Cthu, NOSHEET, 	PAL3, 1, &ai_null, &ai_null, &ai_null }, // Cthulhu
	{ &SPR_Gunsmith,NOSHEET,PAL3, 1, &onspawn_snap, &ai_gunsmith, &ai_null }, // Gunsmith
	{ NULL, SHEET_BAT, 		PAL0, 1, &onspawn_batHang, &ai_batHang, &ondeath_default }, // Bat
	/* 0x020 (32) */
	{ &SPR_LifeUp, NOSHEET, PAL1, 1, &ai_null, &ai_lifeup, &ai_null }, // Life Capsule
	{ NULL, SHEET_IGORSHOT, PAL1, 1, &ai_null, &ai_balrogShot, &ai_null }, // Balrog Shot
	{ &SPR_Bed, NOSHEET, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Bed
	{ NULL, SHEET_MANNAN, 	PAL3, 1, &ai_null, &ai_mannan, &ai_null }, // Mannan
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrogFlying, &ondeath_balrogFlying },
	{ &SPR_Sign, NOSHEET, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Sign Post
	{ &SPR_Fire, NOSHEET, 	PAL1, 1, &ai_null, &ai_fireplace, &ai_null }, // Fireplace
	{ &SPR_SaveSign,NOSHEET,PAL1, 1, &onspawn_op2anim, &ai_null, &ai_null }, // Save Sign
	{ &SPR_Santa, NOSHEET, 	PAL3, 1, &ai_null, &ai_santa, &ai_null }, // Santa
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_door, &ai_null, &ai_null }, // Busted Doorway
	{ &SPR_Sue, NOSHEET, 	PAL3, 1, &onspawn_sue, &ai_sue, &ai_null }, // Sue
	{ &SPR_Board, NOSHEET, 	PAL1, 2, &onspawn_blackboard, &ai_null, &ai_null }, // Blackboard
	{ NULL, SHEET_POLISH, 	PAL3, 1, &ai_null, &ai_polish, &ondeath_default }, // Polish
	{ NULL, SHEET_BABY, 	PAL3, 1, &ai_null, &ai_baby, &ondeath_default }, // Baby
	{ NULL, NOSHEET, 		PAL0, 0, &ai_null, &ai_trigger, &ai_null }, // Trigger
	{ NULL, SHEET_CROC, 	PAL3, 2, &ai_null, &ai_sandcroc, &ondeath_default }, // Sandcroc
	/* 0x030 (48) */
	{ &SPR_OmgShot,NOSHEET, PAL3, 1, &onspawn_persistent, &ai_omega_shot, &ai_null }, // Omega Shot
	{ NULL, SHEET_SKULLH, 	PAL1, 1, &ai_null, &ai_skullhead, &ondeath_default }, // Skullhead
	{ &SPR_Skullhead,NOSHEET,PAL1,1, &ai_null, &ai_skullhead, &ondeath_default }, // Skeleton
	{ NULL, SHEET_CROW, 	PAL3, 1, &ai_null, &ai_crow, &ondeath_default }, // Crow & Skullhead
	{ &SPR_Robot, NOSHEET, 	PAL3, 1, &ai_null, &ai_null, &ai_null }, // Blue Robot
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???UNKNOWN???
	{ NULL, SHEET_SKULLH, 	PAL1, 1, &ai_null, &ai_skullhead, &ondeath_default }, // Skullstep
	{ &SPR_Kazuma, NOSHEET, PAL3, 1, &onspawn_snap, &ai_kazuma, &ai_null }, // Kazuma
	{ NULL, SHEET_BEETLE, 	PAL3, 1, &ai_null, &ai_beetle, &ondeath_default }, // Beetle
	{ NULL, SHEET_CROW, 	PAL3, 1, &ai_null, &ai_crow, &ondeath_default }, // Crow
	{ &SPR_Basu, NOSHEET, 	PAL3, 1, &onspawn_basu, &ai_basu, &ondeath_default }, // Basu
	{ &SPR_DoorE, NOSHEET,  PAL1, 1, &ai_null, &ai_theDoor, &ondeath_default }, // The Door
	{ &SPR_Toroko, NOSHEET, PAL3, 1, &onspawn_op2flip, &ai_toroko, &ai_null }, // Toroko
	{ &SPR_King, NOSHEET, 	PAL3, 1, &onspawn_op2flip, &ai_king, &ai_null }, // King
	{ &SPR_KazuCom,NOSHEET, PAL3, 1, &onspawn_snap, &ai_npc_at_computer, &ai_null }, // Kazuma Typing
	{ &SPR_Toroko, NOSHEET, PAL3, 1, &onspawn_torokoAtk, &ai_torokoAtk, &ai_null }, // Toroko
	/* 0x040 (64) */
	{ NULL, SHEET_CRITTER, 	PAL2, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Critter
	{ NULL, SHEET_BAT, 		PAL0, 1, &onspawn_batVertical, &ai_batVertical, &ondeath_default }, // Bat
	{ &SPR_Bubble, NOSHEET, PAL0, 1, &onspawn_persistent, &ai_misery_bubble, &ai_null }, // Misery's Bubble
	{ &SPR_Misery, NOSHEET, PAL0, 1, &onspawn_persistent, &ai_misery_float, &ai_null }, // Misery
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrogRunning, &ondeath_balrogRunning },
	{ NULL, SHEET_PIGNON, 	PAL1, 1, &ai_null, &ai_pignon, &ondeath_default }, // Pignon
	{ &SPR_Sparkle,NOSHEET, PAL1, 1, &ai_null, &ai_sparkle, &ai_null }, // Sparkle
	{ &SPR_Fish, NOSHEET, 	PAL0, 1, &ai_null, &ai_chinfish, &ondeath_default }, // Chinfish
	{ &SPR_Sprinkler,NOSHEET,PAL1,1, &ai_null, &ai_sprinkler, &ai_null }, // Sprinkler
	{ NULL, SHEET_DROP, 	PAL1, 1, &onspawn_persistent, &ai_water_droplet, &ai_null }, // Water Drop
	{ &SPR_Jack, NOSHEET, 	PAL3, 1, &onspawn_snap, &ai_jack, &ai_null }, // Jack
	{ &SPR_Kanpachi,NOSHEET,PAL3, 1, &ai_null, &ai_kanpachi_fishing, &ai_null }, // Kanpachi
	{ NULL, SHEET_FLOWER, 	PAL3, 1, &onspawn_flower, &ai_null, &ai_null }, // Flowers
	{ &SPR_Sanda, NOSHEET, 	PAL3, 2, &ai_null, &ai_null, &ai_null }, // Sandame
	{ &SPR_Pot, NOSHEET, 	PAL1, 1, &onspawn_op2anim, &ai_null, &ai_null }, // Pot
	{ &SPR_Mahin, NOSHEET, 	PAL3, 1, &onspawn_snap, &ai_mahin, &ai_null }, // Mahin
	/* 0x050 (80) */
	{ &SPR_Keeper, NOSHEET, PAL1, 1, &onspawn_gkeeper, &ai_gkeeper, &ondeath_default }, // Gravekeeper
	{ NULL,SHEET_PIGNONB, 	PAL1, 1, &ai_null, &ai_pignon, &ondeath_default }, // Big Pignon
	{ &SPR_Misery, NOSHEET, PAL0, 1, &onspawn_persistent, &ai_misery_stand, &ai_null }, // Misery
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igorscene, &ai_null }, // Igor
	{ NULL, SHEET_BASUSHOT, PAL0, 1, &onspawn_persistent, &ai_genericproj, &ai_null }, // Basu Shot
	{ NULL, SHEET_TERM, 	PAL1, 1, &ai_null, &ai_terminal, &ai_null }, // Terminal
	{ NULL, SHEET_MISSILE, 	PAL1, 1, &onspawn_op2anim, &ai_missile, &ai_null }, // Missile
	{ NULL, SHEET_HEART, 	PAL1, 1, &onspawn_op2anim, &ai_heart, &ai_null }, // Heart
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igor, &ondeath_igor }, // Igor
	{ &SPR_Igor, NOSHEET, 	PAL3, 4, &onspawn_igor, &ai_igordead, &ai_null }, // Igor
	{ NULL, NOSHEET, 		PAL0, 0, &ai_null, &ai_null, &ai_null },
	{ &SPR_Cage, NOSHEET, 	PAL1, 1, &onspawn_snap, &ai_null, &ai_null }, // Cage
	{ &SPR_SueCom, NOSHEET, PAL3, 1, &onspawn_snap, &ai_npc_at_computer, &ai_null }, // Sue Typing
	{ &SPR_Chaco, NOSHEET, 	PAL3, 1, &ai_null, &ai_chaco, &ai_null }, // Chaco
	{ &SPR_Kulala, NOSHEET, PAL0, 2, &ai_null, &ai_kulala, &ondeath_default }, // Kulala
	{ NULL,	SHEET_JELLY, 	PAL0, 1, &ai_null, &ai_jelly, &ondeath_default }, // Jelly
	/* 0x060 (96) */
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, &ai_null }, // Fans (L,U,R,D)
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, &ai_null },
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, &ai_null },
	{ NULL, SHEET_FAN, 		PAL1, 1, &onspawn_fan, &ai_fan, &ai_null },
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // Grate
	{ NULL, SHEET_POWERS, 	PAL1, 1, &ai_null, &ai_powers, &ai_null }, // Power Screen
	{ NULL, SHEET_POWERF, 	PAL1, 1, &onspawn_powerc, &ai_powerc, &ai_null }, // Power Flow
	{ &SPR_ManShot,NOSHEET, PAL1, 1, &ai_null, &ai_mannanShot, &ai_null }, // Mannan Shot
	{ NULL, SHEET_FROG, 	PAL3, 1, &onspawn_frog, &ai_frog, &ondeath_default }, // Frog
	{ &SPR_Hey, NOSHEET, 	PAL1, 1, &ai_null, &ai_hey, &ai_null }, // "Hey!"
	{ &SPR_Hey, NOSHEET, 	PAL1, 1, &ai_null, &ai_hey, &ai_null }, // "Hey!" Emitter
	{ &SPR_Malco, NOSHEET, 	PAL1, 1, &ai_null, &ai_malco, &ai_null }, // Malco
	{ NULL, SHEET_REDSHOT, 	PAL1, 1, &onspawn_persistent, &ai_genericproj, &ai_null }, // Balfrog Shot
	{ &SPR_Malco, NOSHEET, 	PAL1, 1, &onspawn_malcoBroken, &ai_null, &ai_null }, // Malco
	{ NULL, SHEET_PUCHI, 	PAL3, 1, &onspawn_frog, &ai_frog, &ondeath_default }, // Puchi
	{ &SPR_Quotele,NOSHEET, PAL0, 1, &onspawn_teleOut, &ai_teleOut, &ai_null }, // Tele Out
	/* 0x070 (112) */
	{ &SPR_Quotele,NOSHEET, PAL0, 1, &onspawn_teleIn, &ai_teleIn, &ai_null }, // Tele In
	{ &SPR_Booster,NOSHEET, PAL3, 1, &ai_null, &ai_booster, &ai_null }, // Prof. Booster
	{ &SPR_Press, NOSHEET, 	PAL1, 1, &ai_null, &ai_press, &ondeath_default }, // Press
	{ NULL, NOSHEET, 		PAL0, 2, &ai_null, &ai_null, &ondeath_default }, // Ravil
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // Red Flower Petals
	{ &SPR_Curly, NOSHEET, 	PAL3, 1, &onspawn_snap, &ai_curly, &ai_null }, // Curly
	{ &SPR_CurlyB, NOSHEET, PAL3, 1, &onspawn_snap, &ai_curlyBoss, &ondeath_curlyBoss }, // Curly
	{ &SPR_Table, NOSHEET, 	PAL1, 1, &ai_null, &ai_null, &ai_null }, // Table & Chair
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, &ai_null, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, &ai_null, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_BARMIMI, 	PAL3, 1, &ai_null, &ai_curlys_mimigas, &ondeath_default },
	{ NULL, SHEET_MGUN, 	PAL0, 1, &onspawn_persistent, &ai_curlyBossShot, &ai_null }, // Curly Shot
	{ &SPR_Sunstone,NOSHEET,PAL2, 1, &onspawn_sunstone, &ai_sunstone, &ai_null }, // Sunstone
	{ NULL, NOSHEET, 		PAL0, 0, &onspawn_hiddenPowerup, &ai_hiddenPowerup, &ai_null },
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, &ai_null, &ai_null }, // Puppy (Running)
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???UNKNOWN???
	/* 0x080 (128) */
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???UNKNOWN???
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???UNKNOWN???
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, &ai_null, &ai_null }, // Puppy (Tail Wag)
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, &ai_null, &ai_null }, // Puppy (Sleeping)
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppy, &ai_null, &ai_null }, // Puppy (Barking)
	{ &SPR_Jenka, NOSHEET, 	PAL3, 1, &ai_null, &ai_jenka, &ai_null }, // Jenka
	{ &SPR_Armadl, NOSHEET, PAL3, 1, &ai_null, &ai_armadillo, &ondeath_default }, // Armadillo
	{ &SPR_Skullhead,NOSHEET,PAL3,1, &ai_null, &ai_skullhead, &ondeath_default }, // Skeleton
	{ &SPR_Puppy, NOSHEET, 	PAL1, 1, &onspawn_puppyCarry, &ai_puppyCarry, &ai_null }, // Puppy (Carried)
	{ &SPR_BigDoorFrame,NOSHEET,PAL1,2, &ai_null, &ai_null, &ai_null }, // Large Door (Frame)
	{ &SPR_BigDoor,NOSHEET, PAL1, 1, &onspawn_doorway, &ai_doorway, &ai_null }, // Large Door
	{ &SPR_Doctor, NOSHEET, PAL3, 1, &onspawn_snap, &ai_doctor, &ai_null }, // Doctor
	{ &SPR_ToroBoss,NOSHEET,PAL3, 2, &onspawn_torokoBoss, &ai_torokoBoss, &ondeath_torokoBoss },
	{ &SPR_ToroBlock,NOSHEET,PAL1,1, &onspawn_persistent, &ai_torokoBlock, &ai_null },
	{ &SPR_ToroFlower,NOSHEET,PAL3,1,&onspawn_persistent, &ai_torokoFlower, &ondeath_default },
	{ &SPR_Jenka, NOSHEET, 	PAL3, 1, &onspawn_jenka, &ai_null, &ai_null }, // Jenka
	/* 0x090 (144) */
	{ &SPR_Toroko, NOSHEET, PAL3, 1, &ai_null, &ai_toroko_teleport_in, &ai_null },
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, 		PAL0, 4, &ai_null, &ai_null, &ai_null }, // Lightning
	{ NULL, NOSHEET, 		PAL2, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Purple Critter
	{ &SPR_LabShot,NOSHEET, PAL2, 1, &onspawn_persistent, &ai_null, &ai_null }, // Critter Shot
	{ &SPR_MazeBlock,NOSHEET,PAL2,1, &onspawn_block, &ai_blockh, &ai_null }, // Moving Block (H)
	{ &SPR_Quote, NOSHEET, 	PAL0, 1, &onspawn_persistent, &ai_player, &ai_null }, // Quote
	{ &SPR_Robot, NOSHEET, 	PAL3, 1, &ai_null, &ai_blue_robot, &ai_null }, // Blue Robot
	{ NULL, NOSHEET, 		PAL2, 1, &ai_null, &ai_shutter_stuck, &ondeath_default }, // Shutter (Stuck)
	{ NULL, SHEET_GAUDI, 	PAL2, 1, &ai_null, &ai_gaudi, &ondeath_default }, // Gaudi
	{ NULL, SHEET_GAUDID, 	PAL2, 1, &ai_null, &ai_gaudiDying, &ondeath_default }, // Gaudi (Dying)
	{ NULL, SHEET_GAUDI, 	PAL2, 1, &ai_null, &ai_gaudiFlying, &ondeath_default }, // Gaudi (Flying)
	{ &SPR_LabShot,NOSHEET, PAL2, 1, &onspawn_persistent, &ai_gaudiArmoredShot, &ai_null },
	{ &SPR_MazeBlock,NOSHEET,PAL2,1, &onspawn_block, &ai_blockv, &ai_null }, // Moving Block (V)
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_null, &ai_null }, // Monster X Shot
	{ NULL, NOSHEET, 		PAL0, 4, &ai_null, &ai_null, &ai_null }, // Monster X Cat
	/* 0x0A0 (160) */
	{ NULL, SHEET_DARK, 	PAL3, 2, &ai_null, &ai_pooh_black, &ondeath_default }, // Pooh Black
	{ NULL, SHEET_DARKBUB, 	PAL3, 1, &ai_null, &ai_pooh_black_bubble, &ai_null }, // Pooh Black Bubble
	{ NULL, SHEET_DARK, 	PAL3, 2, &ai_null, &ai_pooh_black_dying, &ai_null }, // Pooh Black (Dying)
	{ &SPR_DrGero, NOSHEET, PAL3, 1, &ai_null, &generic_npc_states, &ai_null }, // Dr. Gero
	{ &SPR_Nurse, NOSHEET, 	PAL3, 1, &ai_null, &generic_npc_states, &ai_null }, // Nurse Hasumi
	{ &SPR_Curly, NOSHEET, 	PAL3, 1, &ai_null, &ai_curly, &ai_null }, // Curly (Collapsed)
	{ &SPR_GaudiMerch,NOSHEET,PAL3,1,&ai_null, &ai_null, &ai_null }, // Chaba
	{ &SPR_Booster,NOSHEET, PAL3, 1, &onspawn_persistent, &ai_booster_falling, &ai_null }, // Booster
	{ &SPR_Boulder,NOSHEET, PAL3, 4, &ai_null, &ai_boulder, &ai_null }, // Boulder
	{ &SPR_Balrog, NOSHEET, PAL1, 2, &onspawn_persistent, &ai_balrog_boss_missiles, &ondeath_balrogFlying },
	{ NULL, NOSHEET, 		PAL0, 1, &ai_null, &ai_balrog_missile, &ai_null }, // Balrog Missile
	{ &SPR_CoreThing,NOSHEET,PAL2,1, &ai_null, &ai_firewhirr, &ondeath_default }, // Fire Whirr
	{ &SPR_CoreThingShot,NOSHEET,PAL2,1, &onspawn_persistent, &ai_firewhirr_shot, &ai_null },
	{ &SPR_GaudiArmor,NOSHEET,PAL2,1,&ai_null, &ai_gaudiArmored, &ai_null },
	{ NULL, NOSHEET, 		PAL2, 1, &onspawn_persistent, &ai_null, &ai_null }, // Gaudi Shot
	{ &SPR_GaudiEgg,NOSHEET,PAL2, 1, &ai_null, &ai_gaudi_egg, &ai_null }, // Gaudi Egg
	/* 0x0B0 (176) */
	{ &SPR_BuyoBase,NOSHEET, PAL2, 1, &ai_null, &ai_buyobuyo_base, &ai_null }, // Buyobuyo Base
	{ &SPR_Buyo, 	NOSHEET, PAL2, 1, &ai_null, &ai_buyobuyo, &ondeath_default }, // Buyobuyo
	{ &SPR_CoreShot1,NOSHEET,PAL3, 1, &onspawn_persistent, &ai_minicore_shot, &ai_null }, // Core Spinning Projectile
	{ &SPR_CoreShot2,NOSHEET,PAL3, 1, &onspawn_persistent, &ai_core_ghostie, &ai_null }, // Core Wisp Projectile
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, &onspawn_persistent, &ai_curly_ai, &ai_null }, // Curly (AI)
	{ &SPR_Polar, 	NOSHEET, PAL0, 1, &onspawn_persistent, &ai_cai_gun, &ai_null }, // Curly Polar Star
	{ &SPR_MGun, 	NOSHEET, PAL1, 1, &onspawn_persistent, &ai_cai_gun, &ai_null }, // Curly Machine Gun
	{ &SPR_Bubble, 	NOSHEET, PAL0, 1, &onspawn_persistent, &ai_cai_watershield, &ai_null }, // Curly Bubble Shield
	{ NULL, 		NOSHEET, PAL2, 1, &ai_null, &ai_shutter, &ai_null }, // Shutter (Large)
	{ NULL, 		NOSHEET, PAL2, 1, &ai_null, &ai_shutter, &ai_null }, // Shutter (Small)
	{ &SPR_Baby, 	NOSHEET, PAL2, 1, &ai_null, &ai_shutter, &ai_null }, // Lift Block
	{ &SPR_FuzzCore,NOSHEET, PAL2, 1, &ai_null, &ai_fuzz_core, &ondeath_default }, // Fuzz Core
	{ &SPR_Fuzz, 	NOSHEET, PAL2, 1, &ai_null, &ai_fuzz, &ondeath_default }, // Fuzz
	{ &SPR_CoreShot4,NOSHEET,PAL3, 1, &onspawn_persistent, &ai_core_blast, &ai_null }, // Homing Flame Projectile
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_almond_robot, &ai_null }, // Surface Robot
	{ NULL, 		NOSHEET, PAL0, 2, &onspawn_persistent, &ai_waterlevel, &ai_null }, // Water Level
	/* 0x0C0 (192) */
	{ &SPR_Buggy, 	NOSHEET, PAL1, 1, &ai_null, &ai_motorbike, &ai_null }, // Scooter
	{ &SPR_Buggy2, 	NOSHEET, PAL1, 2, &ai_null, &ai_null, &ai_null }, // Scooter (Pieces)
	{ &SPR_Robot, 	NOSHEET, PAL3, 1, &ai_null, &ai_null, &ai_null }, // Blue Robot (Pieces)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Grate Mouth
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Motion Wall
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Porcupine Fish
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Ironhead Projectile
	{ NULL, 		NOSHEET, PAL0, 0, &ai_null, &ai_null, &ai_null }, // Underwater Current
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Dragon Zombie
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Dragon Zombie (Dead)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Dragon Zombie Projectile
	{ NULL, 		NOSHEET, PAL0, 1, &onspawn_snap, &ai_critter, &ondeath_default }, // Blue Hopping Critter
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Falling Spike (Small)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Falling Spike (Large)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Counter Bomb
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Countdown Balloon
	// 0x0D0 (208) */
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Basu (2)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Basu Projectile (2)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Green Beetle (Follow 2)
	{ &SPR_Spikes, 	NOSHEET, PAL1, 1, &onspawn_spike, &ai_null, &ai_null }, // Spikes
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Sky Dragon
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Night Spirit
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Night Spirit Projectile
	{ NULL, 		NOSHEET, PAL0, 2, &ai_null, &ai_null, &ondeath_default }, // White Sandcroc
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Debug Cat
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Itoh
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???
	{ NULL, 		NOSHEET, PAL0, 0, &ai_null, &ai_null, &ai_null }, // Smoke/Current Generator
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Shovel Brigade
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Shovel Brigade (Walking)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Prison Bars
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Momorin
	/* 0x0E0 (224) */
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Chie
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Megane
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Kanpachi
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Bucket
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Droll (Guard)
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Red Flower Sprouts
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Blooming Red Flowers
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Rocket
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Orangebell
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // ???
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Picked Red Flowers
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Midorin
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Gunfish
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Gunfish Projectile
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ondeath_default }, // Lethal Press
	{ NULL, 		NOSHEET, PAL0, 1, &ai_null, &ai_null, &ai_null }, // Cage Bars
	/* 0x0F0 (240) */
	{ NULL, 		NOSHEET, PAL0, 1, // Jailed Mimiga
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Critter
		&onspawn_snap, &ai_critter, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bat
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bat Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Acid Drop
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Acid Drop Generator
		&ai_null, &ai_null, &ondeath_default },
	{ &SPR_Press, 	NOSHEET, PAL1, 1, // Press (Proximity)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Boss)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Vanish)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery Energy Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery Lightning Ball
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Misery Lightning
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Energy Capsule
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Helicopter
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	/* 0x100 (256) */
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Before Fight)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Crystal
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sleeping Mimiga
		&ai_null, &ai_null, &ondeath_default },
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, // Curly (Carried)
		&onspawn_persistent, &ai_curly_carried, &ai_null },
	{ NULL, 		NOSHEET, PAL0, 1, // Shovel Brigade (Caged)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Chie (Caged)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Chaco (Caged)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Boss)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Wave
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Ball (Fast)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor Red Ball (Slow)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Muscle Doctor (Boss)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Igor
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Energy Bat
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Energy
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Underwater Block
		&ai_null, &ai_null, &ondeath_default },
	/* 0x110 (272) */
	{ NULL, 		NOSHEET, PAL0, 0, // Water Block Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Droll Projectile
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Droll
		&ai_null, &ai_null, &ondeath_default },
	{ &SPR_Puppy, 	NOSHEET, PAL1, 1, // Puppy (With Item)
		&onspawn_puppy, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Demon
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Demon Projectile
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Little Family
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling Block (Large)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sue (Teleport In)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Red Energy)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Mini Undead Core
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Transformed)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sue (Transformed)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Spinning Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Dot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Smoke
		&ai_null, &ai_null, &ondeath_default },
	/* 0x120 (288) */
	{ NULL, 		NOSHEET, PAL0, 1, // Glowing Rock Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Critter
		&onspawn_snap, &ai_critter, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Orange Bat
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Mini Core (Before Fight)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Quake
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Huge Energy Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Falling Block Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Cloud
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Cloud Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Doctor (Uncrowned)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Balrog/Misery (Bubble)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Demon Crown
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Fish Missile (Orange)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	/* 0x130 (304) */
	{ NULL, 		NOSHEET, PAL0, 1, // Gaudi (Sitting)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Puppy (Small)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Balrog (Nurse)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Santa (Caged)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Stumpy
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Sword)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Archer)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute Projectile
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ma Pignon
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Falling (?)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Hopping Enemy (?)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Defeated)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Mesa
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Mesa (Defeated)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	/* 0x140 (320) */
	{ &SPR_Curly, 	NOSHEET, PAL3, 1, // Curly (Hell)
		&ai_null, &ai_null, &ai_null },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Deleet
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Bute (Generated)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Bute Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Heavy Press Projectile
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Itoh/Sue (Ending)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Transmogrifier
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Building Fan
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Rolling
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Bone Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Shockwave
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 4, // Ballos Lightning
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Sweat
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ika-chan
		&ai_null, &ai_null, &ondeath_default },
	/* 0x150 (336) */
	{ NULL, 		NOSHEET, PAL0, 0, // Ika-chan Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Numahachi
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Green Devil
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Green Devil Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos (Boss)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Skull Shot
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Hoppy
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Ballos Spikes
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Statue
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bute (Archer)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Statue (Breakable)
		&ai_null, &ai_null, &ondeath_default },
	/* 0x160 (352) */
	{ NULL, 		NOSHEET, PAL0, 1, // King (Sword)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Red Bute (Sword)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Kill Zone
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // ???
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Balrog (Rescue)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Puppy (Ghost)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 1, // Misery (Wind)
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 0, // Water Drop Generator
		&ai_null, &ai_null, &ondeath_default },
	{ NULL, 		NOSHEET, PAL0, 2, // Thank You
		&ai_null, &ai_null, &ondeath_default },
	/* ^ 0x168 (360) ^ */
	/* MAJOR BOSSES - Don't exist in npc.tbl, but need AI methods */
	/* 0x169 (361) */
	{ &SPR_Omega, 	NOSHEET, PAL3, 6, // Omega
		&onspawn_omega, &ai_omega, &ondeath_omega },
	{ &SPR_Balfrog1, NOSHEET, PAL3, 6, // Balfrog
		&onspawn_balfrog, &ai_balfrog, &ondeath_balfrog },
	{ NULL, NOSHEET, PAL3, 8, // Monster X
		&ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 1, // Core (Controller)
		&onspawn_core, &ai_core, &ondeath_core },
	{ NULL, NOSHEET, PAL3, 2, // Ironhead
		&ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 3, // Sisters
		&ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 3, // Undead Core
		&ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 3, // Heavy Press
		&ai_null, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 3, // Ballos
		&ai_null, &ai_null, &ai_null },
	/* ^ 0x171 (369) ^ */
	/* BOSS PARTS - Separate entities belonging to the bosses which don't
	 * already exist in the NPC table, for whatever reason */
	/* 0x172 (370) */
	{ NULL, NOSHEET, PAL3, 6, // Core (Front)
		&onspawn_persistent, &ai_core_front, &ai_null },
	{ NULL, NOSHEET, PAL3, 9, // Core (Back)
		&onspawn_persistent, &ai_core_back, &ai_null },
	{ &SPR_MiniCore, NOSHEET, PAL3, 2, // Mini Core
		&onspawn_persistent, &ai_minicore, &ai_null },
	{ &SPR_OmegaLeg, NOSHEET, PAL3, 1, // Omega Leg
		&onspawn_persistent, &ai_null, &ai_null },
	{ &SPR_OmgStrut, NOSHEET, PAL3, 1, // Omega Strut
		&onspawn_persistent, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 2, // Monster X Tread
		&onspawn_x_tread, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 4, // Monster X Door
		&onspawn_x_door, &ai_null, &ai_null },
	{ NULL, NOSHEET, PAL3, 1, // Monster X Target
		&onspawn_x_target, &ai_null, &ai_null },
};
