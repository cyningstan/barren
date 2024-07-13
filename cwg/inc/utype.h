/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 13-Aug-2020.
 *
 * Unit Type Header.
 */

/* types defined in this header */
typedef struct unit_type UnitType;

#ifndef __UTYPE_H__
#define __UTYPE_H__

/*----------------------------------------------------------------------
 * Data Structures.
 */

/**
 * @struct unit_type is the class for the types of unit in the battle.
 */
struct unit_type {

    /*
     * Attributes
     */

    /** @var name is the name of the unit type. */
    char name[CWG_MAXLEN];

    /** @var cost is the cost to build this unit. */
    int cost;
 
    /** @var hits is the maximum number of hit points. */
    int hits;

    /** @var power is the attack power of the unit type. */
    int power;

    /** @var range is the attack range of the unit type. */
    int range;

    /** @var armour is the protective armour of the unit type. */
    int armour;

    /** @var moves is the movement points of the unit type. */
    int moves;

    /** @var builds is the list of units that this one builds. */
    int builds[CWG_UTYPES];

    /*
     * Methods
     */
    
    /**
     * Destroy a unit type when it is no longer needed.
     * @param utype is the unit type to destroy.
     */
    void (*destroy) (UnitType *utype);

    /**
     * Write the unit type to an already open file.
     * @param utype is the unit type to write.
     * @param output is the output file.
     * @return 1 on success, 0 on failure.
     */
    int (*write) (UnitType *utype, FILE *output);

    /**
     * Read the unit type from an already open file.
     * @param utype is the unit type to read.
     * @param input is the input file.
     * @return 1 on success, 0 on failure.
     */
    int (*read) (UnitType *utype, FILE *input);

};

/*----------------------------------------------------------------------
 * Constructor and Class Function Prototypes.
 */

/**
 * Standard constructor.
 * @return the newly initialised unit type.
 */
UnitType *new_UnitType (void);

#endif
