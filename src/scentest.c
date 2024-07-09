/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 29-Dec-2020.
 *
 * Scenario Test Program.
 * Prepares a saved game with the required parameters.
 */

/*----------------------------------------------------------------------
 * Headers
 */

/* ANSI C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* project-specific headers */
#include "barren.h"
#include "fatal.h"
#include "config.h"
#include "game.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @var config The configuration. */
static Config *config;

/** @var game The game. */
static Game *game;

/*----------------------------------------------------------------------
 * Level 1 Function Definitions.
 */

/**
 * Initialise configuration.
 * @return 1 if successful, 0 if not.
 */
static int initconfig (void)
{
    if (! (config = new_Config ())) {
	printf ("Out of memory creating config\n");
	return 0;
    }
    config->load ();
    return 1;
}

/**
 * Initialise a game from the configuration settings.
 * @return 1 if successful, 0 if not.
 */
static int initgame (void)
{
    if (! (game = new_Game ())) {
	printf ("Out of memory creating game\n");
	return 0;
    }
    strcpy (game->campaignfile, config->campaignfile);
    sprintf (game->filename, "%08lx.gam", time (NULL));
    game->playertypes[0] = config->playertypes[0];
    game->playertypes[1] = config->playertypes[1];
    game->state = STATE_BRIEFING;
    game->scenid = 0;
    return 1;
}

/**
 * Interpret the arguments.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return     1 if arguments valid, 0 if not.
 */
static int interpretargs (int argc, char **argv)
{
    /* local variables */
    int c, /* argument count */
	p, /* human player */
	d = 1; /* difficulty */

    /* scan through arguments */
    for (c = 1; c < argc; ++c)
	if (! strncmp (argv[c], "-c", 2)) {
	    strcpy (game->campaignfile, &argv[c][2]);
	    if (! strchr (game->campaignfile, '.'))
		strcat (game->campaignfile, ".cam");
	} else if (! strncmp (argv[c], "-s", 2))
	    game->scenid = atoi (&argv[c][2]) - 1;
	else if (! strncmp (argv[c], "-d", 2))
	    d = atoi (&argv[c][2]);
	else if (! strncmp (argv[c], "-p", 2)) {
	    p = atoi (&argv[c][2]) - 1;
	    game->playertypes[p] = PLAYER_HUMAN;
	    game->playertypes[1 - p] = d;
	} else {
	    printf ("invalid argument %s\n", argv[c]);
	    return 0;
	}

    /* no errors, return */
    return 1;
}

/**
 * Set up the game object.
 * @return 1 if successful, 0 if not.
 */
static int setupgame (void)
{
    Scenario *scenario;

    /* randomise start player */
    srand (time (NULL));

    /* load the chosen campaign */
    if (! (game->campaign = new_Campaign ())) {
	printf ("Out of memory creating campaign\n");
	return 0;
    }
    strcpy (game->campaign->filename, game->campaignfile);
    if (! (game->campaign->load (game->campaign, 0))) {
	printf ("Cannot load campaign file %s\n", game->campaignfile);
	return 0;
    }

    /* set up the parameters from the command line */
    scenario = game->campaign->scenarios[game->scenid];
    game->battle = scenario->battle->clone (scenario->battle);
    game->battle->start = game->battle->side = rand () % 2;
    game->turnno = 0;

    /* success! */
    return 1;
}

/**
 * Save the game file.
 * @return 1 if successful, 0 if not.
 */
static int savegame (void)
{
    char *c; /* character counter */
    for (c = game->campaignfile; *c; ++c)
	*c = toupper (*c);
    if (game->save (game)) {
	printf ("Saved game %s\n", game->filename);
	return 1;
    } else {
	printf ("Cannot save game %s\n", game->filename);
	return 0;
    }
}

/**
 * Clean up.
 */
static void cleanup (void)
{
    game->destroy (game);
    config->destroy ();
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Main Program.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return     0 if successful, >0 if not.
 */
int main (int argc, char **argv)
{
    if (! initconfig ())
	return 1;
    if (! initgame ())
	return 1;
    if (! interpretargs (argc, argv))
	return 2;
    if (! setupgame ())
	return 3;
    if (! savegame ())
	return 4;
    cleanup ();
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

