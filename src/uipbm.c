/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * PBM Turn User Interface Screen Module.
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include <io.h>

/* project-specific headers */
#include "barren.h"
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "config.h"
#include "game.h"
#include "turn.h"

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

    /** @var turn The next turn loaded. */
    Turn *turn;

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

/** @var receivedmenu The menu to show when a turn file is received. */
static char *receivedmenu[] = {
    "Cancel menu",
    "Proceed",
    "New game",
    "Exit game"
};

/** @var waitmenu The menu to show when waiting for a turn file. */
static char *waitmenu[] = {
    "Cancel menu",
    "New game",
    "Exit game"
};

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Check for an inbound turn file, and load it if it exists.
 * @param uiscreen The current screen.
 */
static void checkinbound (UIScreen *uiscreen)
{
    Turn *turn; /* a new turn file */
    Game *game; /* temporary pointer to game */
    Scenario *scenario; /* temporary pointer to current scenario */
    char *ptr; /* pointer to turn file extension */

    /* create a turn object */
    if (! (turn = new_Turn ()))
	fatalerror (FATAL_MEMORY);

    /* determine the turn filename */
    game = uiscreen->data->game;
    strcpy (turn->filename, game->filename);
    if ((ptr = strchr (turn->filename, '.')))
	*ptr = '\0';
    strcat (turn->filename, ".trn");

    /* check if a turn file exists by loading the summary */
    if (! turn->load (turn, 1)) {
	turn->destroy (turn);
	return;
    }

    /* check if this is still our outbound turn */
    if (game->playertypes[turn->player] == PLAYER_PBM) {
	turn->destroy (turn);
	return;
    }

    /* check if it's the next turn */
    scenario = game->campaign->scenarios[game->scenid];
    if (turn->turnno == game->turnno &&
	game->playertypes[turn->start] == PLAYER_PBM);
    else if (turn->turnno == game->turnno + 1 &&
	     game->playertypes[turn->start] == PLAYER_HUMAN);
    else if (turn->turnno == 0 &&
	     (turn->scenid == scenario->next[0] - 1 ||
	      turn->scenid == scenario->next[1] - 1));
    else {
	turn->destroy (turn);
	return;
    }

    /* we have the inbound turn - attempt to load it in full */
    if (turn->load (turn, 0))
	uiscreen->data->turn = turn;
    else
	turn->destroy (turn);
}

/**
 * Process an inbound turn file.
 * @param uiscreen The current screen.
 */
static void processinbound (UIScreen *uiscreen)
{
    Game *game; /* pointer to the current game */
    Turn *turn; /* pointer to the loaded turn */
    char message[128]; /* formatted turn message */

    /* assign convenience pointers */
    game = uiscreen->data->game;
    turn = uiscreen->data->turn;

    /* tell the player what's going on */
    display->prompt ("Please wait...");
    sprintf (message, "Received turn file %s, processing...",
	     turn->filename);
    display->linetext (message, 18);
    display->update ();

    /* process and delete the turn file */
    turn->turntogame (turn, game);
    unlink (turn->filename);

    /* confirm done */
    display->prompt ("");
    display->linetext ("Turn file processed. "
		       "You can now take your next turn.",
		       18);
    display->update ();
}

/**
 * Generate and save an outbound turn file.
 * @param uiscreen The current screen.
 */
static void generateoutbound (UIScreen *uiscreen)
{
    Game *game; /* pointer to the current game */
    Turn *turn; /* the generated turn */
    char message[128]; /* formatted turn message */

    /* tell the player what's going on */
    display->prompt ("Please wait...");
    display->linetext ("Generating turn file for opponent...", 18);
    display->update ();

    /* attempt to generate and save the turn file */
    game = uiscreen->data->game;
    if (! (turn = new_Turn ()))
	fatalerror (FATAL_MEMORY);
    if (! turn->gametoturn (turn, game))
	strcpy (message, "Failed to generate turn file!");
    else if (! turn->save (turn))
	strcpy (message, "Failed to save turn file!");
    else
	sprintf (message, "Turn file %s ready to send!", turn->filename);

    /* post-generation message */
    display->prompt ("");
    display->linetext (message, 18);
    display->update ();
}

/**
 * Allow the player to pan around the game map.
 */
static void pbmmap (UIScreen *uiscreen)
{
    Map *map; /* current map */
    int left, /* left control pressed */
	right, /* right control pressed */
	up, /* up control pressed */
	down, /* down control pressed */
	fire; /* fire control pressed */

    /* main panning loop */
    map = uiscreen->data->game->battle->map;
    do {

	/* wait for valid control event */
	controls->release (250);
	controls->wait ();
	left = controls->left ();
	right = controls->right ();
	up = controls->up ();
	down = controls->down ();
	fire = controls->fire ();

	/* pan around the map */
	if (left || right || up || down) {
	    if (uiscreen->data->x > 0 && left)
		--uiscreen->data->x;
	    else if (uiscreen->data->x < map->width - 9 && right)
		++uiscreen->data->x;
	    else if (uiscreen->data->y > 0 && up)
		--uiscreen->data->y;
	    else if (uiscreen->data->y < map->height - 9 && down)
		++uiscreen->data->y;
	    display->showmap (uiscreen->data->x, uiscreen->data->y);
	    display->update ();
	}
	
    } while (! fire);
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
	if (uiscreen->data) {
	    if (uiscreen->data->turn)
		uiscreen->data->turn->destroy (uiscreen->data->turn);
	    free (uiscreen->data);
	}
	free (uiscreen);
    }
}

/**
 * Initialise the UI state when first encountered.
 * @param uiscreen The screen to affect.
 */
static void init (UIScreen *uiscreen)
{
    int loc; /* centre of friendly units */
    Game *game; /* pointer to game object */

    /* check for an inbound turn file and load it */
    checkinbound (uiscreen);

    /* get the map location to view */
    game = uiscreen->data->game;
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
    Game *game; /* pointer to game */
    int option; /* option chosen from menu */

    /* initialise the display */
    game = uiscreen->data->game;
    display->showpbmturn (game->battle->side,
			  uiscreen->data->x, uiscreen->data->y);
    display->update ();

    /* deal with inbound and outbound turn files */
    if (uiscreen->data->turn)
	processinbound (uiscreen);
    else
	generateoutbound (uiscreen);

    /* main loop */
    while (1) {

	/* let the player pan around the map */
	if (uiscreen->data->turn)
	    display->prompt ("Press FIRE to take your turn");
	else
	    display->prompt ("Hold FIRE for the menu");
	pbmmap (uiscreen);
	display->prompt ("");

	/* get a choice from the menu */
	if (uiscreen->data->turn)
	    option = display->menu (4, receivedmenu, 1);
	else {
	    option = display->menu (3, waitmenu, 0);
	    option += (option > 0);
	}

	/* deal with option */
	switch (option) {
	case 0: /* cancel menu */
	    break;

	case 1: /* proceed */
	    return game->state;

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
 * Construct the Human Turn Screen.
 * @param  game The game object.
 * @return      The new Human Turn Screen.
 */
UIScreen *new_PBMTurnScreen (Game *game)
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
    uiscreen->data->turn = NULL;

    /* return the screen */
    return uiscreen;
}
