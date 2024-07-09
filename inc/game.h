/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Game Header.
 */

/* types defined in this header */
typedef struct game Game;

#ifndef __GAME_H__
#define __GAME_H__

/* header dependencies */
#include "cwg.h"
#include "campaign.h"
#include "report.h"

/*----------------------------------------------------------------------
 * Data Definitions
 */

/**
 * @struct game
 * The structure of a single game.
 */
struct game {

    /*
     * Attributes
     */

    /** @var gamefile The filename of the current game. */
    char filename[13];

    /** @var campaignfile The filename of the game's campaign. */
    char campaignfile[13];

    /** @var playertypes The player type for each side. */
    int playertypes[2];

    /** @var briefed[2] 1 if each player acknowledged the briefing. */
    int briefed[2];

    /** @var debriefed[2] 1 if each player acknowledged the debrief. */
    int debriefed[2];

    /** @var state State the game was saved in. */
    int state;
    
    /** @var scenid Scenario being played. */
    int scenid;

    /** @var turn Turn number. */
    int turnno;

    /** @var battle Current state of the battle. */
    Battle *battle;

    /** @var reports The turn report. */
    Report *report;

    /** @var campaign The game's campaign. */
    Campaign *campaign;

    /*
     * Methods
     */

    /**
     * Destroy a game when it is no longer needed.
     * @param game The game to destroy.
     */
    void (*destroy) (Game *game);

    /**
     * Clear the game data.
     * @param game The game object to clear.
     */
    void (*clear) (Game *game);

    /**
     * Save the game. The filename is taken from the attributes.
     * @param  game The game to save.
     * @return      1 if successful, 0 on failure.
     */
    int (*save) (Game *game);

    /**
     * Load a game. The filename is taken from the attributes.
     * @param  game    The game to load.
     * @param  summary 0 to load the full game, 1 for summary only.
     * @return         1 if successful, 0 on failure.
     */
    int (*load) (Game *game, int summary);

    /**
     * End the game turn.
     * @param  game  The game affected.
     * @return       The next game state.
     */
    int (*turn) (Game *game);

    /**
     * Set up the next scenario.
     * @param  game The game affected.
     * @return      The next game state.
     */
    int (*setscenario) (Game *game);

    /**
     * Calculate the centre of a force.
     * @param  game The game object.
     * @param  side The side to look for.
     * @return      the centre of the force..
     */
    int (*centre) (Game *game, int side);
};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new game object.
 * @return The new game object.
 */
Game *new_Game (void);

#endif
