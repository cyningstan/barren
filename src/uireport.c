/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Turn Report Screen Module.
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* compiler specific headers */
#include <i86.h>

/* project-specific headers */
#include "cwg.h"
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

    /** @var battle The state of battle in the current frame. */
    Battle *battle;

    /** @var xview The x coordinate of the top left of the map view. */
    int xview;

    /** @var yview The y coordinate of the top left of the map view. */
    int yview;

    /** @var unit The location of the last selected friendly unit. */
    int loc;

    /** @var frame The frame being displayed. */
    int frame;

    /** @var playing 1 if the report is playing, 0 if paused. */
    int playing;

};

/** @var display A pointer to the display module. */
static Display *display;

/** @var controls A pointer to the game controls module. */
static Controls *controls;

/** @var config A pointer to the configuration. */
static Config *config;

/** @var humanmenu The menu for taking a turn. */
static char *reportmenu[] = {
    "Cancel menu",
    "Replay",
    "Done",
    "New game",
    "Exit game"
};

/*----------------------------------------------------------------------
 * Level 4 Private Functions.
 */

/**
 * Brief display of one unit firing at another, with sound.
 * @param origin The unit firing.
 * @param target The unit being fired at.
 * @param data   The UI screen data.
 */
static void showfire (Unit *origin, Unit *target, UIScreenData *data)
{
    Timer *timer; /* delay timer */

    /* update the display */
    display->showunitfire (origin->x, origin->y,
			   data->xview, data->yview);
    display->showunitblast (target->x, target->y,
			    data->xview, data->yview);
    display->update ();

    /* noise and delay */
    timer = new_Timer (250);
    display->playsound (DISPLAY_PEWPEW);
    timer->destroy (timer);
}

/**
 * Brief display of a unit being repaired, with sound.
 * @param target The unit being repaired
 * @param data   The UI screen data.
 */
static void showrepair (Unit *target, UIScreenData *data)
{
    Timer *timer; /* delay timer */
    display->showspanner (target->x, target->y,
			  data->xview, data->yview);
    display->update ();
    timer = new_Timer (250);
    display->playsound (DISPLAY_HAMMER);
    timer->destroy (timer);
}

/*----------------------------------------------------------------------
 * Level 3 Private Functions.
 */

/**
 * Simulate a unit movement on the game map.
 * @param uiscreen The user interface screen object.
 */
static void moveunit (UIScreen *uiscreen)
{
    UIScreenData *data; /* shortcut to data */
    Report *report; /* convenience pointer to the report */
    Map *map; /* convenience pointer to the map */
    Campaign *campaign; /* convenience poniter to the campaign */
    int origin, /* origin location of move */
	target, /* target location of move */
	xo, /* x location of origin */
	yo, /* y location of origin */
	xt, /* x location of target */
	yt; /* y location of target */

    /* initialise convenience variables */
    data = uiscreen->data;
    report = data->game->report;
    map = data->battle->map;
    campaign = data->game->campaign;

    /* ascertain the origin and target locations */
    origin = report->origin (report, data->frame);
    xo = origin % map->width;
    yo = origin / map->width;
    target = report->target (report, data->frame);
    xt = target % map->width;
    yt = target / map->width;

    /* move the unit on the conceptual map */
    map->units[target] = map->units[origin];
    map->units[origin] = CWG_NO_UNIT;

    /* update the unit's position */
    data->battle->units[map->units[target]]->x = xt;
    data->battle->units[map->units[target]]->y = yt;
    data->loc = target;

    /* update the display map */
    display->updatemap (campaign, data->battle, xo, yo);
    display->updatemap (campaign, data->battle, xt, yt);
}

/**
 * Simulate a unit attack on the game map.
 * @param uiscreen The user interface screen object.
 */
static void attackunit (UIScreen *uiscreen)
{
    UIScreenData *data; /* shortcut to data */
    Report *report; /* convenience pointer to the report */
    Map *map; /* convenience pointer to the map */
    Campaign *campaign; /* convenience poniter to the campaign */
    Unit *attacker, /* the attacking unit */
	*defender; /* the defending unit */
    int origin, /* origin location of move */
	target, /* target location of move */
	result; /* result of the battle */
    char message[128]; /* attack message */
    Timer *timer; /* timer object */

    /* initialise convenience variables */
    data = uiscreen->data;
    report = data->game->report;
    map = data->battle->map;
    campaign = data->game->campaign;
    result = report->action (report, data->frame) - ACTION_ATTACK;
    origin = report->origin (report, data->frame);
    target = report->target (report, data->frame);
    attacker = data->battle->units[map->units[origin]];
    defender = data->battle->units[map->units[target]];

    /* show an attack, and the return fire if appropriate */
    showfire (attacker, defender, data);
    if (result & CWG_ATT_RETURN_FIRE)
	showfire (defender, attacker, data);

    /* print the report message */
    sprintf (message, "%s %s %s %s %s",
	     campaign->corpnames[attacker->side],
	     attacker->name,
	     (result == CWG_ATT_DEFR_KILLED) ? "destroys" : "attacks",
	     campaign->corpnames[defender->side],
	     defender->name);
    strcat (message,
	    (result == CWG_ATT_ATTR_KILLED) ?
	    " and is destroyed!" :
	    ".");
    display->linetext (message, 18);

    /* remove any destroyed unit from the map */
    if (result == CWG_ATT_DEFR_KILLED) {
	defender->destroy (defender);
	data->battle->units[map->units[target]] = NULL;
	map->units[target] = CWG_NO_UNIT;
    } else if (result == CWG_ATT_ATTR_KILLED) {
	attacker->destroy (attacker);
	data->battle->units[map->units[origin]] = NULL;
	map->units[origin] = CWG_NO_UNIT;
    }
    
    /* show the aftermath */
    display->updatemap (campaign, data->battle,
			attacker->x, attacker->y);
    display->updatemap (campaign, data->battle,
			defender->x, defender->y);
    display->showmap (data->xview, data->yview);
    display->update ();

    /* explosion noise if one of the units is destroyed */
    if (result == CWG_ATT_DEFR_KILLED || result == CWG_ATT_ATTR_KILLED) {
	timer = new_Timer (250);
	display->playsound (DISPLAY_EXPLOSION);
	timer->destroy (timer);
    }
}

/**
 * Simulate a unit being built on the game map.
 * @param uiscreen The user interface screen object.
 */
static void buildunit (UIScreen *uiscreen)
{
    UIScreenData *data; /* shortcut to data */
    Report *report; /* convenience pointer to the report */
    Map *map; /* convenience pointer to the map */
    Campaign *campaign; /* convenience poniter to the campaign */
    Unit *builder, /* the attacking unit */
	*built; /* the defending unit */
    int origin, /* origin location of move */
	target, /* target location of move */
	result, /* result of the battle */
	utypeid, /* unit type built */
	u; /* free unit id */
    char message[128]; /* attack message */

    /* initialise convenience variables */
    data = uiscreen->data;
    report = data->game->report;
    map = data->battle->map;
    campaign = data->game->campaign;
    result = report->action (report, data->frame) - ACTION_ATTACK;
    origin = report->origin (report, data->frame);
    target = report->target (report, data->frame);
    builder = data->battle->units[map->units[origin]];
    utypeid = report->ttype (report, data->frame);

    /* simulate the build */
    for (u = 0; u < CWG_UNITS; ++u)
	if (! data->battle->units[u])
	    break;
    if (u == CWG_UNITS)
	return;
    built = data->battle->units[u] = new_Unit ();
    strcpy (built->name, campaign->unittypes[utypeid]->name);
    built->side = builder->side;
    built->utype = utypeid;
    built->x = target % map->width;
    built->y = target / map->width;
    built->hits = campaign->unittypes[utypeid]->hits;
    built->moves = 0;
    map->units[target] = u;

    /* show the spanner icon on the repaired unit */
    showrepair (built, data);

    /* print the report message */
    sprintf (message, "%s %s builds %s.",
	     data->game->campaign->corpnames[builder->side],
	     builder->name,
	     built->name);
    display->linetext (message, 18);

    /* show the aftermath */
    display->updatemap (campaign, data->battle,
			built->x, built->y);
    display->showmap (data->xview, data->yview);
    display->update ();
}

/**
 * Simulate a unit repair on the game map.
 * @param uiscreen The user interface screen object.
 */
static void repairunit (UIScreen *uiscreen)
{
    UIScreenData *data; /* shortcut to data */
    Report *report; /* convenience pointer to the report */
    Map *map; /* convenience pointer to the map */
    Campaign *campaign; /* convenience poniter to the campaign */
    Unit *builder, /* the attacking unit */
	*built; /* the defending unit */
    int origin, /* origin location of move */
	target, /* target location of move */
	result; /* result of the battle */
    char message[128]; /* attack message */

    /* initialise convenience variables */
    data = uiscreen->data;
    report = data->game->report;
    map = data->battle->map;
    campaign = data->game->campaign;
    result = report->action (report, data->frame) - ACTION_ATTACK;
    origin = report->origin (report, data->frame);
    target = report->target (report, data->frame);
    builder = data->battle->units[map->units[origin]];
    built = data->battle->units[map->units[target]];

    /* show the spanner icon on the repaired unit */
    showrepair (built, data);

    /* print the report message */
    sprintf (message, "%s %s repairs %s.",
	     data->game->campaign->corpnames[builder->side],
	     builder->name,
	     built->name);
    display->linetext (message, 18);

    /* show the aftermath */
    display->updatemap (campaign, data->battle,
			built->x, built->y);
    display->showmap (data->xview, data->yview);
    display->update ();
}

/*----------------------------------------------------------------------
 * Level 2 Private Functions.
 */

/**
 * Play back the report.
 * @param uiscreen A pointer to the UI screen object.
 */
static void play (UIScreen *uiscreen)
{
    UIScreenData *data; /* shortcut to data */
    Report *report; /* convenience pointer to the report */
    Map *map; /* convenience pointer to the map */
    int origin, /* origin location of move */
	target, /* target location of move */
	xo, /* x location of origin */
	yo, /* y location of origin */
	xt, /* x location of target */
	yt; /* y location of target */

    /* initialise convenience variables */
    data = uiscreen->data;
    report = data->game->report;
    map = data->battle->map;

    /* wait for FIRE then display the prompt */
    while (controls->fire ());
    display->linetext ("Report is playing.", 18);
    display->update ();
    display->prompt ("Hold FIRE for options.");

    /* play the report */
    while (! controls->fire () && data->playing) {

	/* ascertain the origin and target locations */
	origin = report->origin (report, data->frame);
	xo = origin % map->width;
	yo = origin / map->width;
	target = report->target (report, data->frame);
	xt = target % map->width;
	yt = target / map->width;

	/* ensure the units are visible on the map */
	if (xo < data->xview || xo >= data->xview + 9 ||
	    yo < data->yview || yo >= data->yview + 9 ||
	    xt < data->xview || xt >= data->xview + 9 ||
	    yt < data->yview || yt >= data->yview + 9 ||
	    data->loc != origin) {

	    /* centre map view on unit */
	    data->xview = xo - 4;
	    data->yview = yo - 4;

	    /* ensure view is entirely within the map */
	    if (data->xview < 0)
		data->xview = 0;
	    else if (data->xview > map->width - 9)
		data->xview = map->width - 9;
	    if (data->yview < 0)
		data->yview = 0;
	    else if (data->yview > map->height - 9)
		data->yview = map->height - 9;

	    /* show the map and pause */
	    display->showmap (data->xview, data->yview);
	    if (origin != data->loc) {
		data->loc = origin;
		display->showmapcursor
		    (xo, yo, data->xview, data->yview);
		display->update ();
		delay (500);
		display->hidemapcursor
		    (xo, yo, data->xview, data->yview);

	    }
	    display->update ();
	    delay (250);
	}

	/* Show the action */
	switch (report->action (report, data->frame)) {
	case ACTION_NONE:
	    break;
	case ACTION_MOVE:
	    moveunit (uiscreen);
	    break;
	case ACTION_ATTACK:
	case ACTION_BATTLE:
	case ACTION_DESTROY:
	case ACTION_ATTACK_FAIL:
	    attackunit (uiscreen);
	    break;
	case ACTION_BUILD:
	    buildunit (uiscreen);
	    break;
	case ACTION_REPAIR:
	    repairunit (uiscreen);
	    break;
	}
	display->showmap (data->xview, data->yview);
	display->update ();

	/* delay and move onto the next frame */
	delay (250);
	++data->frame;
	if (data->frame >= report->count) {
	    data->playing = 0;
	    display->linetext ("Report is finished.", 18);
	    display->update ();
	    display->prompt ("Hold FIRE for options.");
	}
    }

    /* wait for FIRE */
    uiscreen->scrollmap (uiscreen->data->battle,
			 &uiscreen->data->xview,
			 &uiscreen->data->yview);
}

/**
 * The report is paused. Wait for FIRE.
 * @param uiscreen A pointer to the UI screen object.
 */
static void pause (UIScreen *uiscreen)
{
    while (controls->fire ());
    display->prompt ("Hold FIRE for options.");
    uiscreen->scrollmap (uiscreen->data->battle,
			 &uiscreen->data->xview,
			 &uiscreen->data->yview);
}

/*----------------------------------------------------------------------
 * Level 1 Private Functions.
 */

/**
 * Handle report playback.
 * @param uiscreen A pointer to the UI screen object.
 */
static void playback (UIScreen *uiscreen)
{
    if (uiscreen->data->playing)
	play (uiscreen);
    else
	pause (uiscreen);
}

/**
 * Figure out the default menu option for the current situation.
 * @param  uiscreen The user interface screen object.
 * @return          The most appropriate default menu option.
 */
static int defaultoption (UIScreen *uiscreen)
{
    int frame, /* current frame */
	count; /* count of frames */

    /* initialise the convenience variables */
    frame = uiscreen->data->frame;
    count = uiscreen->data->game->report->count;

    /* determine the most appropriate default option */
    if (frame == count)
	return 2; /* done */
    else if (frame == 0 && count > 0)
	return 1; /* replay */
    else
	return 0; /* cancel menu */
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
    Battle *battle; /* current displayed battle state */
    if (uiscreen) {
	if (uiscreen->data) {
	    if ((battle = uiscreen->data->battle))
		battle->destroy (battle);
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
    Game *game; /* convenience pointer to the game */
    Battle *battle; /* convenience pointer to the battle */
    int loc; /* centre of friendly units */
    Report *report; /* convenience variable */

    /* quit if there is no report */
    report = uiscreen->data->game->report;
    if (! report || ! report->count || ! report->battle)
	return;

    /* assign some convenience variables */
    game = uiscreen->data->game;
    game->state = STATE_REPORT;
    uiscreen->data->battle = battle =
	report->battle->clone (report->battle);

    /* get the map location to view */
    loc = uiscreen->maplocation (battle);
    uiscreen->data->xview = (loc % battle->map->width) - 4;
    uiscreen->data->yview = (loc / battle->map->width) - 4;

    /* prepare the player turn map */
    display->preparemap (game->campaign, battle);
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
    Report *report; /* convenience pointer to the report */

    /* skip the report screen if there is nothing to report */
    report = uiscreen->data->game->report;
    if (! report || ! report->count || ! report->battle)
	return STATE_HUMAN;

    /* show the start screen if this is a 2-player game */
    game = uiscreen->data->game;
    state = uiscreen->startscreen (game);
    if (state != STATE_REPORT)
	return state;

    /* initialise the display */
    display->showturnreport (uiscreen->data->xview,
			     uiscreen->data->yview);
    display->linetext ("Report is ready to replay.", 18);
    display->prompt
	("Battle Report: hold FIRE for menu");
    display->update ();

    /* main loop */
    while (1) {

	/* report playback */
	playback (uiscreen);
	if (uiscreen->data->playing) {
	    display->linetext ("Report is paused.", 18);
	    display->update();
	}
	display->prompt ("");

	/* get a choice from the menu */
	option = defaultoption (uiscreen);
	option = display->menu (5, reportmenu, option);
	switch (option) {

	case 0: /* cancel menu */
	    break;

	case 1: /* replay report */
	    uiscreen->data->battle->destroy (uiscreen->data->battle);
	    uiscreen->data->battle =
		report->battle->clone
		(report->battle);
	    display->preparemap (game->campaign,
				 uiscreen->data->battle);
	    uiscreen->data->frame = 0;
	    uiscreen->data->playing = 1;
	    uiscreen->data->loc = -1;
	    break;

	case 2: /* done with report */
	    report->destroy (report);
	    game->report = NULL;
	    return STATE_HUMAN;

	case 3: /* new game */
	    return STATE_NEWGAME;

	case 4: /* exit game */
	    return STATE_QUIT;

	}
    }
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Construct the Turn Report Screen.
 * @param  game The game object.
 * @return      The new Turn Report Screen.
 */
UIScreen *new_TurnReportScreen (Game *game)
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
    uiscreen->data->battle = NULL;
    uiscreen->data->xview
	= uiscreen->data->yview = 0;
    uiscreen->data->loc = -1;
    uiscreen->data->frame = 0;
    uiscreen->data->playing = 0;

    /* return the screen */
    return uiscreen;
}
