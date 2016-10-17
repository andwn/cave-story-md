#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include "common.h"

// IDs for sounds, identical to NXEngine for AI compatibility
#define SND_MENU_MOVE		1
#define SND_MSG				2
#define SND_BONK_HEAD		3
#define SND_SWITCH_WEAPON	4
#define SND_MENU_PROMPT		5
#define SND_HOPPY_JUMP		6
#define SND_DOOR			11
#define SND_BLOCK_DESTROY	12
#define SND_GET_XP			14
#define SND_PLAYER_JUMP		15
#define SND_PLAYER_HURT		16
#define SND_PLAYER_DIE		17
#define SND_MENU_SELECT		18
#define SND_HEALTH_REFILL	20
#define SND_BUBBLE			21
#define SND_CHEST_OPEN		22
#define SND_THUD			23
#define SND_PLAYER_WALK		24
#define SND_FUNNY_EXPLODE	25
#define SND_QUAKE			26
#define SND_LEVEL_UP		27
#define SND_SHOT_HIT		28
#define SND_TELEPORT		29
#define SND_ENEMY_JUMP		30
#define SND_TINK			31
#define SND_POLAR_STAR_L1_2	32
#define SND_SNAKE_FIRE		33
#define SND_FIREBALL		34
#define SND_EXPLOSION1		35
#define SND_GUN_CLICK		37
#define SND_GET_ITEM		38
#define SND_EM_FIRE			39
#define SND_STREAM1			40
#define SND_STREAM2			41
#define SND_GET_MISSILE		42
#define SND_COMPUTER_BEEP	43
#define SND_MISSILE_HIT		44
#define SND_XP_BOUNCE		45
#define SND_IRONH_SHOT_FLY	46
#define SND_EXPLOSION2		47
#define SND_BUBBLER_FIRE	48
#define SND_POLAR_STAR_L3	49
#define SND_ENEMY_SQUEAK	50
#define SND_ENEMY_HURT		51
#define SND_ENEMY_HURT_BIG	52
#define SND_ENEMY_HURT_SMALL	53
#define SND_ENEMY_HURT_COOL	54
#define SND_ENEMY_SQUEAK2	55
#define SND_SPLASH			56
#define SND_ENEMY_DAMAGE	57
#define SND_PROPELLOR		58
#define SND_SPUR_CHARGE_1	59
#define SND_SPUR_CHARGE_2	60
#define SND_SPUR_CHARGE_3	61
#define SND_SPUR_FIRE_1		62
#define SND_SPUR_FIRE_2		63
#define SND_SPUR_FIRE_3		64
#define SND_SPUR_MAXED		65
#define SND_EXPL_SMALL		70
#define SND_LITTLE_CRASH	71
#define SND_BIG_CRASH		72
#define SND_BUBBLER_LAUNCH	100
#define SND_LIGHTNING_STRIKE	101
#define SND_JAWS			102
#define SND_CHARGE_GUN		103
#define SND_104				104
#define SND_PUPPY_BARK		105
#define SND_SLASH			106
#define SND_BLOCK_MOVE		107
#define SND_IGOR_JUMP		108
#define SND_CRITTER_FLY		109
#define SND_DROLL_SHOT_FLY	110
#define SND_MOTOR_RUN		111
#define SND_MOTOR_SKIP		112
#define SND_BOOSTER			113
#define SND_CORE_HURT		114
#define SND_CORE_THRUST		115
#define SND_CORE_CHARGE		116
#define SND_NEMESIS_FIRE	117
#define SND_150		150
#define SND_151		151
#define SND_152		152
#define SND_153		153
#define SND_154		154
#define SND_155		155

// IDs for the title screen songs
#define SONG_TITLE 		0x18
#define SONG_TOROKO 	0x28
#define SONG_KING 		0x29

// Initialize sound system
void sound_init();
// Play sound by ID, priority is 0-15, 15 is most important
void sound_play(u8 id, u8 priority);
// Play a song by ID
void song_play(u8 id);
// Stops music if playing, same as song_play(0)
void song_stop();
// Resume previous song that was playing
// TSC calls this after fanfare tracks
void song_resume();
// Returns ID of current song
u8 song_get_playing();

#endif /* INC_AUDIO_H_ */
