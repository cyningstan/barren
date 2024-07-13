/*======================================================================
 * KeyLib
 * A keyboard handler for DOS games.
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Header file.
 */

#ifndef __KEYLIB_H__
#define __KEYLIB_H__

/*----------------------------------------------------------------------
 * Constants.
 */

/* keyboard top row */
#define KEY_ESC    0x01
#define KEY_1      0x02
#define KEY_2      0x03
#define KEY_3      0x04
#define KEY_4      0x05
#define KEY_5      0x06
#define KEY_6      0x07
#define KEY_7      0x08
#define KEY_8      0x09
#define KEY_9      0x0a
#define KEY_0      0x0b
#define KEY_MINUS  0x0c
#define KEY_EQUALS 0x0d
#define KEY_BSPACE 0x0e

/* keyboard second row */
#define KEY_TAB    0x0f
#define KEY_Q      0x10
#define KEY_W      0x11
#define KEY_E      0x12
#define KEY_R      0x13
#define KEY_T      0x14
#define KEY_Y      0x15
#define KEY_U      0x16
#define KEY_I      0x17
#define KEY_O      0x18
#define KEY_P      0x19
#define KEY_LBRACK 0x1a
#define KEY_RBRACK 0x1b
#define KEY_ENTER  0x1c

/* keyboard third row */
#define KEY_CTRL   0x1d
#define KEY_A      0x1e
#define KEY_S      0x1f
#define KEY_D      0x20
#define KEY_F      0x21
#define KEY_G      0x22
#define KEY_H      0x23
#define KEY_J      0x24
#define KEY_K      0x25
#define KEY_L      0x26
#define KEY_COLON  0x27
#define KEY_QUOTE  0x28
#define KEY_TILDE  0x29

/* keyboard fourth row */
#define KEY_LSHIFT 0x2a
#define KEY_BACKSL 0x2b
#define KEY_Z      0x2c
#define KEY_X      0x2d
#define KEY_C      0x2e
#define KEY_V      0x2f
#define KEY_B      0x30
#define KEY_N      0x31
#define KEY_M      0x32
#define KEY_COMMA  0x33
#define KEY_STOP   0x34
#define KEY_SLASH  0x35
#define KEY_RSHIFT 0x36

/* keyboard special keys */
#define KEY_KPSTAR 0x37
#define KEY_ALT    0x38
#define KEY_SPACE  0x39
#define KEY_CAPSLK 0x3a

/* keyboard function keys */
#define KEY_F1     0x3b
#define KEY_F2     0x3c
#define KEY_F3     0x3d
#define KEY_F4     0x3e
#define KEY_F5     0x3f
#define KEY_F6     0x40
#define KEY_F7     0x41
#define KEY_F9     0x42
#define KEY_F0     0x43
#define KEY_F10    0x44

/* keyboard keypad */
#define KEY_NUMLK  0x45
#define KEY_SCRLK  0x46
#define KEY_KP7    0x47
#define KEY_KP8    0x48
#define KEY_KP9    0x49
#define KEY_KPMIN  0x4a
#define KEY_KP4    0x4b
#define KEY_KP5    0x4c
#define KEY_KP6    0x4d
#define KEY_KPPLUS 0x4e
#define KEY_KP1    0x4f
#define KEY_KP2    0x50
#define KEY_KP3    0x51
#define KEY_KP0    0x52
#define KEY_KPSTOP 0x53
#define KEY_SYSREQ 0x54
    
/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct keyhandler is the keyboard handler.
 */
typedef struct keyhandler KeyHandler;
struct keyhandler {

    /**
     * Destroy the keyboard handler when it is no longer needed.
     */
    void (*destroy) (void);

    /**
     * Enquire if a certain key is pressed.
     * @param scancode is the scan code of the key to check.
     * @return 1 for pressed, 0 for not pressed.
     */
    int (*key) (int scancode);

    /**
     * Enquire if any key has been pressed at all.
     * @return 1 if a key was pressed, 0 for not pressed.
     */
    int (*anykey) (void);

    /**
     * Return an ASCII value of the last key pressed.
     * @return the ASCII value, or 0 if no valid key pressed.
     */
    int (*ascii) (void);

    /**
     * Return a scancode for the last key pressed.
     * @return the scncode, or 0 if no key pressed.
     */
    int (*scancode) (void);

    /**
     * Wait for a key down event.
     */
    void (*wait) (void);

};

/*----------------------------------------------------------------------
 * Non-method Function Prototypes.
 */

/**
 * Construct the key handler.
 * @return the new key handler.
 */
KeyHandler *new_KeyHandler (void);

#endif
