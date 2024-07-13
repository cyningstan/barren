/*======================================================================
 * KeyLib
 * A keyboard handler for DOS games.
 *
 * Copyright (C) Damian Gareth Walker 2021.
 *
 * Demonstration Program.
 */

/*----------------------------------------------------------------------
 * Included headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "keylib.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var keys is the keyboard handler. */
static KeyHandler *keys;

/*----------------------------------------------------------------------
 * Level 1 Functions.
 */

/**
 * Check for the enter key 3 times by scancode.
 */
static void enterscancode (void)
{
    int c; /* counter */
    printf ("Press ENTER 3 times to return to menu.\n");
    for (c = 1; c <= 3; ++c) {
	while (! keys->key (KEY_ENTER));
	printf ("ENTER press %d...\n", c);
	while (keys->key (KEY_ENTER));
    }
}

/**
 * Check for the enter key 3 times by ASCII.
 */
static void enterascii (void)
{
    int c; /* counter */
    printf ("Press ENTER 3 times to return to menu.\n");
    for (c = 1; c <= 3; ++c) {
	while (keys->ascii () != 0x0d);
	printf ("ENTER press %d...\n", c);
    }
}

/**
 * Enter a line of text.
 */
static void enterline (void)
{
    int ch; /* character */
    printf ("Enter a line of text:\n");
    do {
	keys->wait ();
	ch = keys->ascii ();
	if (ch) {
	    printf ("%c", ch);
	    fflush (stdout);
	}
    } while (ch != 0x0d);
    printf ("\n");
}

/**
 * Print out scancodes as they are pressed.
 */
static void printscancodes (void)
{
    int scancode; /* scan code of last key pressed */
    printf ("Press keys, ENTER to end.\n");
    do {
	keys->wait ();
	if ((scancode = keys->scancode ())) {
	    printf ("%02x ", scancode);
	    fflush (stdout);
	}
    } while (scancode != KEY_ENTER);
    printf ("\n");
}

/*----------------------------------------------------------------------
 * Top Level Functions.
 */

/**
 * Main program
 */
int main (void)
{
    int c; /* character pressed */

    /* attempt to initialise the keyboard handler */
    if (! (keys = new_KeyHandler ())) {
	printf ("Cannot initialise keyboard handler.\n");
	return 1;
    }

    /* Main loop */
    do {
	
	/* print the menu */
	printf ("1. Check for ENTER by scancode\n");
	printf ("2. Check for ENTER by ASCII code\n");
	printf ("3. Enter a line of text\n");
	printf ("4. Print scancodes\n");
	printf ("0. Quit the program\n");

	/* get a key */
	do {
	    keys->wait ();
	    c = keys->ascii ();
	} while (c < '0' || c > '4');

	switch (c) {
	case '1':
	    enterscancode ();
	    break;
	case '2':
	    enterascii ();
	    break;
	case '3':
	    enterline ();
	    break;
	case '4':
	    printscancodes ();
	    break;
	}

    } while (c != '0');

    keys->destroy ();
    return 0;
}
