/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Main Program Module.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* project-specific headers */
#include "barren.h"
#include "controls.h"
#include "display.h"
#include "fatal.h"
#include "config.h"
#include "game.h"
#include "uiscreen.h"

/*----------------------------------------------------------------------
 * Data Definitions
 */

/**
 * @var colourset is the colour set:
 * 0 for 640x200 black and white dithered;
 * 1 for 320x200 black, cyan, magenta, white;
 * 2 for 320x200 dark grey, cyan, red, white.
 */
static int colourset = 2;

/** @var quiet Game is silent if 1, or has sound and music if 1. */
static int quiet = 0;

/** @var controls The controls object. */
static Controls *controls = NULL;

/** @var display is the display object. */
static Display *display = NULL;

/** @var game is a loaded game. */
static Game *game = NULL;

/** @var config Pointer to the configuration. */
static Config *config = NULL;

/** @var state The current unsaved game state. */
static int state;

/*----------------------------------------------------------------------
 * Level 2 Routines
 */

/**
 * Read through the argument list for recognised parameters.
 * @param argc is the argument count from the command line.
 * @param argv is an array of arguments.
 */
static void initialiseargs (int argc, char **argv)
{
    /* local variables */
    int c; /* argument count */

    /* scan through arguments */
    for (c = 1; c < argc; ++c)
	if (! strcmp (argv[c], "-p"))
	    colourset = 1;
	else if (! strcmp (argv[c], "-m"))
	    colourset = 0;
	else if (! strcmp (argv[c], "-q"))
	    quiet = 1;
	else
	    fatalerror (FATAL_COMMAND_LINE);
}

/*----------------------------------------------------------------------
 * Level 1 Routines
 */

/**
 * Initialise the program.
 * @param argc is the argument count from the command line.
 * @param argv is the array of arguments from the command line.
 */
static void initialiseprogram (int argc, char **argv)
{
    /* check command line */
    initialiseargs (argc, argv);

    /* initialise the random number generator */
    srand (time (NULL));

    /* initialise controls */
    if (! (controls = new_Controls ()))
	fatalerror (FATAL_DISPLAY);

    /* initialise the display and assets */
    if (! (display = new_Display (colourset, quiet)))
	fatalerror (FATAL_DISPLAY);
    display->showtitlescreen ();

    /* initialise configuration */
    config = new_Config ();
    config->load ();

    /* initialise the game object */
    game = new_Game ();
    if (*config->gamefile) {
	strcpy (game->filename, config->gamefile);
	if (game->load (game, 0))
	    display->setgame (game);
	else {
	    game->clear (game);
	    *config->gamefile = '\0';
	}
    }
    state = game->state;

    /* await the fire key */
    display->titlekey ();
}

/**
 * The main game loop selects a screen to activate.
 */
static int playgame (void)
{
    UIScreen *uiscreen; /* user interface screen to present */

    /* build the screen */
    switch (state) {
    case STATE_NEWGAME:
	uiscreen = new_NewGameScreen (game);
	break;
    case STATE_BRIEFING:
	uiscreen = new_BriefingScreen (game);
	break;
    case STATE_REPORT:
	uiscreen = new_TurnReportScreen (game);
	break;
    case STATE_HUMAN:
	uiscreen = new_HumanTurnScreen (game);
	break;
    case STATE_COMPUTER:
	uiscreen = new_ComputerTurnScreen (game);
	break;
    case STATE_PBM:
	uiscreen = new_PBMTurnScreen (game);
	break;
    case STATE_DEBRIEFING:
	uiscreen = new_DebriefingScreen (game);
	break;
    case STATE_ENDGAME:
	uiscreen = new_EndGameScreen (game);
	break;
    default:
	return 0;	
    }

    /* show the screen */
    uiscreen->init (uiscreen);
    state = uiscreen->show (uiscreen);
    uiscreen->destroy (uiscreen);

    /* return */
    return (state != STATE_QUIT);
}

/**
 * Clean up when the user quits normally.
 */
static void endprogram (void)
{
    Cwg *cwg;
    config->save ();
    if (game) {
	if (*game->filename)
	    game->save (game);
	game->destroy (game);
    }
    display->destroy ();
    controls->destroy ();
    config->destroy ();
    if ((cwg = get_Cwg ()))
        cwg->destroy ();
}

/*----------------------------------------------------------------------
 * Public Level Routines.
 */

/**
 * Share the control handler with other modules.
 * @return A pointer to the game control handler.
 */
Controls *getcontrols (void)
{
    return controls;
}

/**
 * Share the display handler with other modules.
 * @return A pointer to the display module.
 */
Display *getdisplay (void)
{
    return display;
}

/**
 * Share the config handler with other modules.
 * @return A pointer to the config module.
 */
Config *getconfig (void)
{
    return config;
}

/**
 * Main Program.
 * @param argc is the number of command line arguments.
 * @param argv is an array of command line arguments.
 * @return 0 if successful, >0 on error.
 */
int main (int argc, char **argv)
{
    initialiseprogram (argc, argv);
    while (playgame ());
    endprogram ();
    return 0;
}
