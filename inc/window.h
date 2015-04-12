#ifndef INC_WINDOW_H_
#define INC_WINDOW_H_

#include "common.h"

// Instant text mode
#define TM_NORMAL	0
#define TM_LINE		1 // CAT/SAT
#define TM_ALL		2 // TUR

void window_open(u8 mode);

bool window_is_open();

void window_clear();

void window_close();

void window_set_face(u16 face);

void window_draw_char(u8 c);

void window_scroll_text();

void window_set_textmode(u8 mode);

bool window_tick();

void window_prompt_open();

void window_prompt_close();

bool window_prompt_answer();

bool window_prompt_update();

#endif /* INC_WINDOW_H_ */
