#ifndef RES_LOCAL_H
#define RES_LOCAL_H

#include "md/types.h"

typedef enum {
    MENU_INTRO_PRESENTS_1,
    MENU_INTRO_PRESENTS_2,

    MENU_TITLE_START_GAME,
    MENU_TITLE_SOUND_TEST,
    MENU_TITLE_CONFIG,
    MENU_TITLE_STAGE_SELECT,

    MENU_SAVE_NEW_GAME,
    MENU_SAVE_LOAD_DATA,
    MENU_SAVE_COPY_DATA,
    MENU_SAVE_PASTE_DATA,
    MENU_SAVE_DELETE_DATA,
    MENU_SAVE_CONFIRM_DELETE,
    MENU_SAVE_COPY,
    MENU_SAVE_DELETE,

    MENU_SOUND_TRACK,
    MENU_SOUND_PLAY,
    MENU_SOUND_STOP,
    MENU_SOUND_QUIT,
    MENU_SOUND_STOPPED,
    MENU_SOUND_PLAYING,
    MENU_SOUND_PAUSED,

    MENU_CFG_HEADER_1,
    MENU_CFG_HEADER_2,
    MENU_CFG_HEADER_3,

    MENU_CFG_PAD_JUMP,
    MENU_CFG_PAD_SHOOT,
    MENU_CFG_PAD_SWITCH_3BTN,
    MENU_CFG_PAD_FFWD,
    MENU_CFG_PAD_SWITCH_RT,
    MENU_CFG_PAD_SWITCH_LT,
    MENU_CFG_PAD_MAP,
    MENU_CFG_PAD_PAUSE,
    MENU_CFG_PAD_PRESS,

    MENU_CFG_APPLY,
    MENU_CFG_RESET,

    MENU_CFG_60FPS_MODE,
    MENU_CFG_ENABLE_FFWD,
    MENU_CFG_UP_TO_TALK,
    MENU_CFG_HELL_SHAKE,
    MENU_CFG_IFRAME_BUG,
    MENU_CFG_MESSAGE_BLIP,
    MENU_CFG_MUTE_BGM,
    MENU_CFG_MUTE_SFX,

    MENU_CFG_ERASE_COUNTER,
    MENU_CFG_FACTORY_RESET,
    MENU_CFG_CONFIRM_ERASE,
    MENU_CFG_START_3_TIMES,
} MenuLocStr;

/* Language pointer tables */
extern const uint8_t LANGUAGE[4];

extern const uint32_t TSC_GLOB[4];
extern const uint32_t TSC_STAGE[95];

extern const uint32_t BMP_ASCII;
extern const uint32_t BMP_KANJI;

extern const uint32_t STAGE_NAMES;
extern const uint32_t MUSIC_NAMES;
extern const uint32_t CREDITS_STR;
extern const uint32_t MENU_STR;

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
