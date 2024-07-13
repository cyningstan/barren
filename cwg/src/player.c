/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Created: 07-Oct-2021.
 *
 * Player Module - stub to be extended.
 */

/*----------------------------------------------------------------------
 * Includes.
 */

/* standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* project headers */
#include "cwg.h"
#include "player.h"
#include "battle.h"
#include "utype.h"
#include "terrain.h"
#include "unit.h"
#include "map.h"

/*----------------------------------------------------------------------
 * Public Method Definitions.
 */

/**
 * Destroy a player when no longer needed.
 * @param player is the player to destroy.
 */
static void destroy (Player *player)
{
    if (player)
	free (player);
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
    return 1;
}

/*----------------------------------------------------------------------
 * Top Level Function Definitions.
 */

/**
 * Create a new player.
 * @return The new player.
 */
Player *new_Player (void)
{
    Player *player;

    /* reserve memory for the player */
    if (! (player = malloc (sizeof (Player))))
	return NULL;
    player->data = NULL;

    /* initialise the methods */
    player->destroy = destroy;
    player->movehook = NULL;
    player->attackhook = NULL;
    player->createhook = NULL;
    player->restorehook = NULL;
    player->initialise = initialise;
    player->action = action;
    player->turn = turn;

    /* return the new player */
    return player;
}
