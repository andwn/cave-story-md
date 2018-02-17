#include "common.h"

#include "ai_gen.h"
#include "entity.h"
#include "npc.h"
#include "resources.h"
#include "sheet.h"
#include "vdp.h"

#include "tables.h"

const npc_info_def npc_info[NPC_COUNT + 9 + 18 + 9] = {
	{ NULL,				NOSHEET, 		PAL0, 0, &onspawn_op2snap, 		&ai_nothing, 		&ai_null 			}, // OBJ_NULL
	{ NULL,				SHEET_ENERGY, 	PAL1, 1, &onspawn_energy, 		&ai_energy, 		&ai_null 			}, // OBJ_XP
	{ NULL,				SHEET_BEHEM, 	PAL1, 1, &ai_null, 				&ai_behemoth, 		&ondeath_default 	}, // OBJ_BEHEMOTH
	{ NULL,				NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			},
	{ NULL,				NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // OBJ_SMOKE_CLOUD
	{ NULL,				SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // OBJ_CRITTER_HOPPING_GREEN
	{ NULL,				SHEET_BEETLE, 	PAL3, 1, &onspawn_op2flip, 		&ai_beetle, 		&ondeath_default 	}, // Beetle
	{ NULL,				SHEET_BASIL, 	PAL1, 1, &onspawn_basil, 		&ai_basil, 			&ai_null 			}, // Basil
	{ NULL,				SHEET_BEETLE, 	PAL3, 1, &onspawn_beetleFollow, &ai_beetleFollow, 	&ondeath_default 	},
	{ &SPR_Balrog,		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrog_drop_in, &ai_null 			}, // Balrog
	{ NULL,				NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			},
	{ NULL,				SHEET_IGORSHOT, PAL1, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Igor Shot
	{ &SPR_Balrog,		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrog, 		&ai_null 			}, // Balrog
	{ NULL,				SHEET_FFIELD, 	PAL1, 1, &ai_null, 				&ai_forcefield, 	&ai_null 			}, // Forcefield
	{ &SPR_Key,			NOSHEET, 		PAL1, 1, &ai_null, 				&ai_lifeup, 		&ai_null 			}, // Santa's Key
	{ &SPR_Chest,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_chest, 			&ai_null 			}, // Chest
	/* 0x010 (16) */
	{ &SPR_Save,		NOSHEET, 		PAL1, 1, &onspawn_interactive, 	&ai_savepoint,		&ai_null 			}, // Save Point
	{ &SPR_Refill,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_refill, 		&ai_null 			}, // Refill
	{ &SPR_Door,		NOSHEET, 		PAL1, 1, &onspawn_door, 		&ai_door, 			&ai_null 			}, // Door
	{ &SPR_Balrog,		NOSHEET, 		PAL1, 2, &onspawn_persistent,	&ai_balrog_bust_in, &ai_null 			}, // Balrog
	{ &SPR_Computer,	NOSHEET,		PAL1, 1, &ai_null, 				&ai_computer, 		&ai_null 			}, // Computer
	{ &SPR_Chest2,		NOSHEET, 		PAL1, 1, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Open Chest
	{ &SPR_Tele,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Teleporter
	{ &SPR_TeleLight,	NOSHEET,		PAL1, 1, &onspawn_teleLight, 	&ai_teleLight, 		&ai_null 			}, // Tele Light
	{ NULL,				SHEET_PCRITTER, PAL0, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Power Critter
	{ &SPR_Platform,	NOSHEET,		PAL1, 1, &onspawn_lift, 		&ai_lift, 			&ai_null 			}, // Lift Platform
	{ NULL,				SHEET_BAT, 		PAL3, 1, &ai_null, 				&ai_batCircle, 		&ondeath_default 	}, // Bat
	{ NULL,				SHEET_TRAP, 	PAL1, 1, &onspawn_trap, 		&ai_null, 			&ai_null 			}, // Death Trap
	{ NULL,				SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Critter
	{ &SPR_Cthu,		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Cthulhu
	{ &SPR_Gunsmith,	NOSHEET,		PAL3, 1, &onspawn_op2snap, 		&ai_gunsmith,		&ai_null 			}, // Gunsmith
	{ NULL,				SHEET_BAT, 		PAL3, 1, &onspawn_batHang, 		&ai_batHang, 		&ondeath_default 	}, // Bat
	/* 0x020 (32) */
	{ &SPR_LifeUp,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_lifeup, 		&ai_null 			}, // Life Capsule
	{ NULL,				SHEET_IGORSHOT, PAL1, 1, &ai_null, 				&ai_balrogShot, 	&ondeath_nodrop 	}, // Balrog Shot
	{ &SPR_Bed,			NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Bed
	{ NULL,				SHEET_MANNAN, 	PAL3, 1, &onspawn_op2flip, 		&ai_mannan, 		&ai_null 			}, // Mannan
	{ &SPR_Balrog,		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrogFlying, 	&ondeath_balrogFlying },
	{ &SPR_Sign,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Sign Post
	{ &SPR_Fire,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_fireplace, 		&ai_null 			}, // Fireplace
	{ &SPR_SaveSign,	NOSHEET,		PAL1, 1, &onspawn_op2anim, 		&ai_null, 			&ai_null 			}, // Save Sign
	{ &SPR_Santa,		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_santa, 			&ai_null 			}, // Santa
	{ NULL,				NOSHEET, 		PAL0, 0, &onspawn_door, 		&ai_null, 			&ai_null 			}, // Busted Doorway
	{ &SPR_Sue,			NOSHEET, 		PAL0, 1, &onspawn_sue, 			&ai_sue, 			&ai_null 			}, // Sue
	{ &SPR_Board,		NOSHEET, 		PAL1, 2, &onspawn_blackboard, 	&ai_null, 			&ai_null 			}, // Blackboard
	{ NULL,				SHEET_POLISH, 	PAL3, 1, &ai_null, 				&ai_polish, 		&ondeath_default 	}, // Polish
	{ NULL,				SHEET_BABY, 	PAL3, 1, &ai_null, 				&ai_baby, 			&ondeath_default 	}, // Baby
	{ NULL,				NOSHEET, 		PAL0, 0, &onspawn_trigger, 		&ai_trigger, 		&ai_null 			}, // Trigger
	{ &SPR_Sandcroc,	NOSHEET,		PAL3, 2, &ai_null, 				&ai_sandcroc, 		&ondeath_default 	}, // Sandcroc
	/* 0x030 (48) */
	{ NULL,				SHEET_OMGSHOT,  PAL3, 1, &onspawn_persistent, 	&ai_omega_shot, 	&ondeath_default 	}, // Omega Shot
	{ NULL,				SHEET_SKULLH,	PAL1, 1, &ai_null, 				&ai_skullhead, 		&ondeath_default 	}, // Skullhead
	{ &SPR_Bone,		NOSHEET,		PAL1, 1, &onspawn_persistent, 	&ai_skeleton_shot, 	&ondeath_nodrop 	}, // Skeleton
	{ NULL,				SHEET_CROW, 	PAL3, 1, &ai_null, 				&ai_crow, 			&ondeath_default 	}, // Crow & Skullhead
	{ &SPR_Robot,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Blue Robot
	{ NULL,				NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???UNKNOWN???
	{ NULL,				SHEET_SKULLH,	PAL1, 1, &ai_null, 				&ai_skullhead, 		&ondeath_default 	}, // Skullstep
	{ &SPR_Kazuma,		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_kazuma, 		&ai_null 			}, // Kazuma
	{ NULL,				SHEET_BEETLE, 	PAL3, 1, &ai_null, 				&ai_beetle, 		&ondeath_default 	}, // Beetle
	{ NULL,				SHEET_CROW, 	PAL3, 1, &ai_null, 				&ai_crow, 			&ondeath_default 	}, // Crow
	{ &SPR_Basu,		NOSHEET, 		PAL3, 1, &onspawn_basu, 		&ai_basu, 			&ondeath_default 	}, // Basu
	{ &SPR_DoorE,		NOSHEET,  		PAL1, 1, &ai_null, 				&ai_theDoor, 		&ondeath_default 	}, // The Door
	{ &SPR_Toroko,		NOSHEET, 		PAL3, 1, &onspawn_op2flip, 		&ai_toroko, 		&ai_null 			}, // Toroko
	{ &SPR_King,		NOSHEET, 		PAL3, 1, &onspawn_op2flip, 		&ai_king, 			&ai_null 			}, // King
	{ &SPR_KazuCom,		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_npc_at_computer,&ai_null 			}, // Kazuma Typing
	{ &SPR_Toroko,		NOSHEET, 		PAL3, 1, &onspawn_torokoAtk, 	&ai_torokoAtk, 		&ai_null 			}, // Toroko
	/* 0x040 (64) */
	{ NULL,				SHEET_CRITTER, 	PAL2, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Critter
	{ NULL,				SHEET_BAT, 		PAL0, 1, &ai_null, 				&ai_batVertical, 	&ondeath_default 	}, // Bat
	{ &SPR_Bubble,		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_misery_bubble, 	&ai_null 			}, // Misery's Bubble
	{ &SPR_Misery,		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_misery_float, 	&ai_null 			}, // Misery
	{ &SPR_Balrog,		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrogRunning, 	&ondeath_balrogRunning },
	{ NULL,				SHEET_PIGNON, 	PAL1, 1, &ai_null, 				&ai_pignon, 		&ondeath_pignon 	}, // Pignon
	{ &SPR_Sparkle,		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_sparkle, 		&ai_null 			}, // Sparkle
	{ &SPR_Fish,		NOSHEET, 		PAL0, 1, &ai_null, 				&ai_chinfish, 		&ondeath_default 	}, // Chinfish
	{ &SPR_Sprinkler,	NOSHEET,		PAL1, 1, &ai_null, 				&ai_sprinkler, 		&ai_null 			}, // Sprinkler
	{ NULL,				SHEET_DROP, 	PAL1, 1, &onspawn_persistent, 	&ai_water_droplet, 	&ai_null 			}, // Water Drop
	{ &SPR_Jack,		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_jack, 			&ai_null 			}, // Jack
	{ &SPR_Kanpachi,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_kanpachi_fish,	&ai_null 			}, // Kanpachi
	{ NULL,				SHEET_FLOWER, 	PAL3, 1, &onspawn_flower, 		&ai_null, 			&ai_null 			}, // Flowers
	{ &SPR_Sanda,		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_sanda, 			&ai_null 			}, // Sandame
	{ &SPR_Pot,			NOSHEET, 		PAL1, 1, &onspawn_op2anim, 		&ai_null, 			&ai_null 			}, // Pot
	{ &SPR_Mahin,		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_mahin, 			&ai_null 			}, // Mahin
	/* 0x050 (80) */
	{ &SPR_Keeper,		NOSHEET, 		PAL1, 1, &onspawn_gkeeper, 		&ai_gkeeper, 		&ondeath_default 	}, // Gravekeeper
	{ NULL,				SHEET_PIGNONB, 	PAL1, 1, &ai_null, 				&ai_pignon, 		&ondeath_default 	}, // Big Pignon
	{ &SPR_Misery,		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_misery_stand, 	&ai_null 			}, // Misery
	{ &SPR_Igor,		NOSHEET, 		PAL3, 4, &onspawn_igor, 		&ai_igorscene, 		&ai_null 			}, // Igor
	{ NULL,				SHEET_BASUSHOT, PAL0, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Basu Shot
	{ NULL,				SHEET_TERM, 	PAL1, 1, &ai_null, 				&ai_terminal, 		&ai_null 			}, // Terminal
	{ NULL,				SHEET_MISSILE, 	PAL1, 1, &onspawn_op2anim, 		&ai_missile, 		&ai_null 			}, // Missile
	{ NULL,				SHEET_HEART, 	PAL1, 1, &onspawn_op2anim, 		&ai_heart, 			&ai_null 			}, // Heart
	{ &SPR_Igor,		NOSHEET, 		PAL3, 4, &onspawn_igor, 		&ai_igor, 			&ondeath_igor 		}, // Igor
	{ &SPR_Igor,		NOSHEET, 		PAL3, 4, &onspawn_igor, 		&ai_igordead, 		&ai_null 			}, // Igor
	{ NULL,				NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			},
	{ &SPR_Cage, 		NOSHEET, 		PAL1, 1, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Cage
	{ &SPR_SueCom, 		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_npc_at_computer,&ai_null 			}, // Sue Typing
	{ &SPR_Chaco, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_chaco, 			&ai_null 			}, // Chaco
	{ &SPR_Kulala, 		NOSHEET, 		PAL0, 2, &ai_null, 				&ai_kulala, 		&ondeath_kulala 	}, // Kulala
	{ NULL,				SHEET_JELLY, 	PAL0, 1, &onspawn_jelly, 		&ai_jelly, 			&ondeath_default 	}, // Jelly
	/* 0x060 (96) */
	{ NULL, 			SHEET_FAN, 		PAL1, 1, &onspawn_fan, 			&ai_fan, 			&ai_null 			}, // Fans (L,U,R,D)
	{ NULL, 			SHEET_FAN, 		PAL1, 1, &onspawn_fan, 			&ai_fan, 			&ai_null 			},
	{ NULL, 			SHEET_FAN, 		PAL1, 1, &onspawn_fan, 			&ai_fan, 			&ai_null 			},
	{ NULL, 			SHEET_FAN, 		PAL1, 1, &onspawn_fan, 			&ai_fan, 			&ai_null 			},
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Grate
	{ NULL, 			SHEET_POWERS, 	PAL1, 1, &ai_null, 				&ai_powers, 		&ai_null 			}, // Power Screen
	{ NULL, 			SHEET_POWERF, 	PAL1, 1, &onspawn_powerf, 		&ai_powerf, 		&ai_null 			}, // Power Flow
	{ &SPR_ManShot,		NOSHEET, 		PAL1, 1, &onspawn_persistent, 	&ai_mannanShot, 	&ai_null 			}, // Mannan Shot
	{ NULL, 			SHEET_FROG, 	PAL3, 1, &onspawn_frog, 		&ai_frog, 			&ondeath_default 	}, // Frog
	{ &SPR_Hey, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_hey, 			&ai_null 			}, // "Hey!"
	{ &SPR_Hey, 		NOSHEET, 		PAL1, 1, &onspawn_hey, 			&ai_hey, 			&ai_null 			}, // "Hey!" Emitter
	{ &SPR_Malco, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_malco, 			&ai_null 			}, // Malco
	{ NULL, 			SHEET_REDSHOT, 	PAL1, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Balfrog Shot
	{ &SPR_Malco, 		NOSHEET, 		PAL1, 1, &onspawn_malcoBroken, 	&ai_null, 			&ai_null 			}, // Malco
	{ NULL, 			SHEET_PUCHI, 	PAL3, 1, &onspawn_frog, 		&ai_frog, 			&ondeath_default 	}, // Puchi
	{ &SPR_Quotele,		NOSHEET, 		PAL0, 1, &onspawn_teleOut, 		&ai_teleOut, 		&ai_null 			}, // Tele Out
	/* 0x070 (112) */
	{ &SPR_Quotele,		NOSHEET, 		PAL0, 1, &onspawn_teleIn, 		&ai_teleIn, 		&ai_null 			}, // Tele In
	{ &SPR_Booster,		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_booster, 		&ai_null 			}, // Prof. Booster
	{ &SPR_Press, 		NOSHEET, 		PAL1, 1, &onspawn_press, 		&ai_press, 			&ondeath_default 	}, // Press
	{ &SPR_Ravil,		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_ravil, 			&ondeath_ravil 		}, // Ravil
	{ &SPR_RedFl2, 		NOSHEET, 		PAL1, 1, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Red Flower Petals
	{ &SPR_Curly, 		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_curly, 			&ai_null 			}, // Curly
	{ NULL, 			SHEET_CURLYB,	PAL3, 1, &onspawn_snap, 		&ai_curlyBoss, 		&ondeath_curlyBoss 	}, // Curly
	{ &SPR_Table, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Table & Chair
	{ NULL, 			SHEET_BARMIMI, 	PAL3, 1, &ai_null, 				&ai_curlys_mimigas, &ondeath_default 	},
	{ NULL, 			SHEET_BARMIMI, 	PAL3, 1, &ai_null, 				&ai_curlys_mimigas, &ondeath_default 	},
	{ NULL, 			SHEET_BARMIMI, 	PAL3, 1, &ai_null, 				&ai_curlys_mimigas, &ondeath_default 	},
	{ NULL, 			SHEET_MGUN, 	PAL0, 1, &onspawn_persistent, 	&ai_curlyBossShot, 	&ai_null 			}, // Curly Shot
	{ &SPR_Sunstone,	NOSHEET,		PAL2, 1, &onspawn_sunstone, 	&ai_sunstone, 		&ai_null 			}, // Sunstone
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_hiddenPowerup,&ai_hiddenPowerup, 	&ai_null 			},
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppy, 		&ai_puppy_run, 		&ai_null 			}, // Puppy (Running)
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	/* 0x080 (128) */
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppy, 		&ai_puppy_wag, 		&ai_null 			}, // Puppy (Tail Wag)
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppy, 		&ai_null, 			&ai_null 			}, // Puppy (Sleeping)
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppy, 		&ai_puppy_bark, 	&ai_null 			}, // Puppy (Barking)
	{ &SPR_Jenka, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_jenka, 			&ai_null 			}, // Jenka
	{ &SPR_Armadl, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_armadillo, 		&ondeath_default 	}, // Armadillo
	{ &SPR_Skeleton,	NOSHEET,		PAL1, 1, &ai_null, 				&ai_skeleton, 		&ondeath_default 	}, // Skeleton
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppyCarry, 	&ai_puppyCarry, 	&ai_null 			}, // Puppy (Carried)
	{ &SPR_BigDoorFrame,NOSHEET,		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Large Door (Frame)
	{ &SPR_BigDoor,		NOSHEET, 		PAL1, 1, &onspawn_doorway, 		&ai_doorway, 		&ai_null 			}, // Large Door
	{ &SPR_Doctor, 		NOSHEET, 		PAL3, 1, &onspawn_snap, 		&ai_doctor, 		&ai_null 			}, // Doctor
	{ &SPR_ToroBoss,	NOSHEET,		PAL3, 2, &onspawn_torokoBoss, 	&ai_torokoBoss, 	&ondeath_torokoBoss },
	{ NULL,				SHEET_BLOCK,	PAL1, 1, &onspawn_persistent, 	&ai_torokoBlock, 	&ai_null 			},
	{ NULL, 			SHEET_FLOWER,	PAL3, 1, &onspawn_persistent, 	&ai_torokoFlower, 	&ondeath_default 	},
	{ &SPR_Jenka, 		NOSHEET, 		PAL3, 1, &onspawn_jenka, 		&ai_null, 			&ai_null 			}, // Jenka
	/* 0x090 (144) */
	{ &SPR_Toroko, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_toroko_tele_in, &ai_null 			},
	{ &SPR_King, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Sword?
	{ &SPR_Lightning,	NOSHEET,		PAL1, 6, &onspawn_lightning, 	&ai_lightning, 		&ai_null 			}, // Lightning
	{ NULL, 			SHEET_CRITTER, 	PAL2, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Purple Critter
	{ &SPR_LabShot,		NOSHEET, 		PAL2, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Critter Shot
	{ &SPR_MazeBlock,	NOSHEET,		PAL2, 1, &onspawn_block, 		&ai_blockh, 		&ai_null 			}, // Moving Block (H)
	{ &SPR_Quote, 		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_player, 		&ai_null 			}, // Quote
	{ &SPR_Robot, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_blue_robot, 	&ai_null 			}, // Blue Robot
	{ NULL, 			NOSHEET, 		PAL2, 0, &ai_null, 				&ai_shutter_stuck, 	&ondeath_nodrop 	}, // Shutter (Stuck)
	{ NULL, 			SHEET_GAUDI, 	PAL2, 1, &ai_null, 				&ai_gaudi, 			&ondeath_default 	}, // Gaudi
	{ NULL, 			SHEET_GAUDI, 	PAL2, 1, &ai_null, 				&ai_gaudiDying, 	&ondeath_default 	}, // Gaudi (Dying)
	{ NULL, 			SHEET_GAUDI, 	PAL2, 1, &ai_null, 				&ai_gaudiFlying, 	&ondeath_default 	}, // Gaudi (Flying)
	{ &SPR_LabShot,		NOSHEET, 		PAL2, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_default 	}, // Gaudi Shot
	{ &SPR_MazeBlock,	NOSHEET,		PAL2, 1, &onspawn_block, 		&ai_blockv, 		&ai_null 			}, // Moving Block (V)
	{ NULL, 			SHEET_XFISHY, 	PAL3, 1, &onspawn_persistent, 	&ai_x_fishy_missile,&ondeath_default 	}, // Monster X Shot
	{ &SPR_XCat, 		NOSHEET, 		PAL3, 6, &onspawn_persistent, 	&ai_x_defeated, 	&ai_null 			}, // Monster X Cat
	/* 0x0A0 (160) */
	{ &SPR_Dark, 		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_pooh_black, 	&ondeath_pooh_black }, // Pooh Black
	{ NULL, 			SHEET_DARKBUB, 	PAL3, 1, &ai_null, 				&ai_poohblk_bubble, &ondeath_nodrop 	}, // Pooh Black Bubble
	{ &SPR_Dark, 		NOSHEET,		PAL3, 2, &ai_null, 				&ai_poohblk_dying, 	&ai_null 			}, // Pooh Black (Dying)
	{ &SPR_DrGero, 		NOSHEET, 		PAL3, 1, &onspawn_gero, 		&ai_gero, 			&ai_null 			}, // Dr. Gero
	{ &SPR_Nurse, 		NOSHEET, 		PAL3, 1, &onspawn_gero, 		&ai_gero, 			&ai_null 			}, // Nurse Hasumi
	{ &SPR_Curly, 		NOSHEET, 		PAL3, 1, &onspawn_curly_down, 	&ai_curly, 			&ai_null 			}, // Curly (Collapsed)
	{ &SPR_GaudiMerch,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Chaba
	{ &SPR_Booster,		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_booster_falling,&ai_null 			}, // Booster
	{ &SPR_Boulder,		NOSHEET, 		PAL3, 4, &ai_null, 				&ai_boulder, 		&ai_null 			}, // Boulder
	{ &SPR_Balrog, 		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrog_boss_msl,&ondeath_balrogFlying },
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_balrog_missile, &ai_null 			}, // Balrog Missile
	{ &SPR_FireWhir,	NOSHEET,		PAL2, 1, &ai_null, 				&ai_firewhirr, 		&ondeath_default 	}, // Fire Whirr
	{ NULL,				SHEET_FIREWSHOT,PAL2, 1, &onspawn_persistent, 	&ai_firewhirr_shot, &ai_null 			},
	{ NULL, 			SHEET_GAUDI,	PAL2, 1, &ai_null, 				&ai_gaudiArmor, 	&ai_null 			},
	{ NULL, 			SHEET_GAUDISHOT,PAL2, 1, &onspawn_persistent, 	&ai_gaudiArmorShot, &ondeath_nodrop 	}, // Gaudi Shot
	{ NULL,				SHEET_GAUDIEGG,	PAL2, 1, &ai_null, 				&ai_gaudi_egg, 		&ai_null		 	}, // Gaudi Egg
	/* 0x0B0 (176) */
	{ NULL,				SHEET_BUYOB, 	PAL2, 1, &ai_null, 				&ai_buyobuyo_base, 	&ai_null 			}, // Buyobuyo Base
	{ NULL, 			SHEET_BUYO, 	PAL2, 1, &ai_null, 				&ai_buyobuyo, 		&ondeath_default 	}, // Buyobuyo
	{ NULL, 			SHEET_CORES1,	PAL2, 1, &onspawn_persistent, 	&ai_minicore_shot, 	&ondeath_nodrop 	}, // Core Spinning Projectile
	{ NULL, 			SHEET_CORES3,	PAL2, 1, &onspawn_persistent, 	&ai_core_ghostie, 	&ondeath_nodrop 	}, // Core Wisp Projectile
	{ &SPR_Curly, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_curly_ai, 		&ai_null 			}, // Curly (AI)
	{ NULL, 			SHEET_CGUN, 	PAL1, 1, &onspawn_persistent, 	&ai_cai_gun, 		&ai_null 			}, // Curly Polar Star
	{ NULL, 			SHEET_CGUN, 	PAL1, 1, &onspawn_persistent, 	&ai_cai_gun, 		&ai_null 			}, // Curly Machine Gun
	{ &SPR_Bubble, 		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_cai_watershield,&ai_null 			}, // Curly Bubble Shield
	{ &SPR_BigShutter,	NOSHEET,		PAL2, 1, &onspawn_shutter, 		&ai_shutter, 		&ai_null 			}, // Shutter (Large)
	{ &SPR_Shutter, 	NOSHEET, 		PAL2, 1, &onspawn_shutter, 		&ai_shutter, 		&ai_null 			}, // Shutter (Small)
	{ &SPR_CoreLift,	NOSHEET, 		PAL2, 1, &ai_null, 				&ai_shutter, 		&ai_null 			}, // Lift Block
	{ &SPR_FuzzCore,	NOSHEET, 		PAL2, 1, &ai_null, 				&ai_fuzz_core, 		&ondeath_default 	}, // Fuzz Core
	{ &SPR_Fuzz, 		NOSHEET, 		PAL2, 1, &onspawn_persistent, 	&ai_fuzz, 			&ondeath_default 	}, // Fuzz
	{ NULL, 			SHEET_CORES4,	PAL1, 1, &onspawn_persistent, 	&ai_core_blast, 	&ai_null 			}, // Homing Flame Projectile
	{ &SPR_Robot3, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_almond_robot, 	&ai_null 			}, // Surface Robot
	{ NULL, 			NOSHEET, 		PAL0, 2, &onspawn_persistent, 	&ai_waterlevel, 	&ai_null 			}, // Water Level
	/* 0x0C0 (192) */
	{ &SPR_Buggy, 		NOSHEET, 		PAL3, 1, &onspawn_pushup, 		&ai_motorbike, 		&ai_null 			}, // Scooter
	{ &SPR_Buggy2, 		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_null, 			&ai_null 			}, // Scooter (Pieces)
	{ &SPR_Robot, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Blue Robot (Pieces)
	{ &SPR_GrateMouth,	NOSHEET,		PAL2, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Grate Mouth
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // Motion Wall
	{ NULL, 			SHEET_BLOWFISH, PAL3, 1, &onspawn_persistent, 	&ai_ironh_fishy, 	&ondeath_default 	}, // Porcupine Fish
	{ &SPR_ManShot, 	NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_ironh_shot, 	&ai_null 			}, // Ironhead Projectile
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // Underwater Current
	{ &SPR_BabyDragon,	NOSHEET,		PAL3, 2, &ai_null, 				&ai_dragon_zombie, 	&ai_null 			}, // Dragon Zombie
	{ &SPR_BabyDragon,	NOSHEET,		PAL3, 2, &onspawn_deaddragon, 	&ai_null, 			&ai_null 			}, // Dragon Zombie (Dead)
	{ NULL, 			SHEET_FIRE, 	PAL1, 1, &onspawn_persistent, 	&ai_genericproj, 	&ai_null 			}, // Dragon Zombie Projectile
	{ NULL, 			SHEET_CRITTER,  PAL3, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Blue Hopping Critter
	{ NULL, 			SHEET_SMSTAL, 	PAL2, 1, &ai_null, 				&ai_fallingspike_sm,&ondeath_default 	}, // Falling Spike (Small)
	{ NULL, 			SHEET_LGSTAL, 	PAL2, 1, &ai_null, 				&ai_fallingspike_lg,&ondeath_default 	}, // Falling Spike (Large)
	{ &SPR_Cloud, 		NOSHEET, 		PAL3, 4, &ai_null, 				&ai_counterbomb, 	&ondeath_default 	}, // Counter Bomb
	{ &SPR_Countdown,	NOSHEET,		PAL3, 1, &onspawn_persistent, 	&ai_counterbomb_num,&ai_null 			}, // Countdown Balloon
	// 0x0D0 (208) */
	{ NULL, 			SHEET_BASU, 	PAL3, 1, &onspawn_basu, 		&ai_basu, 			&ondeath_default 	}, // Basu (2)
	{ NULL, 			SHEET_BASUSHOT, PAL3, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Basu Projectile (2)
	{ NULL, 			SHEET_BEETLE, 	PAL3, 1, &onspawn_beetleFollow, &ai_beetleFollow, 	&ondeath_default 	}, // Green Beetle (Follow 2)
	{ &SPR_Spikes, 		NOSHEET, 		PAL1, 1, &onspawn_spike, 		&ai_null, 			&ai_null 			}, // Spikes
	{ &SPR_SkyDragon, 	NOSHEET, 		PAL3, 4, &ai_null, 				&ai_sky_dragon, 	&ai_null 			}, // Sky Dragon
	{ &SPR_NightSpirit,	NOSHEET,		PAL2, 4, &onspawn_persistent, 	&ai_night_spirit, 	&ondeath_default 	}, // Night Spirit
	{ NULL, 			SHEET_NIGHTSHOT,PAL2, 1, &onspawn_persistent, 	&ai_night_spirit_sh,&ondeath_default 	}, // Night Spirit Projectile
	{ &SPR_Croc2, 		NOSHEET, 		PAL2, 2, &ai_null, 				&ai_sandcroc, 		&ondeath_default 	}, // White Sandcroc
	{ &SPR_Cat, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Debug Cat
	{ &SPR_Itoh, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_npc_itoh, 		&ai_null 			}, // Itoh
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // Smoke/Current Generator
	{ &SPR_ShovelMimi,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_mimiga_farmer, 	&ai_null 			}, // Shovel Brigade
	{ &SPR_ShovelMimi,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_mimiga_farmer, 	&ai_null 			}, // Shovel Brigade (Walking)
	{ &SPR_JailBars, 	NOSHEET, 		PAL1, 1, &onspawn_jailbars, 	&ai_null, 			&ai_null 			}, // Prison Bars
	{ &SPR_Momo, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_npc_momorin, 	&ai_null 			}, // Momorin
	/* 0x0E0 (224) */
	{ &SPR_Chie, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Chie
	{ &SPR_Megane, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Megane
	{ &SPR_Kanpachi2,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_kanpachi_stand, &ai_null 			}, // Kanpachi
	{ &SPR_Bucket, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Bucket
	{ &SPR_Droll, 		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_droll_guard, 	&ai_null 			}, // Droll (Guard)
	{ &SPR_RedFl, 		NOSHEET, 		PAL0, 2, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Red Flower Sprouts
	{ &SPR_RedFl3, 		NOSHEET, 		PAL0, 2, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Blooming Red Flowers
	{ &SPR_Rocket, 		NOSHEET, 		PAL0, 1, &ai_null, 				&ai_rocket, 		&ai_null 			}, // Rocket
	{ &SPR_BigBat, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_orangebell, 	&ondeath_default 	}, // Orangebell
	{ NULL, 			SHEET_BAT,		PAL3, 1, &ai_null, 				&ai_orangebell_baby,&ondeath_default 	}, // Orangebell Baby
	{ &SPR_RedFl, 		NOSHEET, 		PAL0, 2, &onspawn_snap, 		&ai_null, 			&ai_null 			}, // Picked Red Flowers
	{ NULL, 			SHEET_MIDO, 	PAL0, 1, &ai_null, 				&ai_midorin, 		&ondeath_default 	}, // Midorin
	{ &SPR_Gunfish, 	NOSHEET, 		PAL0, 1, &ai_null, 				&ai_gunfish, 		&ondeath_default 	}, // Gunfish
	{ NULL, 			SHEET_GUNFSHOT, PAL0, 1, &onspawn_persistent, 	&ai_gunfish_shot, 	&ondeath_nodrop 	}, // Gunfish Projectile
	{ NULL, 			SHEET_PRESS, 	PAL1, 1, &ai_null, 				&ai_prox_press_hoz, &ondeath_default 	}, // Lethal Press
	{ &SPR_JailBars2,	NOSHEET,		PAL2, 2, &onspawn_cent_cage, 	&ai_null, 			&ai_null 			}, // Cage Bars
	/* 0x0F0 (240) */
	{ &SPR_JailedMimi,	NOSHEET,		PAL3, 1, &ai_null, 				&ai_mimiga_caged, 	&ai_null 			}, // Jailed Mimiga
	{ NULL, 			SHEET_CRITTER, 	PAL3, 1, &onspawn_snap, 		&ai_critter, 		&ondeath_default 	}, // Red Critter
	{ NULL, 			SHEET_BAT, 		PAL3, 1, &onspawn_persistent, 	&ai_red_bat, 		&ondeath_default 	}, // Red Bat
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_red_bat_spawner,&ai_null 			}, // Red Bat Generator
	{ NULL, 			SHEET_ACID, 	PAL3, 1, &onspawn_persistent, 	&ai_lava_drip, 		&ai_null 			}, // Acid Drop
	{ NULL, 			SHEET_ACID, 	PAL3, 1, &ai_null, 				&ai_lava_spawner,	&ai_null 			}, // Acid Drop Generator
	{ NULL, 			SHEET_PRESS, 	PAL1, 1, &onspawn_press, 		&ai_prox_press_vert,&ondeath_default 	}, // Press (Proximity)
	{ &SPR_Misery2, 	NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_boss_misery, 	&ondeath_boss_misery }, // Misery (Boss)
	{ NULL, 			SHEET_IGORSHOT, PAL3, 1, &onspawn_persistent, 	&ai_genericproj, 	&ondeath_nodrop 	}, // Misery Energy Shot
	{ &SPR_Misery2, 	NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Misery (Vanish)
	{ NULL, 			SHEET_RING, 	PAL3, 1, &onspawn_persistent, 	&ai_misery_ball, 	&ai_null 			}, // Misery Lightning Ball
	{ NULL, 			SHEET_SHOCK, 	PAL3, 1, &onspawn_persistent, 	&ai_black_lightning,&ai_null 			}, // Misery Lightning
	{ NULL, 			SHEET_RING, 	PAL3, 1, &onspawn_persistent, 	&ai_misery_ring, 	&ondeath_default 	}, // Misery Ring
	{ &SPR_EnCap, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_xp_capsule, 	&ondeath_default 	}, // Energy Capsule
	{ &SPR_Heli, 		NOSHEET, 		PAL1, 8, &onspawn_persistent, 	&ai_heli, 			&ai_null 			}, // Helicopter
	{ &SPR_HeliBlade,	NOSHEET, 		PAL1, 4, &onspawn_heliblade1, 	&ai_heli_blade, 	&ai_null 			}, // Helicopter blade
	/* 0x100 (256) */
	{ &SPR_Doctor, 		NOSHEET,		PAL3, 1, &onspawn_persistent, 	&ai_doctor_crowned, &ai_null 			}, // Doctor (Before Fight)
	{ NULL, 			SHEET_CRYSTAL, 	PAL1, 1, &onspawn_red_crystal, 	&ai_red_crystal, 	&ai_null 			}, // Red Crystal
	{ &SPR_MimiSleep,	NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Sleeping Mimiga
	{ &SPR_Curly, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_curly_carried, 	&ai_null 			}, // Curly (Carried)
	{ NULL, 			SHEET_MIMI,		PAL3, 1, &ai_null, 				&ai_mimiga_caged, 	&ai_null 			}, // Shovel Brigade (Caged)
	{ &SPR_Chie, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_mimiga_caged, 	&ai_null 			}, // Chie (Caged)
	{ &SPR_Chaco2, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_mimiga_caged, 	&ai_null 			}, // Chaco (Caged)
	{ &SPR_DoctorB, 	NOSHEET, 		PAL1, 1, &onspawn_persistent, 	&ai_boss_doctor, 	&ondeath_event 		}, // Doctor (Boss)
	{ NULL, 			SHEET_DOCSHOT, 	PAL1, 1, &onspawn_persistent, 	&ai_doctor_shot, 	&ondeath_nodrop 	}, // Doctor Red Wave
	{ NULL, 			SHEET_DOCSHOT, 	PAL1, 1, &onspawn_persistent, 	&ai_doctor_blast, 	&ondeath_nodrop 	}, // Doctor Red Ball (Fast)
	{ NULL, 			SHEET_DOCSHOT, 	PAL1, 1, &onspawn_persistent, 	&ai_doctor_blast, 	&ondeath_nodrop 	}, // Doctor Red Ball (Slow)
	{ &SPR_DoctorM, 	NOSHEET, 		PAL1, 4, &onspawn_persistent, 	&ai_muscle_doctor, 	&ondeath_event 		}, // Muscle Doctor (Boss)
	{ &SPR_Igor, 		NOSHEET, 		PAL3, 4, &ai_null, 				&ai_igor_balcony, 	&ondeath_default 	}, // Igor
	{ NULL, 			SHEET_BAT, 		PAL1, 1, &onspawn_persistent, 	&ai_doctor_bat, 	&ondeath_default 	}, // Red Energy Bat
	{ NULL, 			SHEET_REDDOT, 	PAL1, 1, &onspawn_persistent, 	&ai_red_energy, 	&ai_null 			}, // Red Energy
	{ NULL, 			SHEET_IRONHBLK, PAL2, 1, &onspawn_persistent, 	&ai_ironh_brick, 	&ai_null 			}, // Underwater Block
	/* 0x110 (272) */
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_brick_spawner, 	&ai_null 			}, // Water Block Generator
	{ &SPR_DrollShot, 	NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_droll_shot, 	&ondeath_nodrop 	}, // Droll Projectile
	{ &SPR_Droll, 		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_droll, 			&ondeath_default 	}, // Droll
	{ &SPR_Puppy, 		NOSHEET, 		PAL1, 1, &onspawn_puppy, 		&ai_puppy_wag, 		&ai_null 			}, // Puppy (With Item)
	{ &SPR_DrollRed, 	NOSHEET, 		PAL3, 2, &ai_null, 				&ai_red_demon, 		&ondeath_red_demon 	}, // Red Demon
	{ &SPR_DrollShot, 	NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_droll_shot, 	&ondeath_nodrop 	}, // Red Demon Projectile
	{ &SPR_Littles, 	NOSHEET, 		PAL3, 1, &ai_null, 				&ai_little_family, 	&ai_null 			}, // Little Family
	{ NULL, 			SHEET_BLOCK, 	PAL2, 1, &onspawn_persistent, 	&ai_falling_block, 	&ai_null 			}, // Falling Block (Large)
	{ &SPR_Sue, 		NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_sue_teleport_in,&ai_null 			}, // Sue (Teleport In)
	{ NULL, 			NOSHEET, 		PAL1, 0, &onspawn_persistent, 	&ai_doctor_ghost, 	&ai_null 			}, // Doctor (Red Energy)
	{ NULL, 			SHEET_MUCORE, 	PAL2, 3, &onspawn_ud_minicore, 	&ai_udmini_platform,&ai_null 			}, // Mini Undead Core
	{ &SPR_MizaMisery,	NOSHEET,		PAL3, 1, &onspawn_persistent, 	&ai_misery_frenzied,&ai_null 			}, // Misery (Transformed)
	{ &SPR_MizaSue, 	NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_sue_frenzied, 	&ai_null 			}, // Sue (Transformed)
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ondeath_nodrop 	}, // Orange Spinning Shot
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Orange Dot
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Orange Smoke
	/* 0x120 (288) */
	{ NULL, 			SHEET_ROCK, 	PAL3, 1, &onspawn_persistent, 	&ai_ud_pellet, 		&ondeath_nodrop 	}, // Glowing Rock Shot
	{ NULL, 			SHEET_CRITTER, 	PAL3, 1, &onspawn_persistent, 	&ai_misery_critter, &ondeath_default 	}, // Orange Critter
	{ NULL, 			SHEET_BAT, 		PAL3, 1, &onspawn_persistent, 	&ai_misery_bat, 	&ondeath_default 	}, // Orange Bat
	{ NULL, 			SHEET_MUCORE, 	PAL2, 3, &onspawn_ud_minicore, 	&ai_udmini_idle, 	&ai_null 			}, // Mini Core (Before Fight)
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_quake, 			&ai_null 			}, // Quake
	{ NULL, 			SHEET_CORES4, 	PAL1, 1, &onspawn_persistent, 	&ai_ud_blast, 		&ai_null 			}, // Huge Energy Shot
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_block_spawner, 	&ai_null 			}, // Falling Block Generator
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Cloud
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Cloud Generator
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Doctor (Uncrowned)
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Balrog/Misery (Bubble)
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Demon Crown
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ondeath_nodrop 	}, // Fish Missile (Orange)
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_scroll_ctrl, 	&ai_null 			}, // Scroll controller
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	/* 0x130 (304) */
	{ NULL, 			NOSHEET, 		PAL2, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Gaudi (Sitting)
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Puppy (Small)
	{ NULL, 			NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Balrog (Nurse)
	{ &SPR_Santa, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_mimiga_caged, 	&ai_null 			}, // Santa (Caged)
	{ NULL, 			SHEET_STUMPY, 	PAL0, 1, &ai_null, 				&ai_stumpy, 		&ondeath_default 	}, // Stumpy
	{ NULL, 			SHEET_BUTE, 	PAL3, 1, &ai_null, 				&ai_bute_flying, 	&ondeath_default 	}, // Bute
	{ NULL, 			SHEET_BUTE, 	PAL3, 1, &ai_null, 				&ai_bute_sword, 	&ondeath_default 	}, // Bute (Sword)
	{ NULL, 			SHEET_BUTE, 	PAL3, 1, &ai_null, 				&ai_bute_archer, 	&ondeath_default 	}, // Bute (Archer)
	{ NULL, 			SHEET_BUTEARW, 	PAL1, 1, &onspawn_persistent, 	&ai_bute_arrow, 	&ai_null 			}, // Bute Projectile
	{ NULL,				SHEET_MAPI, 	PAL3, 1, &onspawn_persistent, 	&ai_ma_pignon, 		&ondeath_default 	}, // Ma Pignon
	{ NULL, 			SHEET_ROCK, 	PAL2, 1, &onspawn_persistent, 	&ai_ma_pignon_rock, &ondeath_default 	}, // Rock
	{ NULL, 			SHEET_MAPI, 	PAL3, 1, &onspawn_persistent, 	&ai_ma_pignon_clone,&ondeath_default 	}, // Clone
	{ NULL, 			SHEET_BUTEDIE, 	PAL3, 1, &ai_null, 				&ai_bute_dying, 	&ondeath_default 	}, // Bute (Defeated)
	{ &SPR_Mesa, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_mesa, 			&ondeath_default 	}, // Mesa
	{ &SPR_Mesa, 		NOSHEET, 		PAL3, 1, &ai_null, 				&ai_bute_dying, 	&ondeath_default 	}, // Mesa (Defeated)
	{ &SPR_ToroBlock, 	NOSHEET, 		PAL1, 1, &onspawn_persistent, 	&ai_mesa_block, 	&ai_null 			}, // Mesa Block
	/* 0x140 (320) */
	{ &SPR_Curly, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_curly_hell, 	&ai_null 			}, // Curly (Hell)
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			SHEET_DELEET, 	PAL1, 1, &onspawn_deleet, 		&ai_deleet, 		&ondeath_default 	}, // Deleet
	{ NULL, 			SHEET_BUTE, 	PAL3, 1, &onspawn_persistent, 	&ai_bute_flying, 	&ondeath_default 	}, // Bute (Generated)
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_bute_spawner, 	&ai_null 			}, // Bute Generator
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Heavy Press Projectile
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Itoh/Sue (Ending)
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Transmogrifier
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Building Fan
	{ NULL, 			SHEET_ROLLING, 	PAL1, 1, &onspawn_persistent, 	&ai_rolling, 		&ondeath_default 	}, // Rolling
	{ NULL, 			SHEET_BONE, 	PAL3, 1, &onspawn_persistent, 	&ai_ballos_bone, 	&ai_null 			}, // Ballos Bone Shot
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_ballos_bone_spawner,&ai_null 		}, // Ballos Shockwave
	{ NULL, 			SHEET_TARGET, 	PAL1, 1, &onspawn_persistent, 	&ai_ballos_target, 	&ai_null 			}, // Ballos Target
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Sweat
	{ NULL, 			SHEET_IKACHAN,  PAL0, 1, &onspawn_persistent, 	&ai_ikachan, 		&ai_null 			}, // Ika-chan
	/* 0x150 (336) */
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_ikachan_spawner,&ai_null 			}, // Ika-chan Generator
	{ &SPR_Droll3, 		NOSHEET, 		PAL3, 2, &ai_null, 				&ai_numahachi, 		&ai_null 			}, // Numahachi
	{ NULL, 			SHEET_DEVIL, 	PAL3, 1, &onspawn_persistent, 	&ai_green_devil, 	&ondeath_default 	}, // Green Devil
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_green_devil_spawner,&ai_null 		}, // Green Devil Generator
	{ &SPR_BallosP, 	NOSHEET, 		PAL0, 4, &onspawn_persistent, 	&ai_ballos_priest, 	&ondeath_default 	}, // Ballos (Boss)
	{ &SPR_BallosSm, 	NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Ballos Smile
	{ NULL, 			SHEET_ROT, 		PAL0, 1, &onspawn_persistent, 	&ai_ballos_rotator, &ai_null 			}, // Ballos Rotator
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Ballos Skull Shot
	{ NULL, 			SHEET_PLATF, 	PAL0, 1, &onspawn_persistent, 	&ai_ballos_platform, &ai_null 			}, // Ballos Platform
	{ &SPR_Hoppy, 		NOSHEET, 		PAL1, 1, &ai_null, 				&ai_hoppy, 			&ondeath_default 	}, // Hoppy
	{ &SPR_Spikes, 		NOSHEET, 		PAL1, 1, &onspawn_persistent, 	&ai_ballos_spikes, 	&ai_null 			}, // Ballos Spikes
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Statue
	{ NULL, 			NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_null, 			&ondeath_default 	}, // Red Bute (Archer)
	{ NULL, 			NOSHEET, 		PAL0, 2, &ai_null, 				&ai_null, 			&ondeath_default 	}, // Statue (Breakable)
	/* 0x160 (352) */
	{ &SPR_King, 		NOSHEET, 		PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // King (Sword)
	{ NULL, 			SHEET_BUTE, 	PAL3, 1, &onspawn_persistent, 	&ai_null, 			&ondeath_default 	}, // Red Bute (Sword)
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // Kill Zone
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_null, 			&ai_null 			}, // ???
	{ &SPR_Balrog, 		NOSHEET, 		PAL1, 2, &onspawn_persistent, 	&ai_balrog, 		&ai_null 			}, // Balrog (Rescue)
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Puppy (Ghost)
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Misery (Wind)
	{ NULL, 			NOSHEET, 		PAL0, 1, &ai_null, 				&ai_null, 			&ai_null 			}, // Water Drop Generator
	{ NULL, 			NOSHEET, 		PAL0, 2, &ai_null, 				&ai_null, 			&ai_null 			}, // Thank You
	/* MAJOR BOSSES - Don't exist in npc.tbl, but need AI methods */
	/* 0x169 (361) */
	{ &SPR_Omega, 		NOSHEET, 		PAL3, 6, &onspawn_omega, 		&ai_omega, 			&ondeath_omega 		}, // Omega
	{ &SPR_Balfrog1, 	NOSHEET, 		PAL3, 6, &onspawn_balfrog, 		&ai_balfrog, 		&ondeath_balfrog 	}, // Balfrog
	{ NULL, 			NOSHEET, 		PAL3, 4, &onspawn_monsterx, 	&ai_monsterx, 		&ondeath_monsterx 	}, // Monster X
	{ NULL, 			NOSHEET, 		PAL3, 1, &onspawn_core, 		&ai_core, 			&ondeath_core 		}, // Core (Controller)
	{ &SPR_Ironh, 		NOSHEET, 		PAL3, 2, &onspawn_ironhead, 	&ai_ironhead, 		&ondeath_ironhead 	}, // Ironhead
	{ NULL, 			NOSHEET, 		PAL3, 3, &onspawn_sisters, 		&ai_sisters, 		&ondeath_sisters 	}, // Sisters
	{ NULL, 			NOSHEET, 		PAL3, 0, &onspawn_undead_core, 	&ai_undead_core, 	&ai_null 			}, // Undead Core
	{ &SPR_HeavyPress, 	NOSHEET, 		PAL1, 12,&onspawn_heavypress, 	&ai_heavypress, 	&ondeath_heavypress }, // Heavy Press
	{ NULL, 			NOSHEET, 		PAL3, 0, &onspawn_ballos, 		&ai_ballos, 		&ondeath_ballos 	}, // Ballos
	/* BOSS PARTS - Separate entities belonging to the bosses which don't already exist in the NPC table, for whatever reason */
	/* 0x172 (370) */
	{ &SPR_CoreFront, 	NOSHEET, 		PAL2, 6, &onspawn_persistent, 	&ai_core_front, 	&ai_null 			}, // Core (Front)
	{ &SPR_CoreBack, 	NOSHEET, 		PAL2, 9, &onspawn_persistent, 	&ai_core_back, 		&ai_null 			}, // Core (Back)
	{ NULL, 			NOSHEET, 		PAL2, 4, &onspawn_persistent, 	&ai_minicore, 		&ai_null 			}, // Mini Core
	{ NULL, 			SHEET_OMGLEG, 	PAL3, 1, &onspawn_omega_leg, 	&ai_null, 			&ai_null 			}, // Omega Leg
	{ &SPR_OmgStrut, 	NOSHEET, 		PAL3, 1, &onspawn_omega_strut, 	&ai_null, 			&ai_null 			}, // Omega Strut
	{ NULL, 			NOSHEET, 		PAL3, 2, &onspawn_x_tread,	 	&ai_x_tread, 		&ai_null 			}, // Monster X Tread
	{ &SPR_XDoor, 		NOSHEET, 		PAL3, 4, &onspawn_x_door, 		&ai_x_door, 		&ai_null 			}, // Monster X Door
	{ NULL, 			SHEET_XTARGET, 	PAL3, 1, &onspawn_x_target, 	&ai_x_target, 		&ondeath_x_target 	}, // Monster X Target
	{ &SPR_XInts, 		NOSHEET, 		PAL3, 6, &onspawn_x_internals, 	&ai_x_internals, 	&ai_null 			}, // Monster X Internals
	{ &SPR_SisBody, 	NOSHEET, 		PAL3, 2, &onspawn_sisters_body, &ai_sisters_body, 	&ai_null 			}, // Sisters Body
	{ NULL, 			SHEET_SISHEAD, 	PAL3, 1, &onspawn_sisters_head, &ai_sisters_head, 	&ai_null 			}, // Sisters Head
	{ &SPR_UCoreFront, 	NOSHEET, 		PAL2, 6, &onspawn_persistent, 	&ai_undead_core_front,&ai_null 			}, // Undead Core (Front)
	{ &SPR_UCoreBack, 	NOSHEET, 		PAL2, 9, &onspawn_persistent, 	&ai_undead_core_back,&ai_null 			}, // Undead Core (Back)
	{ &SPR_UCoreMouth, 	NOSHEET, 		PAL2, 2, &onspawn_persistent, 	&ai_undead_core_face,&ai_null 			}, // Undead Core (Face)
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Heavy Press Shield
	{ &SPR_Ballos, 		NOSHEET, 		PAL0, 15,&onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Ballos Body
	{ &SPR_BallosEye, 	NOSHEET, 		PAL0, 1, &onspawn_persistent, 	&ai_ballos_eye, 	&ai_null 			}, // Ballos Eye
	{ NULL, 			NOSHEET, 		PAL0, 0, &onspawn_persistent, 	&ai_null, 			&ai_null 			}, // Ballos Shield
	/* Splash screen */
	{ &SPR_Sega, 		NOSHEET, 		PAL0, 3, &onspawn_segalogo, 	&ai_segalogo, 		&ai_null 			}, // Sega Logo
	{ &SPR_Sega2, 		NOSHEET, 		PAL0, 3, &onspawn_segalogo, 	&ai_segalogo, 		&ai_null 			}, // Sega Logo being crushed
	{ &SPR_Balrog, 		NOSHEET, 		PAL1, 2, &onspawn_balrog_splash,&ai_balrog_splash, 	&ai_null 			}, // Balrog in splash screen
	/* Misc */
	{ NULL, 			NOSHEET, 		PAL0, 0, &ai_null, 				&ai_trigger_special,&ai_null 			}, // Out of bounds trigger
	{ &SPR_HeliBlade2, 	NOSHEET, 		PAL1, 3, &onspawn_heliblade2, 	&ai_heli_blade, 	&ai_null 			}, // Helicopter blade (smaller)
	{ &SPR_LevelUp, 	NOSHEET, 		PAL1, 2, &onspawn_lvlupdn, 		&ai_lvlupdn, 		&ai_null 			}, // Level Up (English)
	{ &SPR_J_LevelUp, 	NOSHEET, 		PAL1, 2, &onspawn_lvlupdn, 		&ai_lvlupdn, 		&ai_null 			}, // Level Up (Japanese)
	{ &SPR_LevelDown, 	NOSHEET, 		PAL1, 2, &onspawn_lvlupdn, 		&ai_lvlupdn, 		&ai_null 			}, // Level Down (English)
	{ &SPR_J_LevelDown, NOSHEET, 		PAL1, 2, &onspawn_lvlupdn, 		&ai_lvlupdn, 		&ai_null 			}, // Level Down (Japanese)
};
