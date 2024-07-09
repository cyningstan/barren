/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Fatal Error Handler Header.
 */

#ifndef __FATAL_H__
#define __FATAL_H__

/* header dependencies */
#include "cgalib.h"
#include "display.h"
#include "controls.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @enum fatal_return_code is an OS return code */
typedef enum fatal_return_code {
    FATAL_NONE, /* no fatal error */
    FATAL_NODATA, /* cannot create game data */
    FATAL_COMMAND_LINE, /* invalid command line */
    FATAL_DISPLAY, /* cannot initialise display */
    FATAL_INVALIDDATA, /* invalid or missing game data */
    FATAL_MEMORY, /* out of memory */
    FATAL_NOCAMPAIGNS, /* no campaign files found */
    FATAL_INVALIDINIT, /* invalid init file */
    FATAL_EXPIRED, /* beta test version expired */
    FATAL_LAST /* placeholder */
} FatalReturnCode;

/*----------------------------------------------------------------------
 * Public Function Prototypes.
 */

/**
 * Display a fatal error message and quit.
 * @param returncode is the code to return to the OS.
 */
void fatalerror (FatalReturnCode returncode);

/**
 * Tell the fatal error hander about the graphics display, so it knows
 * if it is necessary to clear the screen and return to text mode.
 * @param display is the graphics display, or NULL for text mode.
 */
void fataldisplay (Display *inputdisplay);

/**
 * Tell the fatal error handler about a CGALIB screen. Separate from
 * the game-related graphics display, for non-game utilities like
 * mkcamp.
 * @param screen is the CGALIB screen, or NULL for text mode.
 */
void fatalscreen (Screen *inputscreen);

/**
 * Tell the fatal error hander about the keyboard controls, so it knows
 * if it is necessary to restore the DOS keyboard handler before quitting.
 * @param controls The controls module, or NULL for DOS control.
 */
void fatalcontrols (Controls *inputcontrols);

#endif
