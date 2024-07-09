/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Computer Turn User Interface Screen Module.
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

/* project-specific headers */
#include "cwg.h"
#include "barren.h"
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "config.h"
#include "game.h"
#include "campaign.h"
#include "ai.h"

#include "debug.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct uiscreendata
 * Private data for this UI Screen.
 */
struct uiscreendata {

    /** @var game The game to set up. */
    Game *game;

    /** @var x The location shown in the map window - x coordinate. */
    int x;

    /** @var y The location shown in the map window - y coordinate. */
    int y;

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/*----------------------------------------------------------------------
 * External hooks.
 */

/**
 * Show a progress update counter on the screen.
 * @param message The text message.
 * @param counter The progress counter, 0..100.
 */
static void phaseinprogress (char *message, int counter)
{
    char output[49]; /* message and counter combined */
    sprintf (output, "%.41s (%3d%%)", message, counter);
    display->prompt (output);
}

/**
 * Show a progress update in the text log window.
 * @param message The text message.
 */
static void phasedone (char *message)
{
    display->linetext (message, 18);
    display->update ();
}

/*----------------------------------------------------------------------
 * Public Method Function Definitions.
 */

/**
 * Destroy the current UI screen when no longer needed.
 * @param uiscreen The screen to destroy.
 */
static void destroy (UIScreen *uiscreen)
{
    if (uiscreen) {
	if (uiscreen->data)
	    free (uiscreen->data);
	free (uiscreen);
    }
}

/**
 * Initialise the UI state when first encountered.
 * @param uiscreen The screen to affect.
 */
static void init (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to the game */
    int loc; /* centre of friendly units */

    /* clear the report before the computer player moves */
    game = uiscreen->data->game;
    if (game->report) {
	game->report->destroy (game->report);
	game->report = NULL;
    }
    game->briefed[game->battle->side] = 1;
    game->debriefed[game->battle->side] = 1;

    /* set the movement algorithm to smart */
    game->battle->setmovement (CWG_MOVE_SMART);

    /* get the map location to view */
    loc = uiscreen->maplocation (game->battle);
    uiscreen->data->x = (loc % game->battle->map->width) - 4;
    uiscreen->data->y = (loc / game->battle->map->width) - 4;

    /* prepare the briefing map */
    display->preparemap (game->campaign, game->battle);
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    Game *game; /* pointer to the game */
    AI *ai; /* AI module */

    /* initialise the display */
    game = uiscreen->data->game;
    display->showcomputerturn (game->battle->side,
			       uiscreen->data->x, uiscreen->data->y);
    display->update ();

    /* end the turn */
    ai = get_AI (game, phaseinprogress, phasedone);
    ai->turn ();
    ai->destroy ();
    game->turn (game);
    return game->state;
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct the Human Turn Screen.
 * @param  game The game object.
 * @return      The new Human Turn Screen.
 */
UIScreen *new_ComputerTurnScreen (Game *game)
{
    UIScreen *uiscreen; /* the screen to return */

    /* reserve memory for the screen and its data */
    uiscreen = new_UIScreen ();
    if (! (uiscreen->data = malloc (sizeof (UIScreenData)))) {
	uiscreen->destroy (uiscreen);
	return NULL;
    }

    /* initialise methods */
    uiscreen->destroy = destroy;
    uiscreen->init = init;
    uiscreen->show = show;

    /* get pointers to necessary modules */
    display = getdisplay ();
    controls = getcontrols ();
    config = getconfig ();

    /* initialise attributes */
    uiscreen->data->game = game;

    /* return the screen */
    return uiscreen;
}
