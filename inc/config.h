/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 23-Jan-2021.
 *
 * Init File Hander Header.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* project includes */
#include "uiscreen.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @enum player_type is the type of player. */
typedef char PlayerType;
enum player_type {
    PLAYER_HUMAN, /* human (hotseat) player */
    PLAYER_COMPUTER, /* computer player, easy level */
    PLAYER_FAIR, /* computer player, fair level */
    PLAYER_HARD, /* computer player, hard level */
    PLAYER_PBM, /* PBM player */
    PLAYER_LAST /* placeholder */
};

/** @struct config The configuration. */
typedef struct config Config;
struct config {

    /** @var campaignfile Default campaign file for a new game. */
    char campaignfile[13];

    /** @var playertypes Default player types for a new game. */
    int playertypes[2];

    /** @var gamefile The filename for the game in play. */
    char gamefile[13];

    /*
     * Public Method Declarations.
     */

    /**
     * Destroy the configuration when no longer needed.
     */
    void (*destroy) (void);

    /**
     * Load the configuration from a file.
     */
    void (*load) (void);

    /**
     * Save the configuration to a file.
     */
    void (*save) (void);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new configuration.
 * @return The new configuration.
 */
Config *new_Config (void);

#endif
