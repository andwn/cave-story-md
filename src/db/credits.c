#include "md/types.h"

#include "res/stage.h"
#include "res/pal.h"
#include "resources.h"
#include "md/vdp.h"

#include "tables.h"

// Keep wait times divisible by 16

// Determines which member of the union in credits_info is used
enum CreditCmd {
	TEXT,		// Add a string of text ([)
	ICON,		// Show an icon from casts.pbm (])
	WAIT,		// Delay until next instruction (-)
	MOVE,		// Sets the text X position (+)
	SONG,		// Change the music (!)
	SONG_FADE,	// Fade out music (~)
	FLAG_JUMP,	// Jump to label if a flag is set (f)
	JUMP,		// Jump to label (j)
	LABEL,		// Label that can be jumped to (l)
	PALETTE,	// Swap one of the palettes (no CS equivalent)
	LOADPXE,    // Load 0.pxe
	END,		// Stop scrolling and pause indefinitely (/)
};

#define C_TEXT(str, jstr)	{ .cmd = TEXT, .text = { str, jstr } }
#define C_ICON(id, pal)		{ .cmd = ICON, .icon = { id, pal } }
#define C_WAIT(ticks)		{ .cmd = WAIT, .wait = { ticks } }
#define C_SONG(id)			{ .cmd = SONG, .song = { id } }
#define C_SONG_FADE()		{ .cmd = SONG_FADE }
#define C_FLAG_JUMP(f, l)	{ .cmd = FLAG_JUMP, .fjump = { f, l } }
#define C_JUMP(l)			{ .cmd = JUMP, .jump = { l } }
#define C_LABEL(n)			{ .cmd = LABEL, .label = { n } }
#define C_MOVE(x)			{ .cmd = MOVE, .move = { x } }
#define C_LOADPXE()			{ .cmd = LOADPXE }
#define C_END()				{ .cmd = END }

enum Casts {
	SUE,		KAZUMA,		MOMORIN,	BOOSTER,	ITOH,		DOCTOR,
	CURLY,		COLON,		JENKA,		PUPPY,		MISERY,		BALROG,
	TOROKO,		KING,		JACK,		MAHIN,		SANTA,		CHACO,
	SANDAIME,	KANPACHI,	SHOVELBR,	MEGANE,		CHIE,		ZETT,
	MALCO,		ROBOT,		CHABA,		GERO,		HASUMI,		POOHBLK,
	LITTLES,	CTHULHU,	BA2,		SKYDRAGN,	GUNSMITH,	MUSIC,
	DEBUGCAT,	CHINFISH,	CRITTER,	BAT,		THEDOOR,	PIGNON,
	GPIGNON,	GKEEPER,	MAPIGNON,	BEETLE,		BASU,		BEHEMOTH,
	BASIL,		PCRITTER,	MANNAN,		JELLY,		KULALA,		PUCHI,
	FROG,		PRESS,		RAVIL,		BABY,		POLISH,		ARMADILL,
	SANDCROC,	SKELETON,	SKULHEAD,	CROW,		SKULSTEP,	FLOWER,
	GAUDI,		ARMOR,		GAUDIEGG,	MONSTERX,	FIREWHIR,	BUYO,
	BUYOBASE,	FUZZ,		FUZZCORE,	BLOWFISH,	IRONHEAD,	CNTRBOMB,
	DRAGONZ,	SISTERS,	NIGHTSPR,	HOPPY,		ORANGEBL,	DROLL,
	MIDORIN,	GUNFISH,	STUMPY,		NUMAHCHI,	REDDEMON,	IGOR,
	ROLLING,	DELEET,		BUTE,		MESA,		GRNDEVIL,	BALLOS,
	BALFROG,	OMEGA,		CORE,		UNDCORE,	TOROKOF,	DOCTORF,
	BALLOS2,	HVPRESS,	NAO,		KUROI,		OKAMI,		SWORD,
	GIDEON,		THANKS,
};

const credits_info_def credits_info[] = {
	C_MOVE(144),
	C_SONG(33),
	C_TEXT("  = CAST =  ",			1),
	C_WAIT(320),
	C_TEXT("- Surface Dwellers -",	2),						C_WAIT(64),
	C_TEXT("Plucky, looks up",		3), 					C_WAIT(16),
	C_TEXT(" to her brother",		0),						C_WAIT(16),
	C_TEXT("  Sue Sakamoto",		4),	C_ICON(SUE,		3), C_WAIT(64),
	C_TEXT("Her occasionally",		5),	 					C_WAIT(16),
	C_TEXT(" reliable brother",		0),						C_WAIT(16),
	C_TEXT("  Kazuma Sakamoto",		6),	C_ICON(KAZUMA,	3), C_WAIT(64),
	C_TEXT("Sue's Mother",			7), 					C_WAIT(32),
	C_TEXT("  Momorin Sakamoto",	8),	C_ICON(MOMORIN,	3), C_WAIT(64),
	C_TEXT("Sue's Grandfather",		9), 					C_WAIT(16),
	C_TEXT(" figure",				0), 					C_WAIT(16),
	C_TEXT("  Booster",				10),C_ICON(BOOSTER,	3), C_WAIT(64),
	C_TEXT("The Mimiga-shaped",		11), 					C_WAIT(16),
	C_TEXT(" coward",				0), 					C_WAIT(16),
	C_TEXT("  Itoh",				12),C_ICON(ITOH,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Mimiga Village -",	13),					C_WAIT(64),
	C_TEXT("Sweet and brave",		14), 					C_WAIT(32),
	C_TEXT("  Toroko",				15),C_ICON(TOROKO,	3), C_WAIT(64),
	C_TEXT("The village leader",	16), 					C_WAIT(32),
	C_TEXT("  King",				17),C_ICON(KING,	3), C_WAIT(64),
	C_TEXT("The Number-Two",		18), 					C_WAIT(32),
	C_TEXT("  Jack",				19),C_ICON(JACK,	3), C_WAIT(64),
	C_TEXT("Never stops eating",	20), 					C_WAIT(32),
	C_TEXT("  Mahin",				21),C_ICON(MAHIN,	3), C_WAIT(64),
	C_TEXT("Loves to fish",			22), 					C_WAIT(32),
	C_TEXT("  Kanpachi",			23),C_ICON(KANPACHI,3), C_WAIT(64),
	C_TEXT("Mans the farm",			24), 					C_WAIT(32),
	C_TEXT("  Sandaime",			25),C_ICON(SANDAIME,3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Grasstown -",			26),					C_WAIT(64),
	C_TEXT("Lives alone",			27), 					C_WAIT(32),
	C_TEXT("  Santa",				28),C_ICON(SANTA,	3), C_WAIT(64),
	C_TEXT("The colorful Mimiga",	29), 					C_WAIT(32),
	C_TEXT("  Chaco",				30),C_ICON(CHACO,	3), C_WAIT(64),
	C_TEXT("Power Room watcher",	31), 					C_WAIT(32),
	C_TEXT("  Malco",				32),C_ICON(MALCO,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Sand Zone -",			33),					C_WAIT(64),
	C_TEXT("Ally of the Mimigas",	34), 					C_WAIT(32),
	C_TEXT("  Curly",				35),C_ICON(CURLY,	3), C_WAIT(64),
	C_TEXT("Raised by Curly",		36), 					C_WAIT(32),
	C_TEXT("  Colon",				37),C_ICON(COLON,	3), C_WAIT(64),
	C_TEXT("Original keeper of",	38), 					C_WAIT(16),
	C_TEXT(" the Sand Zone",		0), 					C_WAIT(16),
	C_TEXT("  Jenka",				39),C_ICON(JENKA,	3), C_WAIT(64),
	C_WAIT(32),
	C_TEXT("Jenka's Puppies",		40), 					C_WAIT(32),
	C_TEXT("  Hajime",				41),C_ICON(PUPPY,	3), C_WAIT(32),
	C_TEXT("  Shinobu",				42),C_ICON(PUPPY,	3), C_WAIT(32),
	C_TEXT("  Kakeru",				43),C_ICON(PUPPY,	3), C_WAIT(32),
	C_TEXT("  Mick",				44),C_ICON(PUPPY,	3), C_WAIT(32),
	C_TEXT("  Nene",				45),C_ICON(PUPPY,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Labyrinth -",			46),					C_WAIT(64),
	C_TEXT("The labyrinth's",		47), 					C_WAIT(16),
	C_TEXT(" weapons dealer",		0), 					C_WAIT(16),
	C_TEXT("  Chaba",				48),C_ICON(CHABA,	3), C_WAIT(64),
	C_TEXT("The labyrinth's",		49), 					C_WAIT(16),
	C_TEXT(" resident physician",	0), 					C_WAIT(16),
	C_TEXT("  Dr. Gero",			50),C_ICON(GERO,	3), C_WAIT(64),
	C_TEXT("I am a nurse!",			51), 					C_WAIT(32),
	C_TEXT("  Nurse Hasumi",		52),C_ICON(HASUMI,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Plantation -",		53),					C_WAIT(64),
	C_TEXT("The girl Mimiga",		54), 					C_WAIT(32),
	C_TEXT("  Chie",				55),C_ICON(CHIE,	3), C_WAIT(64),
	C_TEXT("The grandpa Mimiga",	56), 					C_WAIT(32),
	C_TEXT("  Zett",				57),C_ICON(ZETT,	3), C_WAIT(64),
	C_TEXT("Sprinkler Manager",		58), 					C_WAIT(32),
	C_TEXT("  Megane",				59),C_ICON(MEGANE,	3), C_WAIT(64),
	C_TEXT("Believes in the",		60), 					C_WAIT(16),
	C_TEXT(" Doctor",				0), 					C_WAIT(16),
	C_TEXT("  The Shovel Brigade",	61),C_ICON(SHOVELBR,3), C_WAIT(64),
	C_WAIT(64),
	// Normal Ending villains/others
	C_FLAG_JUMP(2000, 90),
	C_TEXT("- Villains -",			62),					C_WAIT(64),
	C_TEXT("The power-snatching",	63), 					C_WAIT(16),
	C_TEXT(" betrayer",				0), 					C_WAIT(16),
	C_TEXT("  The Doctor",			64),C_ICON(DOCTOR,	3), C_WAIT(64),
	C_TEXT("The Doctor's servant",	65), 					C_WAIT(32),
	C_TEXT("  Misery",				66),C_ICON(MISERY,	3), C_WAIT(64),
	C_TEXT("Misery's punching bag",	67), 					C_WAIT(32),
	C_TEXT("  Balrog",				68),C_ICON(BALROG,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Others -",			69 /* nice */),			C_WAIT(64),
	C_TEXT("The Hermit Gunsmith",	70), 					C_WAIT(32),
	C_TEXT("  Tetsuzou Kamadani",	71),C_ICON(GUNSMITH,3), C_WAIT(64),
	C_TEXT("The mysterious",		72), 					C_WAIT(16),
	C_TEXT(" mushroom",				0), 					C_WAIT(16),
	C_TEXT("  Ma Pignon",			73),C_ICON(MAPIGNON,3), C_WAIT(64),
	C_TEXT("The family living in",	74), 					C_WAIT(16),
	C_TEXT(" the outer wall",		0), 					C_WAIT(16),
	C_TEXT("  The Littles",			75),C_ICON(LITTLES,	3), C_WAIT(64),
	C_TEXT("The mysterious",		76), 					C_WAIT(16),
	C_TEXT(" native inhabitants",	0), 					C_WAIT(16),
	C_TEXT("  Cthulhu",				77),C_ICON(CTHULHU,	3), C_WAIT(64),
	C_TEXT("The statue carver",		78), 					C_WAIT(32),
	C_TEXT("  Numahachi",			79),C_ICON(NUMAHCHI,3), C_WAIT(64),
	C_JUMP(100),
	// Best Ending villains/others
	C_LABEL(90),
	C_TEXT("- Villains -",			80),					C_WAIT(64),
	C_TEXT("Entranced by the crown",81), 					C_WAIT(32),
	C_TEXT("  The Doctor",			82),C_ICON(DOCTOR,	3), C_WAIT(64),
	C_TEXT("Cursed by the crown",	83), 					C_WAIT(32),
	C_TEXT("  Misery",				84),C_ICON(MISERY,	3), C_WAIT(64),
	C_TEXT("Ultimately a friend",	85), 					C_WAIT(32),
	C_TEXT("  Balrog",				86),C_ICON(BALROG,	3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Others -",			87),					C_WAIT(64),
	C_TEXT("The Hermit Gunsmith",	88), 					C_WAIT(32),
	C_TEXT("  Tetsuzou Kamadani",	89),C_ICON(GUNSMITH,3), C_WAIT(64),
	C_TEXT("The memory-restoring",	90), 					C_WAIT(16),
	C_TEXT(" mushroom",				0), 					C_WAIT(16),
	C_TEXT("  Ma Pignon",			91),C_ICON(MAPIGNON,3), C_WAIT(64),
	C_TEXT("The family living in",	92), 					C_WAIT(16),
	C_TEXT(" the outer wall",		0), 					C_WAIT(16),
	C_TEXT("  The Littles",			93),C_ICON(LITTLES,	3), C_WAIT(64),
	C_TEXT("The mysterious",		94), 					C_WAIT(16),
	C_TEXT(" native inhabitants",	0), 					C_WAIT(16),
	C_TEXT("  Cthulhu",				95),C_ICON(CTHULHU,	3), C_WAIT(64),
	C_TEXT("The statue carver",		96), 					C_WAIT(32),
	C_TEXT("  Numahachi",			97),C_ICON(NUMAHCHI,3), C_WAIT(64),
	C_WAIT(64),
	C_TEXT("- Bloody Holy Land -",	98),					C_WAIT(64),
	C_TEXT("Jenka's sorrowful",		99), 					C_WAIT(16),
	C_TEXT(" younger brother",		0), 					C_WAIT(16),
	C_TEXT("  Ballos",				100),C_ICON(BALLOS,	3), C_WAIT(64),
	C_LABEL(100),
	// Transition to second part which has short silent cutscenes in various stages
	C_WAIT(144),
	//C_SONG_FADE(),
	//C_WAIT(200),
	C_WAIT(150),
	C_SONG_FADE(),
	C_WAIT(50),
	C_SONG(1),
	C_MOVE(48),
	
	C_TEXT(" = Monsters = ", 101),										C_WAIT(32),
	C_WAIT(48),
	C_TEXT(" Leaps and hops: Critter", 102),		C_ICON(CRITTER,	3), C_WAIT(48),
	C_TEXT(" Every cave needs a: Bat", 103),		C_ICON(BAT,		3), C_WAIT(48),
	C_TEXT(" Possessed by something: The Door", 104),C_ICON(THEDOOR,3), C_WAIT(48),
	C_TEXT(" Charges when angered: Behemoth", 105),	C_ICON(BEHEMOTH,3), C_WAIT(48),
	C_TEXT(" White mushroom: Pignon", 106),			C_ICON(PIGNON,	3), C_WAIT(48),
	C_TEXT(" Worth eating: Giant Pignon", 107),		C_ICON(GPIGNON,	3), C_WAIT(48),
	C_TEXT(" There's only one: Chinfish", 108),		C_ICON(CHINFISH,3), C_WAIT(48),
	C_TEXT(" Slices you up: Gravekeeper", 109),		C_ICON(GKEEPER,	3), C_WAIT(48),
	C_TEXT(" Floor-hugging flash: Basil", 110),		C_ICON(BASIL,	3), C_WAIT(48),
	C_TEXT(" A model insect: Beetle", 111),			C_ICON(BEETLE,	3), C_WAIT(48),
	C_TEXT(" The big flyer: Basu", 112),			C_ICON(BASU,	3), C_WAIT(48),
	C_TEXT(" The crusher: Power Critter", 113),		C_ICON(PCRITTER,3), C_WAIT(48),
	C_TEXT(" White mold ghost: Mannan", 114),		C_ICON(MANNAN,	3), C_WAIT(48),
	C_TEXT(" Tiny frog: Puchi", 115),				C_ICON(PUCHI,	3), C_WAIT(48),
	C_TEXT(" Big croaker: Frog", 116),				C_ICON(FROG,	3), C_WAIT(48),
	C_TEXT(" Floats about: Jelly", 117),			C_ICON(JELLY,	3), C_WAIT(48),
	C_TEXT(" Queen Jelly: Kulala", 118),			C_ICON(KULALA,	3), C_WAIT(48),
	C_TEXT(" Violent Mimiga: Rabil", 119),			C_ICON(RAVIL,	3), C_WAIT(48),
	C_TEXT(" Instant deathtrap: Press", 120),		C_ICON(PRESS,	3), C_WAIT(48),
	C_TEXT(" Sudden chomper: Sandcroc", 121),		C_ICON(SANDCROC,3), C_WAIT(48),
	C_TEXT(" Wandering skull: Skullhead", 122),		C_ICON(SKULHEAD,3), C_WAIT(48),
	C_TEXT(" Sand runner: Skullstep", 123),			C_ICON(SKULSTEP,3), C_WAIT(48),
	C_TEXT(" White for: Skeleton", 124),			C_ICON(SKELETON,3), C_WAIT(48),
	C_TEXT(" Sand Zone Hunter: Crow", 125),			C_ICON(CROW,	3), C_WAIT(48),
	C_TEXT(" Tough missile: Armadillo", 126),		C_ICON(ARMADILL,3), C_WAIT(48),
	C_TEXT(" From one, many: Polish", 127),			C_ICON(POLISH,	3), C_WAIT(48),
	C_TEXT(" Scattering everywhere: Baby", 128),	C_ICON(BABY,	3), C_WAIT(48),
	C_TEXT(" From Frenzied Toroko: Flower", 129),	C_ICON(FLOWER,	3), C_WAIT(48),
	C_TEXT(" Labyrinth dweller: Gaudi", 130),		C_ICON(GAUDI,	3), C_WAIT(48),
	C_TEXT(" Labyrinth warrior: Armor", 131),		C_ICON(ARMOR,	3), C_WAIT(48),
	C_TEXT(" Labyrinth baby: Gaudi Egg", 132),		C_ICON(GAUDIEGG,3), C_WAIT(48),
	C_TEXT(" Fiery fan: Fire Whirr", 133),			C_ICON(FIREWHIR,3), C_WAIT(48),
	C_TEXT(" Unknown organism: Buyobuyo Base", 134),C_ICON(BUYOBASE,3), C_WAIT(48),
	C_TEXT(" And: Buyobuyo", 135),					C_ICON(BUYO,	3), C_WAIT(48),
	C_TEXT(" Gaudi spirit: Fuzz", 136),				C_ICON(FUZZ,	3), C_WAIT(48),
	C_TEXT(" Spirit clump: Fuzz Core", 137),		C_ICON(FUZZCORE,3), C_WAIT(48),
	C_TEXT(" An old friend: Porcupine Fish", 138),	C_ICON(BLOWFISH,3), C_WAIT(48),
	C_TEXT(" Hatching failure: Dragon Zombie", 139),C_ICON(DRAGONZ,	3), C_WAIT(48),
	C_TEXT(" Timed explosive: Counter Bomb", 140),	C_ICON(CNTRBOMB,3), C_WAIT(48),
	C_TEXT(" Outside flutterer: Night Spirit", 141),C_ICON(NIGHTSPR,3), C_WAIT(48),
	C_TEXT(" Jumps: Hoppy", 142),					C_ICON(HOPPY,	3), C_WAIT(48),
	C_TEXT(" Photosynthetic warrior: Midorin", 143),C_ICON(MIDORIN,	3), C_WAIT(48),
	C_TEXT(" Born of earth: Droll", 144),			C_ICON(DROLL,	3), C_WAIT(48),
	C_TEXT(" Stepping stone: Gunfish", 145),		C_ICON(GUNFISH,	3), C_WAIT(48),
	C_TEXT(" Mother bat: Orangebell", 146),			C_ICON(ORANGEBL,3), C_WAIT(48),
	C_TEXT(" Plantation dragonfly: Stumpy", 147),	C_ICON(STUMPY,	3), C_WAIT(48),
	// Best end only
	C_FLAG_JUMP(2000, 190), C_JUMP(200), C_LABEL(190),
	//C_WAIT(2), // Leaving this out, it messes up the alignment
	C_TEXT(" Hell's messenger: Bute", 148),			C_ICON(BUTE,	3), C_WAIT(48),
	C_TEXT(" Hell's messenger: Mesa", 149),			C_ICON(MESA,	3), C_WAIT(48),
	C_TEXT(" Innocent demon: Green Devil", 150),	C_ICON(GRNDEVIL,3), C_WAIT(48),
	C_TEXT(" Rolls along the walls: Rolling", 151),	C_ICON(ROLLING,	3), C_WAIT(48),
	C_TEXT(" Blocks passages: Deleet", 152),		C_ICON(DELEET,	3), C_WAIT(48),
	C_LABEL(200),
	
	C_WAIT(80),
	C_TEXT("= Bosses =", 153),								C_WAIT(32),
	C_WAIT(48),
	C_TEXT("Frenzied Mimiga that", 154), 					C_WAIT(16),
	C_TEXT(" kidnapped Sue", 0),							C_WAIT(16),
	C_TEXT("  Igor", 155),				C_ICON(IGOR,	3), C_WAIT(64),
	C_TEXT("Misery transformed", 156), 						C_WAIT(16),
	C_TEXT(" Balrog into", 0),								C_WAIT(16),
	C_TEXT("  Balfrog", 157),			C_ICON(BALFROG,	3), C_WAIT(64),
	C_TEXT("Machine-monster that", 158), 					C_WAIT(16),
	C_TEXT(" lurks in the sand", 0),						C_WAIT(16),
	C_TEXT("  Omega", 159),				C_ICON(OMEGA,	3), C_WAIT(64),
	C_TEXT("Dosed with red flowers", 160), 					C_WAIT(32),
	C_TEXT("  Toroko +", 161),			C_ICON(TOROKOF,	3), C_WAIT(64),
	C_TEXT("Ghost that invaded", 162), 						C_WAIT(16),
	C_TEXT(" the clinic", 0),								C_WAIT(16),
	C_TEXT("  Pooh Black", 163),		C_ICON(POOHBLK,	3), C_WAIT(64),
	C_TEXT("Big boss of the", 164), 						C_WAIT(16),
	C_TEXT(" labyrinth", 0),								C_WAIT(16),
	C_TEXT("  Monster X", 165),			C_ICON(MONSTERX,3), C_WAIT(64),
	C_TEXT("Heart of the island", 166), 					C_WAIT(32),
	C_TEXT("  Core", 167),				C_ICON(CORE,	3), C_WAIT(64),
	C_TEXT("Big fish in the", 168), 						C_WAIT(16),
	C_TEXT(" island's artery", 0),							C_WAIT(16),
	C_TEXT("  Ironhead", 169),			C_ICON(IRONHEAD,3), C_WAIT(64),
	C_TEXT("Twin dragon surprise", 170), 					C_WAIT(16),
	C_TEXT(" attackers", 0),								C_WAIT(16),
	C_TEXT("  Sisters", 171),			C_ICON(SISTERS,	3), C_WAIT(64),
	C_TEXT("True heroes meet the", 172), 					C_WAIT(32),
	C_TEXT("  Red Demon", 173),			C_ICON(REDDEMON,3), C_WAIT(64),
	C_TEXT("The red crystal", 174), 						C_WAIT(16),
	C_TEXT(" runs wild", 0),								C_WAIT(16),
	C_TEXT("  Muscle Doctor", 175),		C_ICON(DOCTORF,	3), C_WAIT(64),
	C_TEXT("The Doctor and the", 176), 						C_WAIT(16),
	C_TEXT(" island's heart", 0),							C_WAIT(16),
	C_TEXT("  Undead Core", 177),		C_ICON(UNDCORE,	3), C_WAIT(64),
	// Best end only
	C_FLAG_JUMP(2000, 290), C_JUMP(300), C_LABEL(290),
	//C_WAIT(2),
	C_TEXT("The swollen mech", 178), 						C_WAIT(32),
	C_TEXT("  Heavy Press", 179),		C_ICON(HVPRESS,	3), C_WAIT(64),
	C_TEXT("Hate and madness", 180), 						C_WAIT(16),
	C_TEXT(" given flesh", 0),								C_WAIT(16),
	C_TEXT("  Ballos", 181),			C_ICON(BALLOS2,	3), C_WAIT(64),
	C_LABEL(300),
	
	C_WAIT(80),
	C_MOVE(32),
	C_TEXT(" = BGM = ", 182),								C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Access", 183),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Gestation", 184),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Mimiga Village", 185),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Plant", 186),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Balrog's Theme", 187),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Gravity", 188),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Cemetery", 189),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Safety", 190),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Mischievous Robot", 191),	C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Pulse", 192),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("On to Grasstown", 193),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Eyes of Flame", 194),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Meltdown 2", 195),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Tyrant", 196),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Run!", 197),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Jenka 1", 198),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Jenka 2", 199),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Labyrinth Fight", 200),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Geothermal", 201),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Oppression", 202),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Living Waterway", 203),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Quiet", 204),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Scorching Back", 205),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Moonsong", 206),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Hero's End", 207),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Cave Story", 208),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Last Cave", 209),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Balcony", 210),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Charge", 211),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Zombie", 212),				C_ICON(MUSIC,	3), C_WAIT(32),
	C_WAIT(32),
	C_TEXT("Last Battle", 213),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Break Down", 214),			C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Running Hell", 215),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("Seal Chamber", 216),		C_ICON(MUSIC,	3), C_WAIT(32),
	C_TEXT("The Way Back Home", 217),	C_ICON(MUSIC,	3), C_WAIT(32),
	
	C_WAIT(96),
	C_TEXT("= Special Thanks =", 218),						C_WAIT(32),
	C_MOVE(32),
	C_WAIT(32),
	C_TEXT("Torai", 219),				C_ICON(DEBUGCAT,3), C_WAIT(48),
	C_TEXT("Naoku", 220),				C_ICON(SWORD,	3), C_WAIT(48),
	C_TEXT("Kuroihito", 221),			C_ICON(KUROI,	3), C_WAIT(48),
	C_TEXT("BA2", 222),					C_ICON(BA2,		3), C_WAIT(48),
	C_TEXT("Okami", 223),				C_ICON(OKAMI,	3), C_WAIT(48),
	C_TEXT("Nao", 224),					C_ICON(NAO,		3), C_WAIT(48),
	// Pixel's credits don't include these two, I think AGT added them
	//C_TEXT("Shih Tzu", 225),			C_ICON(PUPPY,	3), C_WAIT(48),
	//C_TEXT("Gideon Zhi", 226),			C_ICON(GIDEON,	3), C_WAIT(48),
	C_TEXT("And many others", 227),							C_WAIT(48),
	// TODO: Probably add some MD specific credits before the end
	//C_WAIT(96),
	//C_TEXT("= Mega Drive Version =", 230),                  C_WAIT(32),
    //C_MOVE(32),
    //C_WAIT(32),
    //C_TEXT("")
	C_LOADPXE(),
	C_WAIT(64),
	C_TEXT("Thank you very much.", 228),C_ICON(THANKS,	3), C_WAIT(32),
	C_WAIT(232),
	//C_WAIT(128),
	C_MOVE(80),
	C_TEXT("Cave Story ~ The End", 229),
	C_WAIT(264),
	C_END()
};

const illustration_info_def illustration_info[20] = {
	{ NULL,                          0,   NULL,                  NULL       },
	{ (const uint32_t*) PAT_Ill01, 411, (const uint16_t*) MAP_Ill01, PAL_Ill01 },
	{ (const uint32_t*) PAT_Ill02, 411, (const uint16_t*) MAP_Ill02, PAL_Ill02 },
	{ (const uint32_t*) PAT_Ill03, 411, (const uint16_t*) MAP_Ill03, PAL_Ill03 },
	{ (const uint32_t*) PAT_Ill04, 411, (const uint16_t*) MAP_Ill04, PAL_Ill04 },
	{ (const uint32_t*) PAT_Ill05, 411, (const uint16_t*) MAP_Ill05, PAL_Ill05 },
	{ (const uint32_t*) PAT_Ill06, 411, (const uint16_t*) MAP_Ill06, PAL_Ill06 },
	{ (const uint32_t*) PAT_Ill07, 411, (const uint16_t*) MAP_Ill07, PAL_Ill07 },
	{ (const uint32_t*) PAT_Ill08, 411, (const uint16_t*) MAP_Ill08, PAL_Ill08 },
	{ (const uint32_t*) PAT_Ill09, 411, (const uint16_t*) MAP_Ill09, PAL_Ill09 },
	{ (const uint32_t*) PAT_Ill10, 411, (const uint16_t*) MAP_Ill10, PAL_Ill10 },
	{ (const uint32_t*) PAT_Ill11, 411, (const uint16_t*) MAP_Ill11, PAL_Ill11 },
	{ (const uint32_t*) PAT_Ill12, 411, (const uint16_t*) MAP_Ill12, PAL_Ill12 },
	{ NULL,                          0,   NULL,                  NULL       },
	{ (const uint32_t*) PAT_Ill14, 411, (const uint16_t*) MAP_Ill14, PAL_Ill14 },
	{ (const uint32_t*) PAT_Ill15, 411, (const uint16_t*) MAP_Ill15, PAL_Ill15 },
	{ (const uint32_t*) PAT_Ill16, 411, (const uint16_t*) MAP_Ill16, PAL_Ill16 },
	{ (const uint32_t*) PAT_Ill17, 411, (const uint16_t*) MAP_Ill17, PAL_Ill17 },
	{ (const uint32_t*) PAT_Ill18, 411, (const uint16_t*) MAP_Ill18, PAL_Ill18 },
	{ NULL,                          0,   NULL,                  NULL       },
};
