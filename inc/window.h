#ifndef INC_WINDOW_H_
#define INC_WINDOW_H_

#include "common.h"

/*
 * The message window is drawn to the bottom of the window plane
 * The text uses the same plane as well rather than sprites
 */

// Instant text mode
#define TM_NORMAL	0
#define TM_LINE		1 // CAT/SAT
#define TM_ALL		2 // TUR

extern const u8 ITEM_PAL[40];

// Opens the message window
void window_open(u8 mode);
// Returns TRUE if the message window is open
u8 window_is_open();
// Clears the text/face to plain blue
void window_clear();
// Closes the message window
void window_close();
// Sets the face to draw during the next message (0 erases the face)
// The open parameter will force the window open when TRUE
void window_set_face(u16 face, u8 open);
// Draws a single letter into the window, '\n' will do a new line
void window_draw_char(u8 c);
// Moves the 2nd and 3rd text row to the top so dialog can continue
void window_scroll_text();
// Changes the text mode (show next line immediately or whole message)
void window_set_textmode(u8 mode);
// Counts down until the next letter is to be printed, and will return TRUE
// when the counter resets
u8 window_tick();
// Opens the Yes/No prompt
void window_prompt_open();
// Closes the Yes/No prompt
void window_prompt_close();
// Gets whether the player picked Yes (TRUE) or No (FALSE)
u8 window_prompt_answer();
// Updates the prompt according to player input if open
// Returns TRUE if the player picked an option
u8 window_prompt_update();
// Displays small window under the center of the screen that shows an item's image
void window_show_item(u16 item);
// Same as above but will show a weapon's image
void window_show_weapon(u16 weapon);

void window_update();

#endif /* INC_WINDOW_H_ */
