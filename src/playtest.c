/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 06-Mar-2023.
 *
 * AI Playtest Module.
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
#include "cwg.h"
#include "fatal.h"
#include "campaign.h"
#include "scenario.h"
#include "report.h"
#include "game.h"
#include "ai.h"

/*----------------------------------------------------------------------
 * Data Definitions
 */

/** @var gamecount The number of games to play. */
static int gamecount = 10;

/** @var scenid The scenario to play. */
static int scenid = 1;

/** @var maxturns Maximum number of turns to play. */
static int maxturns = 99;

/** @var campaignfile The campaign filename. */
static char *campaignfile = NULL;

/** @var game The game in progress. */
static Game *game;

/** @var config Pointer to the configuration. */
static Config *config = NULL;

/** @var pass Number of turns passed due to buggy AI */
static int pass = 0;

/*----------------------------------------------------------------------
 * Hooks for AI.
 */

/* dummy hooks */
static void prompthook (char *message, int percent) {}
static void loghook (char *message) {}

/*----------------------------------------------------------------------
 * Level 2 Private Function Definitions
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
	if (! strncmp (argv[c], "-g", 2))
	    gamecount = atoi (&argv[c][2]);
	else if (! strncmp (argv[c], "-s", 2))
	    scenid = atoi (&argv[c][2]);
	else if (! strncmp (argv[c], "-t", 2))
	    maxturns = atoi (&argv[c][2]);
	else if (! campaignfile)
	    campaignfile = argv[c];
	else
	    fatalerror (FATAL_COMMAND_LINE);
}

/**
 * Play a turn in the battle.
 * @param  game The game to play.
 * @return      The victorious side 0 or 1, or -1 if battle continues.
 */
static int playturn (Game *game)
{
    AI *ai; /* pointer to the AI */
    printf ("turn %d %s\r", game->turnno,
	    game->campaign->corpnames[game->battle->side]);
    ai = get_AI (game, prompthook, loghook);
    ai->turn ();
    if (game->report && game->report->count)
	pass = 0;
    else
	pass++;
    game->turn (game);
    game->report->destroy (game->report);
    game->report = NULL;
    return game->battle->victory (game->battle);
}

/*----------------------------------------------------------------------
 * Level 1 Private Function Definitions
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

    /* load the campaign */
    if (! (game = new_Game ()))
	fatalerror (FATAL_MEMORY);
    if (! (game->campaign = new_Campaign ()))
	fatalerror (FATAL_MEMORY);
    strcpy (game->campaign->filename, campaignfile);
    if (! strchr (game->campaign->filename, '.'))
	strcat (game->campaign->filename, ".cam");
    if (! (game->campaign->load (game->campaign, 0)))
	fatalerror (FATAL_INVALIDDATA);

    /* set the configuration */
    if (! (config = new_Config ()))
	fatalerror (FATAL_MEMORY);
    strcpy (config->campaignfile, game->campaign->filename);
    config->playertypes[0] = PLAYER_COMPUTER;
    config->playertypes[1] = PLAYER_COMPUTER;
    *config->gamefile = '\0';

    /* initialise the random number generator */
    srand (time (NULL));
}

/**
 * Main playtest loop.
 */
static void playgame (void)
{
    Scenario *scenario; /* pointer to the scenario */ 
    int victory, /* victory for the game being played */
	victories[2], /* count of victories for each player */
	g; /* game counter */

    /* initialise */
    scenario = game->campaign->scenarios[scenid - 1];
    victories[0] = victories[1] = 0;
    game->report = NULL;

    /* loop through the games */
    for (g = 0; g < gamecount; ++g) {

	/* initialise the game */
	printf ("Fighting battle %d/%d\n", g + 1, gamecount);
	game->battle = scenario->battle->clone (scenario->battle);
	game->battle->start = game->battle->side = rand () % 2;
	game->turnno = 0;

	/* play the turns */
	pass = 0;
	do {
	    playturn (game);
	    victory = game->battle->victory (game->battle);
	} while (victory == -1 && game->turnno < maxturns && pass < 2);

	/* clean up the game */
	if (victory != -1)
	    ++victories[victory];
	game->battle->destroy (game->battle);
	game->battle = NULL;
    }

    /* print the totals */
    printf ("%s: %d wins\n", game->campaign->corpnames[0], victories[0]);
    printf ("%s: %d wins\n", game->campaign->corpnames[1], victories[1]);
    printf ("%d stalemates\n", gamecount - victories[0] - victories[1]);
}

/**
 * Clean up when the user quits normally.
 */
static void endprogram (void)
{
    AI *ai; /* pointer to AI */
    if ((ai = get_AI (game, prompthook, loghook)))
	ai->destroy ();
    //game->destroy (game);
    config->destroy ();
}

/*----------------------------------------------------------------------
 * Public Level Function Definitions
 */

/**
 * Main Program.
 * @param argc is the number of command line arguments.
 * @param argv is an array of command line arguments.
 * @return 0 if successful, >0 on error.
 */
int main (int argc, char **argv)
{
    initialiseprogram (argc, argv);
    playgame ();
    endprogram ();
    return 0;
}

/**
 * Share the config handler with other modules.
 * @return A pointer to the config module.
 */
Config *getconfig (void)
{
    return config;
}

