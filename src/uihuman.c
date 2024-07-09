/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Human Turn User Interface Screen Module.
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

/* compiler specific headers */
#include <i86.h>

/* project-specific headers */
#include "barren.h"
#include "fatal.h"
#include "uiscreen.h"
#include "display.h"
#include "controls.h"
#include "config.h"
#include "game.h"
#include "timer.h"

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

    /** @var xcursor The x coordinate of the cursor. */
    int xcursor;

    /** @var ycursor The y coordinate of the cursor. */
    int ycursor;

    /** @var xview The x coordinate of the top left of the map view. */
    int xview;

    /** @var yview The y coordinate of the top left of the map view. */
    int yview;

    /** @var unit The ID of the last selected friendly unit. */
    int unit;

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/** @var humanmenu The menu for taking a turn. */
static char *humanmenu[] = {
    "Cancel menu",
    "Select",
    "Move",
    "Attack",
    "Build",
    "Repair",
    "Done turn",
    "New game",
    "Exit game"
};

/** @var buildmenu The menu for building a unit. */
static char *buildmenu[] = {
    "Cancel",
    "Build"
};

/** @var hookdata A pointer to the UI Screen data for CWG hooks. */
UIScreenData *hookdata;

/*----------------------------------------------------------------------
 * CWG Display Hook Service Functions.
 */

/**
 * Brief display of one unit firing at another, with sound.
 * @param origin The unit firing.
 * @param target The unit being fired at.
 */
static void showfire (Unit *origin, Unit *target)
{
    Timer *timer; /* timer object */
    display->showunitfire (origin->x, origin->y,
			   hookdata->xview, hookdata->yview);
    display->showunitblast (target->x, target->y,
			    hookdata->xview, hookdata->yview);
    display->update ();
    timer = new_Timer (250);
    display->playsound (DISPLAY_PEWPEW);
    timer->destroy (timer);
}

/**
 * Brief display of the building/repair icon, with sound.
 * @param x     The x location of the unit being built or repaired.
 * @param y     The y location of the unit being built or repaired.
 * @param build 1 for build, 0 for repair.
 */
static void showspanner (int x, int y, int build)
{
    Timer *timer; /* delay timer */
    display->showspanner (x, y, hookdata->xview, hookdata->yview);
    display->update ();
    timer = new_Timer (250);
    display->playsound (DISPLAY_HAMMER);
    timer->destroy (timer);
}

/*----------------------------------------------------------------------
 * CWG Display Hooks.
 */

/**
 * Display square-by-square unit movements.
 * @param unit The unit that has moved.
 * @param x    The x coordinate the unit moved from.
 * @param y    The y coordinate the unit moved from.
 */
static void movehook (Unit *unit, int x, int y)
{
    Game *game; /* convenience pointer to the game */
    int width; /* width of battle map */
    Report *report; /* convenience pointer to report */
    Timer *timer; /* timer object for delay */

    /* add this move to the game report */
    game = hookdata->game;
    width = game->battle->map->width;
    report = game->report;
    report->add (report, ACTION_MOVE, unit->utype, 0,
		 x + width * y, unit->x + width * unit->y);

    /* initialise the timer */
    timer = new_Timer (250);

    /* show the unit's movement */
    display->updatemap (game->campaign, game->battle, x, y);
    display->updatemap (game->campaign, game->battle, unit->x, unit->y);
    display->showmap (hookdata->xview, hookdata->yview);
    display->showunitinfo (unit);
    display->update ();

    /* allow the delay to complete */
    timer->destroy (timer);
}

/**
 * Display an attack.
 * @param attacker The unit that attacked.
 * @param defender The unit that was attacked.
 */
static void attackhook (Unit *attacker, Unit *defender,
			CwgAttackResult result)
{
    char message[128]; /* attack message */
    Game *game; /* convenience pointer to game */
    Report *report; /* convenience pointer to report */
    int width; /* width of the battle map */
    Timer *timer; /* delay timer object */

    /* add this attack to the game report */
    game = hookdata->game;
    width = game->battle->map->width;
    report = game->report;
    report->add (report, ACTION_ATTACK + result,
		 attacker->utype, defender->utype,
		 attacker->x + width * attacker->y,
		 defender->x + width * defender->y); 

    /* show an attack, and the return fire if appropriate */
    showfire (attacker, defender);
    if (result & CWG_ATT_RETURN_FIRE)
	showfire (defender, attacker);

    /* print the report message */
    sprintf (message, "%s %s %s %s %s",
	     hookdata->game->campaign->corpnames[attacker->side],
	     attacker->name,
	     defender->hits ? "attacks" : "destroys",
	     hookdata->game->campaign->corpnames[defender->side],
	     defender->name);
    strcat (message, attacker->hits ? "." : " and is destroyed!");
    display->linetext (message, 12);

    /* show the updated unit info and update the display */
    display->showunitinfo (attacker);
    display->update ();

    /* show the aftermath */
    display->updatemap (hookdata->game->campaign,
			hookdata->game->battle,
			attacker->x, attacker->y);
    display->updatemap (hookdata->game->campaign,
			hookdata->game->battle,
			defender->x, defender->y);
    display->showmap (hookdata->xview, hookdata->yview);
    display->showunitinfo (attacker);
    display->update ();

    /* explosion noise if one of the units is destroyed */
    if (attacker->hits == 0 || defender->hits == 0) {
	timer = new_Timer (250);
	display->playsound (DISPLAY_EXPLOSION);
	timer->destroy (timer);
    }
}

/**
 * Display a built unit.
 * @param builder The unit that is building.
 * @param built   The unit that was built.
 */
static void buildhook (Unit *builder, Unit *built)
{
    char message[128]; /* attack message */
    Game *game; /* convenience pointer to game */
    Report *report; /* convenience pointer to report */
    int width; /* width of the battle map */

    /* add this build to the game report */
    game = hookdata->game;
    width = game->battle->map->width;
    report = game->report;
    report->add (report, ACTION_BUILD,
		 builder->utype, built->utype,
		 builder->x + width * builder->y,
		 built->x + width * built->y);

    /* name the built unit */
    strcpy (built->name, game->campaign->unittypes[built->utype]->name);

    /* show an animation */
    showspanner (built->x, built->y, 1);

    /* print the report message */
    sprintf (message, "%s %s builds %s. %d resources remaining.",
	     game->campaign->corpnames[builder->side],
	     builder->name,
	     built->name,
	     game->battle->resources[builder->side]);
    display->linetext (message, 12);

    /* show the updated unit info and update the display */
    display->showresources (game->battle->resources[builder->side]);
    display->updatemap (hookdata->game->campaign,
			hookdata->game->battle,
			builder->x, builder->y);
    display->updatemap (hookdata->game->campaign,
			hookdata->game->battle,
			built->x, built->y);
    display->showmap (hookdata->xview, hookdata->yview);
    display->showunitinfo (builder);
    display->showmapcursor (hookdata->xcursor, hookdata->ycursor,
			    hookdata->xview, hookdata->yview);
    display->update ();
}

/**
 * Display a repair.
 * @param builder The unit that can build/repair.
 * @param built   The unit that was repaired.
 */
static void repairhook (Unit *builder, Unit *built)
{
    char message[128]; /* attack message */
    Game *game; /* convenience pointer to game */
    Report *report; /* convenience pointer to report */
    int width; /* width of the battle map */

    /* add this repair to the game report */
    game = hookdata->game;
    width = game->battle->map->width;
    report = game->report;
    report->add (report, ACTION_REPAIR,
		 builder->utype, built->utype,
		 builder->x + width * builder->y,
		 built->x + width * built->y);

    /* show an animation */
    showspanner (built->x, built->y, 0);

    /* print the report message */
    sprintf (message, "%s %s repairs %s. %d resources remaining.",
	     game->campaign->corpnames[builder->side],
	     builder->name,
	     built->name,
	     game->battle->resources[builder->side]);
    display->linetext (message, 12);

    /* show the updated unit info and update the display */
    display->showresources (game->battle->resources[builder->side]);
    display->showunitinfo (builder);
    display->update ();
}

/*----------------------------------------------------------------------
 * Level 2 Private Function Definitions.
 */

/**
 * Ask the user for a type of unit to build.
 * @param uiscreen The user interface screen data.
 * @param bcount   The number of unit types that can be built.
 * @param btypes   A list of unit types that can be built (by id)
 * @return         The unit type selected, or -1 if cancelled.
 */
int getbuildtype (UIScreen *uiscreen, int bcount, int *btypes)
{
    int first = 0, /* first unit type in selector */
	current = 0, /* current highlighted unit type */
	option = -1, /* option chosen from build menu */
	u; /* current unit */

    /* initialise the display */
    display->showunittypes (bcount, btypes, first);

    /* main interface loop */
    do {

	/* show the cursor and await a keypress */
	display->showunittypecursor (bcount, btypes, first, current);
	display->update ();
	controls->release (0);
	controls->wait ();
	display->hideunittypecursor (bcount, btypes, first, current);

	/* left pressed? */
	if (controls->left () && current > 0) {
	    --current;
	    if (current < first) {
		first = current;
		display->updateunittypes (bcount, btypes, first);
	    }
	}

	/* right pressed? */
	else if (controls->right () && current < bcount - 1) {
	    ++current;
	    if (current > first + 4) {
		first = current - 4;
		display->updateunittypes (bcount, btypes, first);
	    }
	}

	/* FIRE pressed? */
	else if (controls->fire ())
	    option = display->menu (2, buildmenu, 1);

    } while (option == -1);

    /* restore the builder unit type display */
    u = uiscreen->data->unit;
    display->showunitinfo (uiscreen->data->game->battle->units[u]);
    display->update ();

    /* return the chosen unit */
    return option ? btypes[current] : -1;
}

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions.
 */

/**
 * Show the resources at the start of a turn.
 * @param battle The current battle.
 */
static void showresources (Battle *battle)
{
    char message[64];
    sprintf (message,
	     "Resources available: %d",
	     battle->resources[battle->side]);
    display->linetext (message, 12);
    display->showresources (battle->resources[battle->side]);
    display->update ();
}

/**
 * Allow the player to navigate the map.
 * @param uiscreen The user interface screen object.
 */
static void navigatemap (UIScreen *uiscreen)
{
    Map *map; /* current map */
    int left, /* left control pressed */
	right, /* right control pressed */
	up, /* up control pressed */
	down, /* down control pressed */
	fire, /* fire control pressed */
	xview, /* current x view */
	yview, /* current y view */
	xcursor, /* current x cursor */
	ycursor; /* current y cursor */

    /* initialise */
    map = uiscreen->data->game->battle->map;
    xview = uiscreen->data->xview;
    yview = uiscreen->data->yview;

    /* main navigation loop */
    do {

	/* wait for valid control event */
	controls->release (250);
	controls->wait ();
	left = controls->left ();
	right = controls->right ();
	up = controls->up ();
	down = controls->down ();
	fire = controls->fire ();

	/* move around the map */
	if (left || right || up || down) {

	    /* copy the current cursor location */
	    xcursor = uiscreen->data->xcursor;
	    ycursor = uiscreen->data->ycursor;

	    /* move the cursor */
	    if (xcursor > 0 && left)
		--xcursor;
	    else if (xcursor < map->width - 1 && right)
		++xcursor;
	    else if (ycursor > 0 && up)
		--ycursor;
	    else if (ycursor < map->height - 1 && down)
		++ycursor;

	    /* ensure the cursor is within the view */
	    if (xcursor < xview)
		xview = xcursor;
	    else if (xcursor > 8 + xview)
		xview = xcursor - 8;
	    else if (ycursor < yview)
		yview = ycursor;
	    else if (ycursor > 8 + yview)
		yview = ycursor - 8;

	    /* redisplay the map or erase the old cursor */
	    if (xview != uiscreen->data->xview ||
		yview != uiscreen->data->yview) {
		uiscreen->data->xview = xview;
		uiscreen->data->yview = yview;
		display->showmap (xview, yview);
	    } else
		display->hidemapcursor (uiscreen->data->xcursor,
					uiscreen->data->ycursor,
					xview, yview);

	    /* save the new cursor position */
	    uiscreen->data->xcursor = xcursor;
	    uiscreen->data->ycursor = ycursor;

	    /* display the new cursor and update */
	    display->showmapcursor (xcursor, ycursor, xview, yview);
	    display->update ();

	}
	
    } while (! fire);
}

/**
 * Figure out the default menu option for the current situation.
 * @param  uiscreen The user interface screen object.
 * @return          The most appropriate default menu option.
 */
static int defaultoption (UIScreen *uiscreen)
{
    Map *map; /* shorthand for current battle map */
    int x, /* shorthand for X cursor position */
	y, /* shorthand for Y cursor position */
	uid, /* ID of unit to examine */
	ut = -1, /* unit type */
	s = -1, /* unit side */
	b = 0, /* unit build capability (1/0) */
	d = 255, /* distance between selected unit and current square */
	p, /* player whose turn it is */
	r = 0; /* 1 if unit needs repair */
    Unit *unit = NULL; /* unit cursor is on (or NULL) */
    UnitType *utype = NULL; /* type of unit the cursor is on */

    /* gather information about last selected unit */
    p = uiscreen->data->game->battle->side;
    if ((uid = uiscreen->data->unit) != CWG_NO_UNIT) {
	unit = uiscreen->data->game->battle->units[uid];
	ut = unit->utype;
	utype = uiscreen->data->game->campaign->unittypes[ut];
	for (ut = 0; ut < CWG_UTYPES; ++ut)
	    if ((b = utype->builds[ut]))
		break;
	d = abs (uiscreen->data->xcursor - unit->x);
	if (abs (uiscreen->data->ycursor - unit->y) > d)
	    d = abs (uiscreen->data->ycursor - unit->y);
    }

    /* gather information about unit under the cursor */
    map = uiscreen->data->game->battle->map;
    x = uiscreen->data->xcursor;
    y = uiscreen->data->ycursor;
    uid = map->units[x + map->width * y];
    if (uid != CWG_NO_UNIT) {
	unit = uiscreen->data->game->battle->units[uid];
	s = unit->side;
	ut = unit->utype;
	utype = uiscreen->data->game->campaign->unittypes[ut];
	if (unit->hits < utype->hits)
	    r = 1;
    }

    /* decide option based on unit */
    if (b && d == 1 && uid == CWG_NO_UNIT)
	return 4; /* build on adjacent empty square */
    else if (b && d == 1 && s == p && r)
	return 5; /* repair adjacent friendly unit */
    else if (s == p)
	return 1; /* select friendly unit */
    else if (uid == CWG_NO_UNIT)
	return 2; /* move to empty square */
    else if (s == 1 - p)
	return 3; /* attack enemy unit */
    else
	return 1; /* select by default */
}

/**
 * Select a square and update the information panel.
 * @param uiscreen The screen data.
 */
static void selectsquare (UIScreen *uiscreen)
{
    int u, /* unit number on square */
	loc; /* cursor location as a single integer */
    Battle *battle; /* convenience pointer to battle */

    /* assign convenience variables */
    battle = uiscreen->data->game->battle;
    loc = uiscreen->data->xcursor + battle->map->width *
	uiscreen->data->ycursor;
    
    /* see if this is a friendly unit */
    if ((u = battle->map->units[loc]) != CWG_NO_UNIT &&
	battle->units[u]->side == battle->side)
	uiscreen->data->unit = u;

    /* show the information for the selected square */
    display->showmapinfo (uiscreen->data->xcursor,
			  uiscreen->data->ycursor,
			  uiscreen->data->unit);
    display->update ();
}

/**
 * Move the currently selected unit towards the cursor location.
 * @param uiscreen The screen data.
 */
static void moveunit (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to the game */
    Battle *battle; /* convenience pointer to the battle */
    Unit *unit; /* convenience pointer to the unit */

    /* check if there is a unit selected */
    if (uiscreen->data->unit == CWG_NO_UNIT) {
	display->prompt ("No unit selected");
	return;
    }

    /* ensure we have a report to record the action */
    game = uiscreen->data->game;
    if (! game->report) {
	game->report = new_Report ();
	game->report->battle = game->battle->clone (game->battle);
    } else if (! game->report->battle)
	game->report->battle = game->battle->clone (game->battle);

    /* move the unit */
    battle = uiscreen->data->game->battle;
    unit = battle->units[uiscreen->data->unit];
    if (! battle->move (battle, unit, uiscreen->data->xcursor,
			uiscreen->data->ycursor, movehook)) {
	display->prompt ("Cannot move unit to that location");
	return;
    }

    /* put the cursor back on the map */
    display->showmapcursor (uiscreen->data->xcursor,
			    uiscreen->data->ycursor,
			    uiscreen->data->xview,
			    uiscreen->data->yview);
    display->update ();
}

/**
 * Attack the unit at the cursor location using the currently selected
 * unit.
 * @param uiscreen The user interface screen data.
 */
static void attackunit (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to the game */
    Battle *battle; /* the battle */
    Map *map; /* the battle map */
    int x, /* cursor x location */
	y, /* cursor y location */
	uid; /* unit ID at cursor location */

    /* check if there is a unit selected */
    if (uiscreen->data->unit == CWG_NO_UNIT) {
	display->prompt ("No unit selected");
	return;
    }

    /* check that the cursor is on an enemy unit */
    battle = uiscreen->data->game->battle;
    map = battle->map;
    x = uiscreen->data->xcursor;
    y = uiscreen->data->ycursor;
    if ((uid = map->units[x + map->width * y]) == CWG_NO_UNIT ||
	battle->units[uid]->side == battle->side) {
	display->prompt ("Point to an enemy unit first");
	return;
    }

    /* ensure we have a report to record the action */
    game = uiscreen->data->game;
    if (! game->report) {
	game->report = new_Report ();
	game->report->battle = game->battle->clone (game->battle);
    } else if (! game->report->battle)
	game->report->battle = game->battle->clone (game->battle);

    /* attack the enemy */
    if (! battle->attack (battle, battle->units[uiscreen->data->unit],
			  battle->units[uid], attackhook)) {
	display->prompt ("Cannot attack that target");
	return;
    }

    /* put the cursor back on the map */
    display->showmapcursor (uiscreen->data->xcursor,
			    uiscreen->data->ycursor,
			    uiscreen->data->xview,
			    uiscreen->data->yview);
    display->update ();
}

/**
 * Build a unit at the cursor location using the currently selected 
 * unit.
 * @param uiscreen The user interface screen data.
 */
static void buildunit (UIScreen *uiscreen)
{
    int u, /* unit type counter */
	bcount = 0, /* unit types that can be built */
	btypes[CWG_UTYPES], /* list of build types */
	btype, /* build type chosen */
	x, /* x coordinate of cursor */
	y; /* y coordinate of cursor */
    Game *game; /* shortcut to game */
    Battle *battle; /* the battle */
    UnitType *utype; /* type of unit building */

/* check if there is a unit selected */
    if (uiscreen->data->unit == CWG_NO_UNIT) {
	display->prompt ("No unit selected");
	return;
    }

    /* initialise convenience variables */
    game = uiscreen->data->game;
    u = game->battle->units[uiscreen->data->unit]->utype;
    utype = game->campaign->unittypes[u];
    x = uiscreen->data->xcursor;
    y = uiscreen->data->ycursor;

    /* check what can be built */
    for (u = 0; u < CWG_UTYPES; ++u)
	if (game->campaign->scenarios[game->scenid]->battle->builds[u]
	    && utype->builds[u]) {
	    btypes[bcount] = u;
	    ++bcount;
	}
    if (! bcount) {
	display->prompt ("This unit cannot build.");
	return;
    }

    /* select a unit type to build, and build it */
    btype = getbuildtype (uiscreen, bcount, btypes);
    if (btype != -1) {

	/* ensure we have a report to record the action */
	if (! game->report) {
	    game->report = new_Report ();
	    game->report->battle = game->battle->clone (game->battle);
	} else if (! game->report->battle)
	    game->report->battle = game->battle->clone (game->battle);

	/* now build the unit */
	battle = game->battle;
	if (battle->create (battle, battle->units[uiscreen->data->unit],
				 btype, x, y, buildhook) == CWG_NO_UNIT) {
	    display->prompt ("Cannot build that unit");
	    return;
	}
    }
}

/**
 * Repair the unit at the cursor location using the currently selected
 * unit.
 * @param uiscreen The user interface screen data.
 */
static void repairunit (UIScreen *uiscreen)
{
    Game *game; /* convenience pointer to the game */
    Battle *battle; /* the battle */
    Map *map; /* the battle map */
    int x, /* cursor x location */
	y, /* cursor y location */
	uid; /* unit ID at cursor location */

    /* check if there is a unit selected */
    if (uiscreen->data->unit == CWG_NO_UNIT) {
	display->prompt ("No unit selected");
	return;
    }

    /* check that the cursor is on a friendly unit */
    battle = uiscreen->data->game->battle;
    map = battle->map;
    x = uiscreen->data->xcursor;
    y = uiscreen->data->ycursor;
    if ((uid = map->units[x + map->width * y]) == CWG_NO_UNIT ||
	battle->units[uid]->side != battle->side) {
	display->prompt ("Point to a friendly unit first");
	return;
    }

    /* ensure we have a report to record the action */
    game = uiscreen->data->game;
    if (! game->report) {
	game->report = new_Report ();
	game->report->battle = game->battle->clone (game->battle);
    } else if (! game->report->battle)
	game->report->battle = game->battle->clone (game->battle);

    /* repair the target unit */
    if (! battle->restore (battle, battle->units[uiscreen->data->unit],
			   battle->units[uid], repairhook)) {
	display->prompt ("Cannot repair that unit");
	return;
    }

    /* put the cursor back on the map */
    display->showmapcursor (uiscreen->data->xcursor,
			    uiscreen->data->ycursor,
			    uiscreen->data->xview,
			    uiscreen->data->yview);
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
    Game *game; /* current game */
    int loc; /* centre of friendly units */

    /* give the CWG hooks access to the UI data */
    hookdata = uiscreen->data;

    /* assign some convenience variables */
    game = uiscreen->data->game;
    game->state = STATE_HUMAN;

    /* set the difficulty levels */
    game->battle->setlevel (game->battle->side, 0);
    switch (game->playertypes[1 - game->battle->side]) {
    case PLAYER_HUMAN:
    case PLAYER_PBM:
	game->battle->setlevel (1 - game->battle->side, 0);
	break;
    case PLAYER_COMPUTER:
    case PLAYER_FAIR:
    case PLAYER_HARD:
	game->battle->setlevel
	    (1 - game->battle->side,
	     game->playertypes[1 - game->battle->side] - 1);
	break;
    }

    /* set the movement algorithm to quick */
    game->battle->setmovement (CWG_MOVE_QUICK);

    /* get the map location to view */
    loc = uiscreen->maplocation (game->battle);
    uiscreen->data->xcursor = (loc % game->battle->map->width);
    uiscreen->data->ycursor = (loc / game->battle->map->width);
    uiscreen->data->xview = uiscreen->data->xcursor - 4;
    uiscreen->data->yview = uiscreen->data->ycursor - 4;

    /* see if the cursor is on a friendly unit */
    uiscreen->data->unit =
	game->battle->map->units[uiscreen->data->xcursor +
				 game->battle->map->width *
				 uiscreen->data->ycursor];
    if (uiscreen->data->unit != CWG_NO_UNIT &&
	game->battle->units[uiscreen->data->unit]->side !=
	game->battle->side)
	uiscreen->data->unit = CWG_NO_UNIT;
    
    /* prepare the player turn map */
    display->preparemap (game->campaign, game->battle);
}

/**
 * Show the UI screen.
 * @param uiscreen The screen to show.
 * @return         The ID of the screen to show next.
 */
static UIState show (UIScreen *uiscreen)
{
    int option, /* menu option chosen */
	state; /* state returned by start screen */
    Game *game; /* convenience pointer to the game */

    /* show the start screen if this is a 2-player game. */
    game = uiscreen->data->game;
    state = uiscreen->startscreen (game);
    if (state != STATE_HUMAN)
	return state;
    else if (game->battle->victory (game->battle) != -1)
	return STATE_DEBRIEFING;

    /* initialise the display */
    display->showhumanturn (uiscreen->data->xcursor,
			    uiscreen->data->ycursor,
			    uiscreen->data->xview,
			    uiscreen->data->yview);
    showresources (game->battle);
    display->prompt
	("Movement keys to navigate, hold FIRE for menu");
    display->update ();

    /* main loop */
    while (1) {

	/* let the player navigate the map */
	navigatemap (uiscreen);
	display->prompt ("");

	/* get a choice from the menu */
	option = defaultoption (uiscreen);
	option = display->menu (9, humanmenu, option);
	switch (option) {

	case 0: /* cancel menu */
	    break;

	case 1: /* select */
	    selectsquare (uiscreen);
	    break;

	case 2: /* move */
	    moveunit (uiscreen);
	    if (game->battle->victory (game->battle) != -1)
		return STATE_DEBRIEFING;
	    break;

	case 3: /* attack */
	    attackunit (uiscreen);
	    if (game->battle->victory (game->battle) != -1)
		return STATE_DEBRIEFING;
	    break;

	case 4: /* build */
	    buildunit (uiscreen);
	    if (game->battle->victory (game->battle) != -1)
		return STATE_DEBRIEFING;
	    break;

	case 5: /* repair */
	    repairunit (uiscreen);
	    break;

	case 6: /* done turn */
	    if (uiscreen->confirm ("Really finished?")) {
		game->turn (game);
		return game->state;
	    }
	    break;

	case 7: /* new game */
	    return STATE_NEWGAME;

	case 8: /* exit game */
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
UIScreen *new_HumanTurnScreen (Game *game)
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
    uiscreen->data->xcursor
	= uiscreen->data->ycursor
	= uiscreen->data->xview
	= uiscreen->data->yview = 0;
    uiscreen->data->unit = CWG_NO_UNIT;

    /* return the screen */
    return uiscreen;
}
