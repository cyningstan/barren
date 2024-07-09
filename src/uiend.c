/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * End Game User Interface Module.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "cwg.h"
#include "barren.h"
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "game.h"
#include "campaign.h"
#include "scenario.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct uiscreendata
 * Private data for this UI Screen.
 */
struct uiscreendata {

    /** @var game The game being played. */
    Game *game;

    /** @var x The location shown in the map window - x coordinate. */
    int x;

    /** @var y The location shown in the map window - y coordinate. */
    int y;

    /** @var victor The winner of the game. */
    int victor;

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/** @var briefingmenu The menu for a new game. */
static char *endgamemenu[] = {
    "Cancel menu",
    "New game",
    "Exit game"
};

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Allow the player to pan around the end game map.
 */
static void endgamemap (UIScreen *uiscreen)
{
    uiscreen->scrollmap (uiscreen->data->game->battle,
			 &uiscreen->data->x,
			 &uiscreen->data->y);
}

/*----------------------------------------------------------------------
 * Public Method Function Declarations.
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
    /* local variables */
    Game *game; /* current game */
    int loc; /* centre of friendly units */

    /* get the pointer to the appropriate briefing text */
    game = uiscreen->data->game;
    game->state = STATE_ENDGAME;
    uiscreen->data->victor = game->battle->victory (game->battle);

    /* get the map location to view */
    loc = uiscreen->maplocation (game->battle);
    uiscreen->data->x = (loc % game->battle->map->width) - 4;
    uiscreen->data->y = (loc / game->battle->map->width) - 4;

    /* prepare the debriefing map */
    display->preparemap (game->campaign, game->battle);
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to game */
    int option; /* option chosen from the menu */

    /* initialise the display */
    game = uiscreen->data->game;
    display->showendgame
	(uiscreen->data->victor,
	 uiscreen->data->x, uiscreen->data->y);
    display->update ();

    /* main loop */
    while (1) {

	/* let the player pan around the map */
	display->prompt
	    ("Tap FIRE for a new game.");
	endgamemap (uiscreen);
	display->prompt ("");

	/* get a choice from the menu */
	option = display->menu (3, endgamemenu, 1);
	switch (option) {

	case 0: /* cancel menu */
	    break;

	case 1: /* new game */
	    return STATE_NEWGAME;

	case 2: /* exit game */
	    return STATE_QUIT;
	}
    }
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct the Briefing Screen.
 * @param  game The game object.
 * @return      The new Briefing Screen.
 */
UIScreen *new_EndGameScreen (Game *game)
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
    uiscreen->data->x = 0;
    uiscreen->data->y = 0;
    uiscreen->data->victor = -1;

    /* return the screen */
    return uiscreen;
}
