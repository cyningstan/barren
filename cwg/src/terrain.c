/*======================================================================
 * CWG: Cyningstan Wargame Engine.
 * An engine for simple turn-based wargames.
 *
 * Copyright (C) Damian Gareth Walker, 2020.
 * Created: 15-Aug-2020.
 *
 * Terrain Type Module.
 */

/* Standard C Headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Project Headers */
#include "cwg.h"
#include "terrain.h"

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Destroy a terrain type when it is no longer needed.
 * @param terrain is the terrain to destroy.
 */
static void destroy (Terrain *terrain)
{
    /* free the memory */
    if (terrain)
	free (terrain);
}

/**
 * Write a terrain type to an already open file.
 * @param terrain is the terrain to write.
 * @param output is the file to write to.
 * @return 1 on success, 0 on failure.
 */
static int write (Terrain *terrain, FILE *output)
{
    /* local variables */
    int u, /* unit type counter */
	s = 1; /* success flag */
    Cwg *cwg; /* Cwg object */

    /* get the Cwg object */
    cwg = get_Cwg ();

    /* write the details */
    s &= cwg->writestring (terrain->name, output);
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->writeint (&terrain->moves[u], output);
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->writeint (&terrain->defence[u], output);

    /* successful */
    return s;
}

/**
 * Read a terrain type from an already open file.
 * @param terrain is the terrain to read.
 * @param input is the file to read from.
 * @return 1 on success, 0 on failure.
 */
static int read (Terrain *terrain, FILE *input)
{
    /* local variables */
    int u, /* unit type counter */
	s = 1; /* success flag */
    Cwg *cwg; /* Cwg object */

    /* get the Cwg object */
    cwg = get_Cwg ();

    /* read the details */
    s &= cwg->readstring (terrain->name, input);
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->readint (&terrain->moves[u], input);
    for (u = 0; u < CWG_UTYPES; ++u)
	s &= cwg->readint (&terrain->defence[u], input);

    /* successful */
    return s;
}

/*----------------------------------------------------------------------
 * Constructor Function Prototypes.
 */

/**
 * Default constructor.
 * @returns a new Terrain object.
 */
Terrain *new_Terrain (void)
{
    /* local variables */
    Terrain *terrain; /* the terrain to return */
    int u; /* unit type counter */

    /* reserve memory */
    if (! (terrain = malloc (sizeof (Terrain))))
	return NULL;

    /* initialise data */
    *terrain->name = '\0';
    for (u = 0; u < CWG_UTYPES; ++u)
	terrain->moves[u] = terrain->defence[u] = 0;

    /* initialise methods */
    terrain->destroy = destroy;
    terrain->write = write;
    terrain->read = read;

    /* return the new object */
    return terrain;
}
