/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 02-Sep-2020.
 *
 * Unit Header.
 */

/* types defined in this header */
typedef struct unit Unit;

#ifndef __UNIT_H__
#define __UNIT_H__

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct unit defines everything about the unit.
 */
struct unit {

    /*
     * Attributes
     */
    
    /** @var name is the unit name. */
    char name[33];

    /** @var side is the unit's side. */
    int side;
    
    /** @var utype is the unit type. */
    int utype;

    /** @var x is the X position. */
    int x;

    /** @var y is the Y position. */
    int y;

    /** @var hits is the current hit points. */
    int hits;

    /** @var moves is the current movement points. */
    int moves;

    /*
     * Methods
     */

    /**
     * Destroy a unit when no longer needed.
     * @param unit is the unit to destroy.
     */
    void (*destroy) (Unit *unit);

    /**
     * Clone a unit.
     * @param unit is the unit to clone.
     * @return the new unit.
     */
    Unit *(*clone) (Unit *unit);

    /**
     * Write a unit to a file.
     * @param unit is the unit to write.
     * @param output is the output file handle.
     * @return 1 if successful, 0 if not.
     */
    int (*write) (Unit *unit, FILE *output);

    /**
     * Read a unit from the file.
     * @param unit is the unit to read.
     * @param input is the input file handle.
     * @return 1 if successful, 0 if not.
     */
    int (*read) (Unit *unit, FILE *input);

};

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Standard constructor.
 * @return the newly initialised unit.
 */
Unit *new_Unit (void);

#endif
