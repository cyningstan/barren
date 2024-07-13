/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 26-Sep-2020.
 *
 * Computer Player Module.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"

/*----------------------------------------------------------------------
 * Public Methods
 */

/**
 * Destroy a player when no longer needed.
 * @param player is the player to destroy.
 */
static void destroy (Player *player)
{
    if (player) {
	if (player->data)
	    free (player->data);
	free (player);
    }
}

/**
 * Initialise a player's turn.
 * @param player is the player to initialise.
 * @param battle is the battle being played.
 * @return 1 if successful, 0 if not.
 */
static int initialise (Player *player, Battle *battle)
{
    return 0;
}

/**
 * Take a single player action.
 * @param player is the player to move.
 * @param battle is the battle.
 * @return flag to continue, end turn or quit game.
 */
static PlayerAction action (Player *player, Battle *battle)
{
    return PLAYER_TURN;
}

/**
 * End the player's turn.
 * @param player is the player whose turn is to end.
 * @param battle is the battle.
 * @return 1 if successful, 0 if not.
 */
static int turn (Player *player, Battle *battle)
{
    return 0;
}

/*----------------------------------------------------------------------
 * Constructor Level Functions.
 */

/**
 * Default Constructor.
 * @return a new computer player.
 */
ComputerPlayer *new_ComputerPlayer (void)
{
    /* local variables */
    ComputerPlayer *player; /* the new computer player */

    /* reserve the memory */
    if (! (player = malloc (sizeof (ComputerPlayer))))
	return NULL;

    /* initialise the public attributes */
    player->movehook = NULL;
    player->attackhook = NULL;
    player->createhook = NULL;
    player->restorehook = NULL;

    /* initialise the methods */
    player->destroy = destroy;
    player->initialise = initialise;
    player->action = action;
    player->turn = turn;

    /* return the new player */
    return player;
}
