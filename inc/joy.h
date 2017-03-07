/**
 *  \file joy.h
 *  \brief General controller support.
 *  \author Chilly Willy & Stephane Dallongeville
 *  \date 05/2012
 *
 * This unit provides methods to read controller state.<br>
 */

#define PORT_1          0x0000
#define PORT_2          0x0001

#define JOY_1           0x0000
#define JOY_2           0x0001
#define JOY_NUM         0x0002
#define JOY_ALL         0xFFFF

#define BUTTON_UP       0x0001
#define BUTTON_DOWN     0x0002
#define BUTTON_LEFT     0x0004
#define BUTTON_RIGHT    0x0008
#define BUTTON_A        0x0040
#define BUTTON_B        0x0010
#define BUTTON_C        0x0020
#define BUTTON_START    0x0080
#define BUTTON_X        0x0400
#define BUTTON_Y        0x0200
#define BUTTON_Z        0x0100
#define BUTTON_MODE     0x0800

#define BUTTON_DIR      0x000F
#define BUTTON_BTN      0x0FF0
#define BUTTON_ALL      0x0FFF

#define JOY_TYPE_PAD3           0x00
#define JOY_TYPE_PAD6           0x01
#define JOY_TYPE_UNKNOWN        0x0F

void JOY_init();
uint8_t JOY_getJoypadType(uint16_t joy);
uint16_t  JOY_readJoypad(uint16_t joy);
void JOY_update();
