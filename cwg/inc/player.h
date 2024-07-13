/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 26-Sep-2020.
 *
 * Player Header.
 * This is effectively an abstract class. It is intended that there be
 * "subclass" modules: ComputerPlayer, HumanPlayer, RemotePlayer, etc.
 */

/* types defined in this header */
typedef struct player Player;

#ifndef __PLAYER_H__
#define __PLAYER_H__

/* includes for external types defs */
#include "unit.h"
#include "battle.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @enum player_action is the action desired by the player.
 */
typedef enum player_action {
    PLAYER_CONTINUE, /* continue with turn */
    PLAYER_TURN, /* end turn */
    PLAYER_SURRENDER /* surrender the battle */
} PlayerAction;

/**
 * @struct player is the player structure.
 */
typedef struct player_data PlayerData;
struct player {

    /**
     * @var data The internal data for the player.
     */
    PlayerData *data;

    /**
     * Destroy a player when no longer needed.
     * @param player is the player to destroy.
     */
    void (*destroy) (Player *player);

    /**
     * Movement display hook function. This is run after each step of
     * movement, typically to update the display.
     * @param unit is the unit that was moved.
     * @param x is the original x coordinate before movement.
     * @param y is the original y coordinate before movement.
     */
    void (*movehook) (Unit *unit, int x, int y);

    /**
     * Attack display hook function. This is run after an attack is
     * completed, including any return fire, typically to update the
     * display.
     * @param src is the source unit, the one making the attack.
     * @param tgt is the target unit.
     */
    void (*attackhook) (Unit *src, Unit *tgt);

    /**
     * Create unit display hook function. This is run after a unit is
     * created on the battlefield, typically to update the display.
     * @param src is the source unit, the one doing the creating.
     * @param tgt is the target unit, the one that was created.
     */
    void (*createhook) (Unit *src, Unit *tgt);

    /**
     * Restore unit display hook function. This is run after a unit is
     * restored to full strength on the battlefield, typically to
     * update the display.
     * @param src is the source unit, the one doing the restoring.
     * @param tgt is the target unit, the one being restored.
     */
    void (*restorehook) (Unit *src, Unit *tgt);

    /**
     * Initialise a player's turn.
     * @param player is the player to initialise.
     * @param battle is the battle being played.
     * @return 1 if successful, 0 if not.
     */
    int (*initialise) (Player *player, Battle *battle);

    /**
     * Take a single player action.
     * @param player is the player to move.
     * @param battle is the battle.
     * @return flag to continue, end turn or quit game.
     */
    PlayerAction (*action) (Player *player, Battle *battle);

    /**
     * End the player's turn.
     * @param player is the player whose turn is to end.
     * @param battle is the battle.
     * @return 1 if successful, 0 if not.
     */
    int (*turn) (Player *player, Battle *battle);

};

/*----------------------------------------------------------------------
 * Top Level Function Declarations.
 */

/**
 * Create a new player.
 * @return The new player.
 */
Player *new_Player (void);

#endif
