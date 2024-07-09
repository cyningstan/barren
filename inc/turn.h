/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * Play-by-mail Turn Header.
 */

/* types defined in this header */
typedef struct turn Turn;

#ifndef __TURH_H__
#define __TURN_H__

/* header dependencies */
#include "cwg.h"
#include "report.h"
#include "game.h"

/*----------------------------------------------------------------------
 * Data Definitions
 */

/**
 * @struct turn
 * The structure of a game turn in a PBM game.
 */
struct turn {

    /*
     * Attributes
     */

    /** @var filename The filename of the current turn. */
    char filename[13];

    /** @var campaignfile The filename of the turn's campaign. */
    char campaignfile[13];

    /** @var playertypes The player type for each side. */
    int playertypes[2];

    /** @var briefed[2] 1 if each player acknowledged the briefing. */
    int briefed[2];

    /** @var debriefed[2] 1 if each player acknowledged the debrief. */
    int debriefed[2];

    /** @var scenid Scenario being played. */
    int scenid;

    /** @var player Whose turn it is. */
    int player;

    /** @var start Who started the current scenario. */
    int start;
    
    /** @var turn Turn number. */
    int turnno;

    /** @var battle Current state of the battle. */
    Battle *battle;

    /** @var reports The turn report. */
    Report *report;

    /*
     * Methods
     */

    /**
     * Destroy a turn when it is no longer needed.
     * @param turn The turn to destroy.
     */
    void (*destroy) (Turn *turn);

    /**
     * Clear the turn data.
     * @param turn The turn object to clear.
     */
    void (*clear) (Turn *turn);

    /**
     * Save the turn. The filename is taken from the attributes.
     * @param  turn The turn to save.
     * @return      1 if successful, 0 on failure.
     */
    int (*save) (Turn *turn);

    /**
     * Load a turn. The filename is taken from the attributes.
     * @param  turn    The turn to load.
     * @param  summary 0 to load the full turn, 1 for summary only.
     * @return         1 if successful, 0 on failure.
     */
    int (*load) (Turn *turn, int summary);

    /**
     * Populate a PBM turn object from a game.
     * @param turn The turn to populate.
     * @param game The game affected.
     * @return     1 if successful, 0 on failure.
     */
    int (*gametoturn) (Turn *turn, Game *game);

    /**
     * Populate a game object from a PBM turn.
     * @param turn The turn to populate the game from.
     * @param game The game to populate.
     * @return     1 if successful, 0 on failure.
     */
    int (*turntogame) (Turn *turn, Game *game);

};

/**
 * Construct a new turn object.
 * @return The new turn object.
 */
Turn *new_Turn (void);

#endif
