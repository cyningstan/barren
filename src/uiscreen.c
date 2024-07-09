/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * User Interface Screen Module.
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
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "timer.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var startmenu The menu for the start screen. */
static char *startmenu[] = {
    "Cancel menu",
    "Proceed",
    "New game",
    "Exit game"
};

/** @var lastplayer The last player who saw the start screen. */
static int lastplayer = -1;

/** @var lastgame The last game in which the start game was shown. */
static char lastgame[13];

/** @var confirmmenu The confirmation menu. */
static char *confirmmenu[] = {
    "No",
    "Yes"
};

/*----------------------------------------------------------------------
 * Public Method Function Definitions.
 */

/**
 * Destroy the current UI screen when no longer needed.
 * @param uiscreen The screen to destroy.
 */
static void destroy (UIScreen *uiscreen)
{
    if (uiscreen)
	free (uiscreen);
}

/**
 * Initialise the UI state when first encountered.
 * @param uiscreen The screen to affect.
 */
static void init (UIScreen *uiscreen)
{
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    return STATE_QUIT;
}

/**
 * Calculate map starting position for a player.
 * @param battle The battlefield.
 * @return       The location on which to centre the map.
 */
static int maplocation (Battle *battle)
{
    int xloc, /* x location to return */
	yloc, /* y location to return */
	u, /* unit index */
	x, /* total of x coordinates of friendly units */
	y, /* total of y coordinates of friendly units */
	n; /* number of friendly units counted */
    
    /* sum up the total of units, and their aggregate position */
    for (x = y = n = u = 0; u < CWG_UNITS; ++u)
	if (battle->units[u] &&
	    battle->units[u]->hits &&
	    battle->units[u]->side == battle->side) {
	    x += battle->units[u]->x;
	    y += battle->units[u]->y;
	    ++n;
	}
    
    /* set default position in case there are no friendly units */
    if (! n) {
	xloc = 4 + battle->side * (battle->map->width - 9);
	yloc = 4 + (battle->map->height - 9) / 2;
    }

    /* work out an average position from the aggregate position */
    else {

	/* calculate average position */
	xloc = (n / 2 + x) / n;
	yloc = (n / 2 + y) / n;

	/* ensure we're not looking off the left of right of the map */
	if (xloc < 4)
	    xloc = 4;
	else if (xloc > battle->map->width - 5)
	    xloc = battle->map->width - 5;

	/* ensure we're not looking off the top or bottom of the map */
	if (yloc < 4)
	    yloc = 4;
	else if (yloc > battle->map->height - 5)
	    yloc = battle->map->height - 5;

    }

    /* return the calculated position */
    return xloc + battle->map->width * yloc;
}

/**
 * Player turn start screen, for human-vs-human games.
 * @param  game The game being played.
 * @return      The state implied by the menu.
 */
static UIState startscreen (Game *game)
{
    char message[49]; /* prompt message */
    Timer *timer; /* timer object */

    /* return if the start screen has already been shown. */
    if (lastplayer == game->battle->side &&
	! strcmp (game->filename, lastgame))
	return game->state;
    if (game->playertypes[1 - game->battle->side] != PLAYER_HUMAN)
	return game->state;
    lastplayer = game->battle->side;
    strcpy (lastgame, game->filename);

    /* show the screen and show the prompt */
    display->showstartturn (game->battle->side);
    display->update ();

    /* noise and delay */
    timer = new_Timer (250);
    display->playsound (DISPLAY_COMMS);
    timer->destroy (timer);

    /* main menu loop */
    sprintf (message, "%s, it is your turn.",
	     game->campaign->corpnames[game->battle->side]);
    display->prompt (message);
    do {
	while (controls->fire ());
	while (! controls->fire ());
	switch (display->menu (4, startmenu, 1)) {
	case 0: /* Cancel menu */
	    break;
	case 1: /* Proceed */
	    return game->state;
	case 2: /* New game */
	    return STATE_NEWGAME;
	case 3: /* Exit game */
	    return STATE_QUIT;
	}
    } while (1);
}

/**
 * Display a scrolling map without a cursor.
 * @param battle The battlefield.
 * @param x      Pointer to the leftmost coordinate.
 * @param y      Pointer to the topmost coordinate.
 */
static void scrollmap (Battle *battle, int *x, int *y)
{
    Map *map; /* current map */
    int left, /* left control pressed */
	right, /* right control pressed */
	up, /* up control pressed */
	down, /* down control pressed */
	fire; /* fire control pressed */

    /* main panning loop */
    map = battle->map;
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
	    if (*x > 0 && left)
		--*x;
	    else if (*x < map->width - 9 && right)
		++*x;
	    else if (*y > 0 && up)
		--*y;
	    else if (*y < map->height - 9 && down)
		++*y;
	    display->showmap (*x, *y);
	    display->update ();
	}
	
    } while (! fire);
}

/**
 * Confirm a potentially destructive action.
 * @param  message The message to display.
 * @return         1 if yes, 2 if no.
 */
static int confirm (char *message)
{
    display->prompt (message);
    return display->menu (2, confirmmenu, 0);
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct a new generic UI screen. This won't work on its own but
 * sets up a few elements common to multiple user interface screens.It
 * will usually be called by one of the subclass constructors.
 */
UIScreen *new_UIScreen (void)
{
    UIScreen *uiscreen; /* the screen to return */

    /* reserve memory for the screen and its data */
    if (! (uiscreen = malloc (sizeof (UIScreen))))
	fatalerror (FATAL_MEMORY);

    /* initialise methods */
    uiscreen->destroy = destroy;
    uiscreen->init = init;
    uiscreen->show = show;
    uiscreen->maplocation = maplocation;
    uiscreen->startscreen = startscreen;
    uiscreen->scrollmap = scrollmap;
    uiscreen->confirm = confirm;

    /* get pointers to necessary modules */
    display = getdisplay ();
    controls = getcontrols ();

    /* return the screen */
    return uiscreen;
}
