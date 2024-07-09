/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Fatal Error Handler Module.
 */

/*----------------------------------------------------------------------
 * Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project specific headers */
#include "cgalib.h"
#include "display.h"
#include "controls.h"
#include "fatal.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var messages is an array of error messages */
static char *messages[FATAL_LAST] = {
    "Unknown error",
    "Cannot create game data",
    "Invalid command line",
    "Cannot initialise display",
    "Invalid or missing game data",
    "Out of memory",
    "No campaign files found",
    "Invalid init file BARREN.INI",
    "Beta test version has expired; please contact developer"
};

/** @var screen The CGALIB screen variable. */
static Screen *screen = NULL;

/** @var display The display module. */
static Display *display = NULL;

/** @var controls The controls module. */
static Controls *controls = NULL;


/*----------------------------------------------------------------------
 * Public Functions.
 */

/**
 * Display a fatal error message and quit.
 * @param returncode is the code to return to the OS.
 */
void fatalerror (FatalReturnCode returncode)
{
    /* if we're in graphics mode, return to text mode */
    if (display)
	display->destroy ();
    if (screen)
	scr_destroy (screen);
    if (controls)
	controls->destroy ();

    /* if this is a recognised error, print the message */
    if (returncode > FATAL_NONE && returncode < FATAL_LAST)
	printf ("Error: %s.\n", messages[returncode]);
    else
	printf ("Unknown error %d.\n", returncode);
    exit (returncode);
}

/**
 * Tell the fatal error hander about the graphics display, so it knows
 * if it is necessary to clear the screen and return to text mode.
 * @param display is the graphics display, or NULL for text mode.
 */
void fataldisplay (Display *inputdisplay)
{
    display = inputdisplay;
}

/**
 * Tell the fatal error handler about a CGALIB screen. Separate from
 * the game-related graphics display, for non-game utilities like
 * mkcamp.
 * @param screen is the CGALIB screen, or NULL for text mode.
 */
void fatalscreen (Screen *inputscreen)
{
    screen = inputscreen;
}

/**
 * Tell the fatal error hander about the keyboard controls, so it knows
 * if it is necessary to restore the DOS keyboard handler before quitting.
 * @param controls The controls module, or NULL for DOS control.
 */
void fatalcontrols (Controls *inputcontrols)
{
    controls = inputcontrols;
}
