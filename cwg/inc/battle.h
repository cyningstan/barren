/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 19-Sep-2020.
 *
 * Battle Header.
 */

/* types defined inthis header */
typedef struct battle Battle;

#ifndef __BATTLE_H__
#define __BATTLE_H__

/* includes for required external type defs */
#include "utype.h"
#include "terrain.h"
#include "map.h"
#include "unit.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @enum CwgAttackResult
 * Results of an attack for the display hook.
 * This is effectively a pair of 1-bit flags:
 *   0x1 indicates return fire,
 *   0x2 indicates a unit was destroyed.
 */
typedef enum {
    CWG_ATT_NO_RETURN, /* defender doesn't return fire */
    CWG_ATT_RETURN_FIRE, /* defender returns fire */
    CWG_ATT_DEFR_KILLED, /* defender killed (implies ..._NO_RETURN) */
    CWG_ATT_ATTR_KILLED /* attack killed (implies ..._RETURN_FIRE) */
} CwgAttackResult;

/**
 * @enum MovementAlgorithm
 * Algorithm for unit movement to non-adjacent squares.
 * Designed to allow faster movement for human players who can guide
 * their units around obstacles.
 */
typedef enum {
    CWG_MOVE_QUICK, /* fast movement with rudimentary pathfinding */
    CWG_MOVE_SMART /* movement with full pathfinding */
} CwgMovementAlgorithm;

/* display hooks */
typedef void (*MoveHook) (Unit *, int, int);
typedef void (*AttackHook) (Unit *, Unit *, CwgAttackResult);
typedef void (*CreateHook) (Unit *, Unit *);
typedef void (*RestoreHook) (Unit *, Unit *);

/**
 * @struct battle holds the starting or current state of battle.
 */
struct battle {

    /*
     * Attributes.
     */

    /** @var start is the side who took the first turn. */
    int start;
    
    /** @var side is the side whose turn it is. */
    int side;

    /** @var resources is the starting resources for production. */
    int resources[2];

    /** @var builds is an array of unit types that can be built. */
    int builds[CWG_UTYPES];

    /** @var utypes is the unit type list. */
    UnitType **utypes;

    /** @var terrain is the terrain type list. */
    Terrain **terrain;

    /** @var units is the unit list. */
    Unit *units[CWG_UNITS];

    /** @var map is the map used for the battle. */
    Map *map;

    /**
     * Methods.
     */

    /**
     * Destroy a battle when the game is over.
     * @param battle is the battle to destroy.
     */
    void (*destroy) (Battle *battle);

    /**
     * Clone a battle state.
     * @param battle is the battle to clone.
     * @return a copy of the battle.
     */
    Battle *(*clone) (Battle *battle);

    /**
     * Write the battle to an already open file.
     * @param battle is the battle to write.
     * @param output is the output file.
     * @return 1 on success, 0 on failure.
     */
    int (*write) (Battle *battle, FILE *output);

    /**
     * Read the battle from an already open file.
     * @param battle is the battle to read.
     * @param input is the input file.
     * @return 1 on success, 0 on failure.
     */
    int (*read) (Battle *battle, FILE *input);

    /**
     * Move a unit towards a destination.
     * @param battle is the battle to affect.
     * @param unit is the unit.
     * @param x is the x coordinate to move to.
     * @param y is the y coordinate to move to.
     * @return 1 if successful, 0 if not.
     */
    int (*move) (Battle *battle, Unit *unit, int x, int y,
		 MoveHook hook);

    /**
     * Attack one unit with another.
     * @param battle is the battle to affect.
     * @param unit is the unit to move.
     * @param target is the target to attack.
     * @return 1 if successful, 0 if not.
     */
    int (*attack) (Battle *battle, Unit *unit, Unit *target,
	AttackHook hook);

    /**
     * Create a unit.
     * @param battle is the battle to affect.
     * @param src is the unit doing the building.
     * @param utype is the unit type to build.
     * @param x is the x coordinate to place the new unit.
     * @param y is the y coordinate to place the new unit.
     * @return 1 if successful, 0 if not.
     */
    int (*create) (Battle *battle, Unit *src, int utype,
		     int x, int y, CreateHook hook);

    /**
     * Restore a unit.
     * @param battle is the battle to affect.
     * @param builder is the unit doing the repairing.
     * @param built is the unit to repair.
     * @return 1 if successful, 0 if not.
     */
    int (*restore) (Battle *battle, Unit *builder, Unit *built,
		    RestoreHook hook);

    /**
     * Check for victory on the battlefield.
     * @param battle is the battle to check.
     * @return -1 for ongoing battle, or victorious side 0 or 1.
     */
    int (*victory) (Battle *battle);
    
    /**
     * End a player's turn.
     * @param battle is the battle to affect.
     * @return 1 if successful, 0 if not.
     */
    int (*turn) (Battle *battle);

    /**
     * Set the difficulty level for one side.
     * @param side The side for which to set the difficulty.
     * @param level The difficulty level.
     */
    void (*setlevel) (int side, int level);

    /**
     * Set the movement algorithm.
     * @param algorithm The algorithm to use for movement:
     */
    void (*setmovement) (int algorithm);

};

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Default constructor.
 * @return a new Battle object.
 */
Battle *new_Battle (UnitType **utypes, Terrain **terrain);

#endif
