/*======================================================================
 * KeyLib
 * A keyboard handler for DOS games.
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Module file.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* compiler headers */
#include <dos.h>
#include <conio.h>

/* project headers */
#include "keylib.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var keys is the keyboard handler object */
static KeyHandler *keys = NULL;

/** @var bioskb is the bios keyboard routine. */
static void (interrupt *bioskb) (void);

/** @var keystates records the up (0) or down (1) state of each key */
static int keystates[0x80];

/** @var keydown is the scancode of the last key pressed (or 0) */
static int keydown;

/** @var scancode-to-ascii input table */
static int uskeys[][3] =
{
    /* above top row */
    {KEY_ESC, 27, 27},

    /* top row */
    {KEY_TILDE, '`', '~'},
    {KEY_1, '1', '!'},
    {KEY_2, '2', '@'},
    {KEY_3, '3', '#'},
    {KEY_4, '4', '$'},
    {KEY_5, '5', '%'},
    {KEY_6, '6', '^'},
    {KEY_7, '7', '&'},
    {KEY_8, '8', '*'},
    {KEY_9, '9', '('},
    {KEY_0, '0', ')'},
    {KEY_BSPACE, 8, 8},

    /* second row */
    {KEY_TAB, 9, 0},
    {KEY_Q, 'q', 'Q'},
    {KEY_W, 'w', 'W'},
    {KEY_E, 'e', 'E'},
    {KEY_R, 'r', 'R'},
    {KEY_T, 't', 'T'},
    {KEY_Y, 'y', 'Y'},
    {KEY_U, 'u', 'U'},
    {KEY_I, 'i', 'I'},
    {KEY_O, 'o', 'O'},
    {KEY_P, 'p', 'P'},
    {KEY_LBRACK, '[', '{'},
    {KEY_RBRACK, ']', '}'},
    {KEY_BACKSL, '\\', '|'},

    /* third row */
    {KEY_A, 'a', 'A'},
    {KEY_S, 's', 'S'},
    {KEY_D, 'd', 'D'},
    {KEY_F, 'f', 'F'},
    {KEY_G, 'g', 'G'},
    {KEY_H, 'h', 'H'},
    {KEY_J, 'j', 'J'},
    {KEY_K, 'k', 'K'},
    {KEY_L, 'l', 'L'},
    {KEY_COLON, ';', ':'},
    {KEY_QUOTE, '\'', '"'},
    {KEY_ENTER, 13, 13},

    /* fourth row */
    {KEY_Z, 'z', 'Z'},
    {KEY_X, 'x', 'X'},
    {KEY_C, 'c', 'C'},
    {KEY_V, 'v', 'V'},
    {KEY_B, 'b', 'B'},
    {KEY_N, 'n', 'N'},
    {KEY_M, 'm', 'M'},
    {KEY_COMMA, ',', '<'},
    {KEY_STOP, '.', '>'},
    {KEY_SLASH, '/', '?'},

    /* below fourth row */
    {KEY_SPACE, ' ', ' '},

    /* numeric keypad */
    {KEY_KPSTAR, '*', '*'},
    {KEY_KP7, 0, '7'},
    {KEY_KP8, 11, '8'},
    {KEY_KP9, 0, '9'},
    {KEY_KPMIN, '-', '-'},
    {KEY_KP4, 8, '4'},
    {KEY_KP5, 0, '5'},
    {KEY_KP6, 9, '6'},
    {KEY_KPPLUS, '+', '+'},
    {KEY_KP1, 0, '1'},
    {KEY_KP2, 10, '2'},
    {KEY_KP3, 0, '3'},
    {KEY_KP0, 0, '0'},
    {KEY_KPSTOP, 127, '.'},

    /* end of list */
    {0, 0, 0}
};

/** @var scantoascii is a table of scan codes to ASCII. */
static int scantoascii[0x80][2];

/*----------------------------------------------------------------------
 * Private Level Functions.
 */

/**
 * Game keyboard handler.
 */
void interrupt gamekb (void)
{
    /* local variables */
    int ack, /* acknowledgement byte */
        keycode; /* key up/down code */

    /* get key code and acknowledgement byte */
    keycode = inp (0x60);
    ack = inp (0x61);

    /* send acknowledgement */
    outp (0x61, ack | 0x80);
    outp (0x61, ack & 0x7f);
    outp (0x20, 0x20);

    /* process key release */
    if (keycode & 0x80)
	keystates[keycode & 0x7f] = 0;

    /* process key press */
    else {
	keystates[keycode] = 1;
	keydown = keycode;
    }
}

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Destroy the keyboard handler when it is no longer needed.
 */
static void destroy (void)
{
    _dos_setvect (9, bioskb);
    free (keys);
    keys = NULL;
}

/**
 * Enquire if a certain key is pressed.
 * @param scancode is the scan code of the key to check.
 * @return 1 for pressed, 0 for not pressed.
 */
static int key (int scancode)
{
    return keystates[scancode];
}

/**
 * Enquire if any key has been pressed at all.
 * @return 1 if a key was pressed, 0 for not pressed.
 */
static int anykey (void)
{
    return keydown != 0;
}

/**
 * Return an ASCII value of the last key pressed.
 * @return the ASCII value, or 0 if no valid key pressed.
 */
static int ascii (void)
{
    int shift, /* the state of the shift keys */
	ch; /* ascii code of key pressed */

    /* return if no keys pressed since last call */
    if (! keydown)
	return 0;

    /* check if shift is pressed */
    shift = keystates[0x2a] | keystates[0x36];

    /* scan the codes for each character */
    if ((ch = scantoascii[keydown][shift])) {
	keydown = 0;
	return ch;
    }

    /* no ASCII key identified */
    return 0;
}

/**
 * Return a scancode for the last key pressed.
 * @return the scncode, or 0 if no key pressed.
 */
static int scancode (void)
{
    int lastkey; /* the last key pressed */
    lastkey = keydown;
    keydown = 0;
    return lastkey;
}

/**
 * Wait for a key down event.
 */
static void wait (void)
{
    do {} while (! keydown);
}

/*----------------------------------------------------------------------
 * Non-method Functions.
 */

/**
 * Construct the key handler.
 * @return the new key handler.
 */
KeyHandler *new_KeyHandler (void)
{
    int c; /* keyboard table counter */

    /* make sure the handler isn't already active */
    if (keys)
	return keys;

    /* attempt to reserve memory */
    if (! (keys = malloc (sizeof (KeyHandler))))
	return NULL;

    /* initialise methods */
    keys->destroy = destroy;
    keys->key = key;
    keys->anykey = anykey;
    keys->ascii = ascii;
    keys->scancode = scancode;
    keys->wait = wait;

    /* initialise the scantoascii table */
    for (c = 0; uskeys[c][0] != 0; ++c) {
	scantoascii[uskeys[c][0]][0] = uskeys[c][1];
	scantoascii[uskeys[c][0]][1] = uskeys[c][2];
    }

    /* return the new keyboard handler */
    bioskb = _dos_getvect (9);
    _dos_setvect (9, gamekb);
    return keys;
}
