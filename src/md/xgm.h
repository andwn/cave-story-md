#ifndef MD_XGM_H
#define MD_XGM_H

#include "types.h"

extern void xgm_init();

extern void xgm_music_play(const uint8_t *xgc);
extern void xgm_music_pause();

extern void xgm_pcm_set(uint8_t id, const uint8_t *sample, uint32_t len);
extern void xgm_pcm_play(uint8_t id, uint8_t priority, uint16_t channel);

#endif //MD_XGM_H
