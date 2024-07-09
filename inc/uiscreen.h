/*======================================================================
 * Barren Planet
 * A turn-based strategy game.
 *
 * Copyright (C) Damian Gareth Walker, 2021.
 * User Interface Screen Header.
 */

/* UIScreen type definition */
typedef struct uiscreen UIScreen;

#ifndef __UISCREEN_H__
#define __UISCREEN_H__

/* header dependencies */
#include "game.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @enum UIState.
 * A list of user interface screens.
 */
typedef enum {
    STATE_NEWGAME, /* user is setting up a new game */
    STATE_BRIEFING, /* user is looking at a briefing */
    STATE_REPORT, /* local human about to view turn report */
    STATE_COMPUTER, /* computer player taking a turn */
    STATE_PBM, /* remote human taking a turn */
    STATE_HUMAN, /* local human taking a turn */
    STATE_DEBRIEFING, /* debriefing */
    STATE_ENDGAME, /* end game */
    STATE_QUIT, /* user wants to leave the program */
    STATE_LAST /* placeholder */
} UIState;

/**
 * @struct UiScreen.
 * The user interface screen.
 */
typedef struct uiscreendata UIScreenData;
struct uiscreen {

    /*
     * Attributes
     */
    
    /** @var data The private UI data. */
    UIScreenData *data;

    /*
     * Methods
     */

    /**
     * Destroy the current UI screen when no longer needed.
     * @param uiscreen The screen to destroy.
     */
    void (*destroy) (UIScreen *uiscreen);

    /**
     * Initialise the UI state when first encountered.
     * @param uiscreen The screen to affect.
     */
    void (*init) (UIScreen *uiscreen);

    /**
     * Show the UI screen.
     * @param uiscreen The screen to show.
     * @return         The ID of the screen to show next.
     */
    UIState (*show) (UIScreen *uiscreen);

    /**
     * Calculate map starting position for a player.
     * @param battle The battlefield.
     * @return       The location on which to centre the map.
     */
    int (*maplocation) (Battle *battle);

    /**
     * Player turn start screen, for human-vs-human games.
     * @param  game The game being played.
     * @return      The state implied by the menu.
     */
    UIState (*startscreen) (Game *game);

    /**
     * Display a scrolling map without a cursor.
     * @param battle The battlefield.
     * @param x      Pointer to the leftmost coordinate.
     * @param y      Pointer to the topmost coordinate.
     */
    void (*scrollmap) (Battle *battle, int *x, int *y);

    /**
     * Confirm a potentially destructive action.
     * @param  message The message to display.
     * @return         1 if yes, 2 if no.
     */
    int (*confirm) (char *message);
};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Construct a new generic UI screen.
 * This won't work on its own but sets up a few elements
 * common to multiple user interface screens (such as
 * inventory browsing). It will usually be called
 * by one of the subclass constructors below.
 */
UIScreen *new_UIScreen (void);

/**
 * Construct the New Game Screen.
 * @param  game The game object.
 * @return      The new New Game Screen.
 */
UIScreen *new_NewGameScreen (Game *game);

/**
 * Construct the Briefing Screen.
 * @param  game The game object.
 * @return      The new Briefing Screen.
 */
UIScreen *new_BriefingScreen (Game *game);

/**
 * Construct the Human Turn Screen.
 * @param  game The game object.
 * @return      The new Human Turn Screen.
 */
UIScreen *new_HumanTurnScreen (Game *game);

/**
 * Construct the Computer Turn Screen.
 * @param  game The game object.
 * @return      The new Computer Turn Screen.
 */
UIScreen *new_ComputerTurnScreen (Game *game);

/**
 * Construct the Human Turn Screen.
 * @param  game The game object.
 * @return      The new Human Turn Screen.
 */
UIScreen *new_PBMTurnScreen (Game *game);

/**
 * Construct the Turn Report Screen.
 * @param  game The game object.
 * @return      The new Turn Report Screen.
 */
UIScreen *new_TurnReportScreen (Game *game);

/**
 * Construct the Debriefing Screen.
 * @param  game The game object.
 * @return      The new Debriefing Screen.
 */
UIScreen *new_DebriefingScreen (Game *game);

/**
 * Construct the End Game Screen.
 * @param  game The game object.
 * @return      The new End Game Screen.
 */
UIScreen *new_EndGameScreen (Game *game);

#endif
