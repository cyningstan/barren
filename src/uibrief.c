/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Briefing Screen Module.
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

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/** @var briefing A pointer to the briefing text. */
static char *briefing;

/** @var briefingmenu The menu for a new game. */
static char *briefingmenu[] = {
    "Cancel menu",
    "Proceed",
    "New game",
    "Exit game"
};

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Allow the player to pan around the briefing map.
 */
static void briefingmap (UIScreen *uiscreen)
{
    Game *game; /* pointer to the game */
    Battle *battle; /* pointer to the battle */
    game = uiscreen->data->game;
    battle = (game->report && game->report->battle) ?
	game->report->battle :
	game->battle;
    uiscreen->scrollmap (battle,
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
    Scenario *scenario; /* current scenario */
    int loc; /* centre of friendly units */

    /* return if this player has already seen the briefing */
    game = uiscreen->data->game;
    if (game->briefed[game->battle->side])
	return;

    /* get the pointer to the appropriate briefing text */
    game->state = STATE_BRIEFING;
    scenario = game->campaign->scenarios[game->scenid];
    briefing = scenario->text[SCENARIO_BRIEFING_1 + game->battle->side];

    /* get the map location to view */
    loc = uiscreen->maplocation (game->battle);
    uiscreen->data->x = (loc % game->battle->map->width) - 4;
    uiscreen->data->y = (loc / game->battle->map->width) - 4;

    /* prepare the briefing map */
    display->preparemap (game->campaign,
			 ((game->report && game->report->battle) ?
			  game->report->battle :
			  game->battle));
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to game */
    int option, /* option chosen from the menu */
	state; /* state returned from the start screen */

    /* check if briefing is to be skipped */
    game = uiscreen->data->game;
    if (game->briefed[game->battle->side])
	return STATE_REPORT;

    /* bring up security screen */
    state = uiscreen->startscreen (game);
    if (state != STATE_BRIEFING)
	return state;

    /* initialise the display */
    display->showbriefing (briefing, game->battle->side,
	uiscreen->data->x, uiscreen->data->y);
    display->update ();

    /* main loop */
    while (1) {

	/* let the player pan around the map */
	display->prompt
	    ("Movement keys to see map, hold FIRE for menu");
	briefingmap (uiscreen);
	display->prompt ("");

	/* get a choice from the menu */
	option = display->menu (4, briefingmenu, 1);
	switch (option) {

	case 0: /* cancel menu */
	    break;

	case 1: /* proceed */
	    game->briefed[game->battle->side] = 1;
	    return STATE_REPORT;

	case 2: /* new game */
	    return STATE_NEWGAME;

	case 3: /* exit game */
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
UIScreen *new_BriefingScreen (Game *game)
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

    /* return the screen */
    return uiscreen;
}
