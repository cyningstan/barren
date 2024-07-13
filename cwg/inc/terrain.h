/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 15-Aug-2020.
 *
 * Terrain Type Header.
 */

/* types defined in this header */
typedef struct terrain Terrain;

#ifndef __TERRAIN_H__
#define __TERRAIN_H__

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/**
 * @struct terrain defines everything about a terrain type.
 */
struct terrain {

    /*
     * Attributes
     */

    /** @var name is the name of the terrain type. */
    char name[CWG_MAXLEN];

    /** @var moves is a table of movement costs. */
    int moves[CWG_UTYPES];

    /** @var defence is a table of defence bonuses. */
    int defence[CWG_UTYPES];

    /*
     * Methods
     */

    /**
     * Destroy a terrain type when it is no longer needed.
     * @param terrain is the terrain to destroy.
     */
    void (*destroy) (Terrain *terrain);

    /**
     * Write a terrain type to an already open file.
     * @param terrain is the terrain to write.
     * @param output is the file to write to.
     * @return 1 on success, 0 on failure.
     */
    int (*write) (Terrain *terrain, FILE *output);

    /**
     * Read a terrain type from an already open file.
     * @param terrain is the terrain to read.
     * @param input is the file to read from.
     * @return 1 on success, 0 on failure.
     */
    int (*read) (Terrain *terrain, FILE *input);

};

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Default constructor.
 * @returns a new Terrain object.
 */
Terrain *new_Terrain (void);

#endif
