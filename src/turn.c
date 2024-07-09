/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Play-by-mail Turn Header.
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
#include "turn.h"
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
 * Destroy any subsidiary objects for this turn.
 * @param turn The turn to clear.
 */
static void destroyobjects (Turn *turn)
{
    if (turn->battle)
	turn->battle->destroy (turn->battle);
    if (turn->report)
	turn->report->destroy (turn->report);
}

/**
 * Initialise the attributes of a turn object.
 * @param turn The turn to initialise.
 */
static void initialiseattributes (Turn *turn)
{
    *turn->filename = '\0';
    strcpy (turn->campaignfile, config->campaignfile);
    turn->playertypes[0] = config->playertypes[0];
    turn->playertypes[1] = config->playertypes[1];
    turn->briefed[0] = turn->briefed[1] = 0;
    turn->debriefed[0] = turn->debriefed[1] = 0;
    turn->scenid = 0;
    turn->player = 0;
    turn->start = 0;
    turn->turnno = 0;
    turn->battle = NULL;
    turn->report = NULL;
}

/*----------------------------------------------------------------------
 * Public Method Level Functions.
 */

/**
 * Destroy the turn when it is no longer needed.
 */
static void destroy (Turn *turn)
{
    if (turn) {
	destroyobjects (turn);
	free (turn);
    }
}

/**
 * Clear the turn data.
 * @param turn The turn object to clear.
 */
static void clear (Turn *turn)
{
    destroyobjects (turn);
    initialiseattributes (turn);
}

/**
 * Save the turn. The filename is taken from the attributes.
 * @param  turn The turn to save.
 * @return      1 if successful, 0 on failure.
 */
static int save (Turn *turn)
{
    FILE *output; /* the output file */
    int success = 1, /* return value */
	noreport = 0; /* constant for no report */

    /* open the output file */
    if (! (output = fopen (turn->filename, "wb")))
	return 0;

    /* write the turn header */
    success = success && fwrite ("BAR100G", 8, 1, output);

    /* write the basic information and battle state */
    success = success &&
	cwg->writestring (turn->campaignfile, output) &&
	cwg->writeint (&turn->playertypes[0], output) &&
	cwg->writeint (&turn->playertypes[1], output) &&
	cwg->writeint (&turn->briefed[0], output) &&
	cwg->writeint (&turn->briefed[1], output) &&
	cwg->writeint (&turn->debriefed[0], output) &&
	cwg->writeint (&turn->debriefed[1], output) &&
	cwg->writeint (&turn->scenid, output) &&
	cwg->writeint (&turn->player, output) &&
	cwg->writeint (&turn->start, output) &&
	cwg->writeint (&turn->turnno, output) &&
	turn->battle->write (turn->battle, output);
    if (turn->report)
	success = success &&
	    turn->report->write (turn->report, output);
    else
	success = success &&
	    fwrite (&noreport, 2, 1, output);

    /* close the output file and return */
    fclose (output);
    return success;
}

/**
 * Load a turn. The filename is taken from the attributes.
 * @param  turn    The turn to load.
 * @param  summary 0 to load the full turn, 1 for summary only.
 * @return         1 if successful, 0 on failure.
 */
static int load (Turn *turn, int summary)
{
    FILE *input; /* the input file */
    int success = 1; /* return value */
    char header[8]; /* header read from file */

    /* open the input file */
    if (! (input = fopen (turn->filename, "rb")))
	return 0;

    /* read the turn header */
    success = success &&
	fread (header, 8, 1, input) &&
	! strncmp (header, "BAR100G", 8);

    /* read the turn settings */
    success = success &&
	cwg->readstring (turn->campaignfile, input);
    success = success &&
	cwg->readint (&turn->playertypes[0], input);
    success = success &&
	cwg->readint (&turn->playertypes[1], input);

    /* load the turn state and progress information */
    success = success &&
	cwg->readint (&turn->briefed[0], input);
    success = success &&
	cwg->readint (&turn->briefed[1], input);
    success = success &&
	cwg->readint (&turn->debriefed[0], input);
    success = success &&
	cwg->readint (&turn->debriefed[1], input);
    success = success &&
	cwg->readint (&turn->scenid, input);
    success = success &&
	cwg->readint (&turn->player, input);
    success = success &&
	cwg->readint (&turn->start, input);
    success = success &&
	cwg->readint (&turn->turnno, input);

    /* stop here if only the summary is needed */
    if (summary) {
	fclose (input);
	return success;
    }

    /* load in the campaign and current battle state */
    if (success) {
	if (turn->battle)
	    turn->battle->destroy (turn->battle);
	turn->battle = new_Battle (NULL, NULL);
	success = success &&
	    turn->battle->read (turn->battle, input);
    }

    /* load in the initial battle state and turn report */
    if (success) {
	if (turn->report)
	    turn->report->clear (turn->report);
	else
	    turn->report = new_Report ();
	success = success &&
	    turn->report->read (turn->report, input);
	if (! turn->report->count) {
	    turn->report->destroy (turn->report);
	    turn->report = NULL;
	}
    }
    
    /* close the input file and return */
    fclose (input);
    return success;
}

/**
 * Populate a PBM turn object from a game.
 * @param turn The turn to populate.
 * @param game The game affected.
 * @return     1 if successful, 0 on failure.
 */
static int gametoturn (Turn *turn, Game *game)
{
    char *ptr; /* pointer to file extension */

    /* clear old data from the turn object */
    turn->clear (turn);

    /* determine the turn filename */
    strcpy (turn->filename, game->filename);
    if ((ptr = strchr (turn->filename, '.')))
	*ptr = '\0';
    strcat (turn->filename, ".trn");

    /* copy all the other attributes across */
    strcpy (turn->campaignfile, game->campaignfile);
    turn->playertypes[0] = game->playertypes[0];
    turn->playertypes[1] = game->playertypes[1];
    turn->briefed[0] = game->briefed[0];
    turn->briefed[1] = game->briefed[1];
    turn->debriefed[0] = game->debriefed[0];
    turn->debriefed[1] = game->debriefed[1];
    turn->scenid = game->scenid;
    turn->player = game->battle->side;
    turn->start = game->battle->start;
    turn->turnno = game->turnno;
    turn->battle = game->battle->clone (game->battle);
    if (turn->report) {
	turn->report->destroy (turn->report);
	turn->report = NULL;
    }
    if (game->report)
	turn->report = game->report->clone (game->report);

    /* return */
    return 1;
}

/**
 * Populate a game object from a PBM turn.
 * @param turn The turn to populate the game from.
 * @param game The game to populate.
 * @return     1 if successful, 0 on failure.
 */
static int turntogame (Turn *turn, Game *game)
{
    char *ptr; /* pointer to file extension */
    int debrief; /* has this player seen the debrief screen? */

    /* this player seen the debrief screen? */
    if (! game->battle)
	debrief = 0;
    else if (game->battle->victory (game->battle) == -1)
	debrief = 0;
    else
	debrief = 1;

    /* clear old data from the turn object */
    game->clear (game);

    /* load in the game campaign */
    game->campaign = new_Campaign ();
    strcpy (game->campaign->filename, turn->campaignfile);
    if (! game->campaign->load (game->campaign, 0))
	return 0;

    /* determine the turn filename */
    strcpy (game->filename, turn->filename);
    if ((ptr = strchr (game->filename, '.')))
	*ptr = '\0';
    strcat (game->filename, ".gam");

    /* swap the human/pbm roles for the players */
    game->playertypes[0] = turn->playertypes[1];
    game->playertypes[1] = turn->playertypes[0];

    /* copy all the other attributes across */
    strcpy (game->campaignfile, turn->campaignfile);
    game->briefed[0] = turn->briefed[0];
    game->briefed[1] = turn->briefed[1];
    game->debriefed[0] = turn->debriefed[0];
    game->debriefed[1] = turn->debriefed[1];
    game->scenid = turn->scenid;
    game->turnno = turn->turnno;
    game->battle = turn->battle->clone (turn->battle);
    if (game->report) {
	game->report->destroy (game->report);
	game->report = NULL;
    }
    if (turn->report)
	game->report = turn->report->clone (turn->report);

    /* point the game battles to the campaign units/terrain */
    game->battle->utypes = game->campaign->unittypes;
    game->battle->terrain = game->campaign->terrain;

    /* figure out the game state */
    game->state = game->turnno ?
	STATE_REPORT :
	STATE_BRIEFING;

    /* return */
    return 1;
}

/*----------------------------------------------------------------------
 * Constructor Functions.
 */

/**
 * Create and initialise a turn.
 * @return the new turn.
 */
Turn *new_Turn (void)
{
    Turn *turn; /* the new turn */

    /* reserve memory for the turn and reports */
    if (! (turn = malloc (sizeof (Turn))))
	return NULL;

    /* initialise the methods */
    turn->destroy = destroy;
    turn->clear = clear;
    turn->save = save;
    turn->load = load;
    turn->gametoturn = gametoturn;
    turn->turntogame = turntogame;

    /* grab library pointers before we use them */
    cwg = get_Cwg ();
    config = getconfig ();

    /* initialise the attributes */
    initialiseattributes (turn);

    /* return the new turn */
    return turn;
}
