#ifndef RES_LOCAL_H
#define RES_LOCAL_H

#include "md/types.h"

/* Language pointer tables */
extern const uint8_t LANGUAGE[4];

extern const uint32_t TSC_GLOB[4];
extern const uint32_t TSC_STAGE[95];

extern const uint32_t BMP_ASCII;
extern const uint32_t BMP_KANJI;

extern const uint32_t STAGE_NAMES;
extern const uint32_t CREDITS_STR;
extern const uint32_t CONFIG_STR;

extern const uint16_t *TS_TITLE[];
extern const uint16_t *TS_MENUTEXT[];
extern const uint16_t *TS_MSGTEXT[];

extern const LocSprite * const SPR_AIR;
extern const LocSprite * const SPR_PROMPT;
extern const LocSprite * const SPR_LEVELUP;
extern const LocSprite * const SPR_LEVELDOWN;
extern const LocSprite * const SPR_EMPTY;
extern const LocSprite * const SPR_AHCHOO;
extern const LocSprite * const SPR_HEY;

#endif //RES_LOCAL_H
