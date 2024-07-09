/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 26-Jan-2021.
 *
 * Game Handler Module.
 */

/*----------------------------------------------------------------------
 * Included Headers.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project-specific headers */
#include "cwg.h"
#include "barren.h"
#include "game.h"
#include "report.h"
#include "config.h"
#include "fatal.h"
#include "uiscreen.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var cwg A pointer to the Cwg library object. */
static Cwg *cwg = NULL;

/** @var config A pointer to the Config library object. */
static Config *config = NULL;

/*----------------------------------------------------------------------
 * Level 1 Private Functions.
 */

/**
 * Destroy any subsidiary objects for this game.
 * @param game The game to clear.
 */
static void destroyobjects (Game *game)
{
    if (game->battle)
	game->battle->destroy (game->battle);
    if (game->report)
	game->report->destroy (game->report);
    if (game->campaign)
	game->campaign->destroy (game->campaign);
}

/**
 * Initialise the attributes of a game object.
 * @param game The game to initialise.
 */
static void initialiseattributes (Game *game)
{
    *game->filename = '\0';
    strcpy (game->campaignfile, config->campaignfile);
    game->playertypes[0] = config->playertypes[0];
    game->playertypes[1] = config->playertypes[1];
    game->briefed[0] = game->briefed[1] = 0;
    game->debriefed[0] = game->debriefed[1] = 0;
    game->state = STATE_NEWGAME;
    game->scenid = 0;
    game->turnno = 0;
    game->battle = NULL;
    game->report = NULL;
    game->campaign = NULL;
}

/*----------------------------------------------------------------------
 * Public Method Level Functions.
 */

/**
 * Destroy the game when it is no longer needed.
 */
static void destroy (Game *game)
{
    if (game) {
	destroyobjects (game);
	free (game);
    }
}

/**
 * Clear the game data.
 * @param game The game object to clear.
 */
static void clear (Game *game)
{
    destroyobjects (game);
    initialiseattributes (game);
}

/**
 * Save the game. The filename is taken from the attributes.
 * @param  game The game to save.
 * @return      1 if successful, 0 on failure.
 */
static int save (Game *game)
{
    FILE *output; /* the output file */
    int success = 1, /* return value */
	noreport = 0; /* constant zero for no report */

    /* open the output file */
    if (! (output = fopen (game->filename, "wb")))
	return 0;

    /* write the game header */
    success = success && fwrite ("BAR100G", 8, 1, output);
    
    /* write the basic information */
    success = success &&
	cwg->writestring (game->campaignfile, output) &&
	cwg->writeint (&game->playertypes[0], output) &&
	cwg->writeint (&game->playertypes[1], output) &&
	cwg->writeint (&game->briefed[0], output) &&
	cwg->writeint (&game->briefed[1], output) &&
	cwg->writeint (&game->debriefed[0], output) &&
	cwg->writeint (&game->debriefed[1], output) &&
	cwg->writeint (&game->state, output) &&
	cwg->writeint (&game->scenid, output) &&
	cwg->writeint (&game->turnno, output);

    /* save the battle */
    success = success &&
	game->battle->write (game->battle, output);

    /* save the report */
    if (game->report)
	success = success &&
	    game->report->write (game->report, output);
    else
	success = success &&
	    fwrite (&noreport, 2, 1, output);

    /* close the output file and return */
    fclose (output);
    return success;
}

/**
 * Load a game. The filename is taken from the attributes.
 * @param  game    The game to load.
 * @param  summary 0 to load the full game, 1 for summary only.
 * @return         1 if successful, 0 on failure.
 */
static int load (Game *game, int summary)
{
    FILE *input; /* the input file */
    int success = 1; /* return value */
    char header[8]; /* header read from file */

    /* open the input file */
    if (! (input = fopen (game->filename, "rb")))
	return 0;

    /* read the game header */
    success = success &&
	fread (header, 8, 1, input) &&
	! strncmp (header, "BAR100G", 8);

    /* read the game settings */
    success = success &&
	cwg->readstring (game->campaignfile, input);
    success = success &&
	cwg->readint (&game->playertypes[0], input);
    success = success &&
	cwg->readint (&game->playertypes[1], input);

    /* stop here if only the summary is needed */
    if (summary) {
	fclose (input);
	return success;
    }

    /* load the game state and progress information */
    success = success &&
	cwg->readint (&game->briefed[0], input);
    success = success &&
	cwg->readint (&game->briefed[1], input);
    success = success &&
	cwg->readint (&game->debriefed[0], input);
    success = success &&
	cwg->readint (&game->debriefed[1], input);
    success = success &&
	cwg->readint (&game->state, input);
    success = success &&
	cwg->readint (&game->scenid, input);
    success = success &&
	cwg->readint (&game->turnno, input);

    /* load in the campaign and current battle state */
    if (success) {
	if (game->campaign)
	    game->campaign->clear (game->campaign);
	else if (! (game->campaign = new_Campaign ()))
	    fatalerror (FATAL_MEMORY);
	strcpy (game->campaign->filename, game->campaignfile);
	success = success && game->campaign->load (game->campaign, 0);
	if (game->battle)
	    game->battle->destroy (game->battle);
	game->battle = new_Battle (game->campaign->unittypes,
				   game->campaign->terrain);
	success = success &&
	    game->battle->read (game->battle, input);
    }

    /* load in the initial battle state and turn report */
    if (success) {
	if (game->report)
	    game->report->clear (game->report);
	else
	    game->report = new_Report ();
	success = success &&
	    game->report->read (game->report, input);
    }

    /* close the input file and return */
    fclose (input);
    return success;
}

/**
 * End the game turn.
 * @param game  The game affected.
 * @return      The next game state
 */
static int turn (Game *game)
{
    int u, /* resource gathering unit id */
	loc, /* unit location */
	victor; /* victor of this battle, if any */
    Unit *unit; /* unit for resource gathering */

    /* next player and, if appropriate, next turn number */
    game->battle->turn (game->battle);
    if (game->battle->side == game->battle->start)
	++game->turnno;

    /* check for victory of both scenario and campaign */
    victor = game->battle->victory (game->battle);
    if (victor == -1);
    else if (! game->debriefed[game->battle->side] &&
	game->playertypes[game->battle->side] != PLAYER_COMPUTER &&
	game->playertypes[game->battle->side] != PLAYER_FAIR &&
	game->playertypes[game->battle->side] != PLAYER_HARD)
	return game->state =
	    (game->playertypes[game->battle->side] == PLAYER_HUMAN) ?
	    STATE_REPORT :
	    STATE_PBM;
    else if (game->campaign->scenarios[game->scenid]->next[victor])
	return game->state = game->setscenario (game);
    else
	return game->state = STATE_ENDGAME;

    /* resource gathering */
    for (u = 0; u < CWG_UNITS; ++u)
	if ((unit = game->battle->units[u]) &&
	    unit->side == game->battle->side &&
	    unit->utype == game->campaign->gatherer)
	{
	    loc = unit->x + game->battle->map->width * unit->y;
	    if (game->battle->map->terrain[loc] ==
		game->campaign->resource)
		game->battle->resources[unit->side] += 2;
	}

    /* select the appropriate game state to start on */
    if (game->playertypes[game->battle->side] == PLAYER_COMPUTER ||
	game->playertypes[game->battle->side] == PLAYER_FAIR ||
	game->playertypes[game->battle->side] == PLAYER_HARD)
	return game->state = STATE_COMPUTER;
    else if (game->playertypes[game->battle->side] == PLAYER_PBM)
	return game->state = STATE_PBM;
    else if (! game->turnno)
	return game->state = STATE_BRIEFING;
    else
	return game->state = STATE_REPORT;
}

/**
 * Set up the next scenario.
 * @param game The game affected.
 * @return     The next game state.
 */
static int setscenario (Game *game)
{
    Scenario *scenario; /* convenience pointer to the scenario */
    int victor; /* who, if anyone, won the current battle */

    /* point to the current scenario */
    scenario = game->campaign->scenarios[game->scenid];

    /* check for no victory - either we haven't started a battle, 
       or the battle is still ongoing. */
    if (! game->battle ||
	(victor = game->battle->victory (game->battle)) == -1);

    /* check for a final victory - no more scenarios */
    else if (scenario->next[victor] == 0)
	return game->state = STATE_ENDGAME;

    /* otherwise point to the next scenario */
    else {
	game->scenid = scenario->next[victor] - 1;
	scenario = game->campaign->scenarios[game->scenid];
    }

    /* set up the battle */
    if (game->battle)
	game->battle->destroy (game->battle);
    game->battle = scenario->battle->clone (scenario->battle);
    game->battle->start = game->battle->side = rand () % 2;
    game->turnno = 0;

    /* set up the report data */
    if (game->report)
	game->report->destroy (game->report);
    game->report = NULL;

    /* reset the briefing/debriefing flags */
    game->briefed[0] = game->briefed[1] = 0;
    game->debriefed[0] = game->debriefed[1] = 0;

    /* ascertain the game state */
    switch (game->playertypes[game->battle->start]) {
    case PLAYER_HUMAN:
	game->state = STATE_BRIEFING;
	break;
    case PLAYER_COMPUTER:
    case PLAYER_FAIR:
    case PLAYER_HARD:
	game->state = STATE_COMPUTER;
	break;
    case PLAYER_PBM:
	game->state = STATE_PBM;
	break;
    }

    /* return the game state */
    return game->state;
}

/**
 * Calculate the centre of a force.
 * @param  game The game object.
 * @param  side The side to look for.
 * @return      the centre of the force..
 */
static int centre (Game *game, int side)
{
    int xloc, /* x location to return */
	yloc, /* y location to return */
	u, /* unit index */
	x, /* total of x coordinates of friendly units */
	y, /* total of y coordinates of friendly units */
	n; /* number of friendly units counted */
    Battle *battle; /* pointer to battle object */

    /* initialise convenience variables */
    battle = game->battle;

    /* sum up the total of units, and their aggregate position */
    for (x = y = n = u = 0; u < CWG_UNITS; ++u)
	if (battle->units[u] &&
	    battle->units[u]->hits &&
	    battle->units[u]->side == side) {
	    x += battle->units[u]->x;
	    y += battle->units[u]->y;
	    ++n;
	}

    /* set default position in case there are no friendly units */
    if (! n) {
	xloc = side * (battle->map->width - 1);
	yloc = 4 + (battle->map->height - 9) / 2;
    }

    /* work out an average position from the aggregate position */
    else {
	xloc = (n / 2 + x) / n;
	yloc = (n / 2 + y) / n;
    }

    /* return the calculated position */
    return xloc + battle->map->width * yloc;
}

/*----------------------------------------------------------------------
 * Constructor Functions.
 */

/**
 * Create and initialise a game.
 * @return the new game.
 */
Game *new_Game (void)
{
    Game *game; /* the new game */

    /* reserve memory for the game and reports */
    if (! (game = malloc (sizeof (Game))))
	return NULL;

    /* initialise the methods */
    game->destroy = destroy;
    game->clear = clear;
    game->save = save;
    game->load = load;
    game->turn = turn;
    game->setscenario = setscenario;
    game->centre = centre;

    /* grab library pointers before we use them */
    cwg = get_Cwg ();
    config = getconfig ();

    /* initialise the attributes */
    initialiseattributes (game);

    /* return the new game */
    return game;
}
