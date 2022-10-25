/*
 * The message window is drawn to the bottom of the window plane
 * The text uses the same plane as well rather than sprites
 */
 
#define WINDOW_ATTR(x) TILE_ATTR(PAL0, 1, 0, 0, TILE_WINDOWINDEX+(x))

// Instant text mode
#define TM_NORMAL	0
#define TM_MSG		1 // TUR
#define TM_ALL		2 // CAT/SAT

//uint8_t linesSinceLastNOD;
extern const uint8_t ITEM_PAL[40];

extern uint8_t windowOnTop;

extern uint16_t showingFace;

// Opens the message window
void window_open(uint8_t mode);
// Returns TRUE if the message window is open
uint8_t window_is_open();
// Clears the text/face to plain blue
void window_clear();
// Closes the message window
void window_close();
// Sets the face to draw during the next message (0 erases the face)
// The open parameter will force the window open when TRUE
void window_set_face(uint16_t face, uint8_t open);
// Draws a single letter into the window, '\n' will do a new line
void window_draw_char(uint8_t c);
// Draws a 16x16 letter or ideograph
void window_draw_jchar(uint8_t iskanji, uint16_t c);
// Moves the 2nd and 3rd text row to the top so dialog can continue
void window_scroll_text();

void window_scroll_jtext();
// Changes the text mode (show next line immediately or whole message)
uint8_t window_get_textmode();
void window_set_textmode(uint8_t mode);
// Counts down until the next letter is to be printed, and will return TRUE
// when the counter resets
uint8_t window_tick();
// Opens the Yes/No prompt
void window_prompt_open();
// Closes the Yes/No prompt
void window_prompt_close();
// Gets whether the player picked Yes (TRUE) or No (FALSE)
uint8_t window_prompt_answer();
// Updates the prompt according to player input if open
// Returns TRUE if the player picked an option
uint8_t window_prompt_update();
// Displays small window under the center of the screen that shows an item's image
void window_show_item(uint16_t item);
// Same as above but will show a weapon's image
void window_show_weapon(uint16_t weapon);

void window_update();
